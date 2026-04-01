#!/usr/bin/env python3

from decimal import MAX_EMAX
import multiprocessing as mp
import logging
import asyncio
import signal
import numpy as np
import sys
from pathlib import Path
import json
import copy

from scipy.spatial.transform import Rotation as R

import doosan_drfl as drfl
from robot_worker import robot_worker, MOVE_TIMEOUT, POLL_INTERVAL
from cognix import CognexCamera, PickupType

# Setup varables
IP_DOOSAN = "192.168.0.50"
PORT_DOOSAN = 12345
IP_CAMERA_TCP = "192.168.0.12"

FORCE_Z_AXIS_DOWN = 100.0 # in mm

CELL_TYPE_1_OFFSET = 710.00 #697.71 # in mm
CELL_TYPE_2_OFFSET = 710.00 #697.71 # in mm
CELL_TYPE_3_OFFSET = 710.00 #697.71 # in mm

TYPE_1_OFFSET = np.array([107.68, -178.97, 108.10], dtype=np.float32)
TYPE_2_OFFSET = np.array([12.10, -179.68, 10.52], dtype=np.float32)
TYPE_3_OFFSET = np.array([116.27, -179.66, 115.47], dtype=np.float32)

DOOSAN_SPEED = 60 #40 #60
DOOSAN_ACCELERATION = 40 #20 #40
DOOSAN_LIN_SPEED = 150 #70 #150
DOOSAN_LIN_ACCELERATION = 75 #30 #75


MAX_FORCE_BOX = 5.5
MAX_FORCE_PLACE_DOWN = 15.0


logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)

_currently_moving: bool = False
_current_force_pose: np.ndarray | None = None

_cell_type_1_offset: float | None = None
_cell_type_2_offset: float | None = None
_cell_type_3_offset: float | None = None

_pickup_up: np.ndarray | None = None

async def wait_for_motion_complete(
    result_queue: mp.Queue,
    timeout: float = MOVE_TIMEOUT,
) -> None:
    global _currently_moving, _current_force_pose
    loop = asyncio.get_running_loop()
    _currently_moving = True

    try:
        result = await asyncio.wait_for(
            loop.run_in_executor(None, result_queue.get),
            timeout=timeout,
        )
    finally:
        _currently_moving = False

    if len(result) == 2:
        logger.debug(f"Stopped based on: {result[0]}. With result: {result[1]}") # Set result to variable to be used
        _current_force_pose = result[1].copy()
    else:
        if result != "done":
            raise RuntimeError(result)

async def execute_poses(
    pose: dict,
    command_queue: mp.Queue,
    result_queue: mp.Queue
) -> None:

    move_type = pose["move_type"]
    if move_type not in ("joint", "linear", "force"):
        logger.warning(f"Wrong move type: {move_type}")
        return

    logger.debug(f"Executing pose: {pose['name']} [{move_type}]")
    pose_array = np.array(pose['pose_array'], dtype=np.float32)

    if "max_force" in pose:
        command_queue.put((move_type, pose_array, pose["max_force"]))
    else:
        command_queue.put((move_type, pose_array))



    try:
        await wait_for_motion_complete(result_queue=result_queue, timeout=MOVE_TIMEOUT)
    except asyncio.TimeoutError:
        logger.error("Move timed out waiting for motion complete")
        command_queue.put(None) # Shutsdown robot worker thread
        raise SystemExit(1)
    except RuntimeError as e:
        logger.error(f"Motion error: {e}")
        command_queue.put(None) #Shutsdowns robot worker thread
        raise SystemExit(1)

    logger.info("Target Reached!")
    await asyncio.sleep(POLL_INTERVAL)

async def toggle_air(
    command_queue: mp.Queue,
    result_queue: mp.Queue,
    index: drfl.GPIO_CTRLBOX_DIGITAL_INDEX = drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Index_1,
    output: bool | None = None
) -> None:
    command_queue.put(("toggle_air", index, output))
    await wait_for_motion_complete(result_queue=result_queue)


async def load_config(filepath: str | Path) -> tuple[dict, str | None, dict]:
    """
    Load poses and sequences from a JSON config file.

    Expected format:
    {
        "home": "A",
        "poses": [
            {"name": "A", "move_type": "joint", "pose_array": [0, 0, 0, 0, 0, 0]},
            ...
        ],
        "sequences": {
            "C": [
                {"type": "pose", "name": "A"},
                {"type": "air", "index": 1, "output": true},
                {"type": "pose", "name": "B"},
                {"type": "pose", "name": "C"}
            ]
        }
    }

    Returns:
        poses     - dict mapping pose name -> pose dict
        home      - name of the home pose (or None)
        sequences - dict mapping destination name -> list of steps
    """
    with open(filepath, "r") as f:
        data = json.load(f)

    # Backwards-compatible: flat list means no sequences defined
    if isinstance(data, list):
        poses = {p["name"]: p for p in data}
        return poses, None, {}

    poses = {p["name"]: p for p in data.get("poses", [])}
    home = data.get("home")
    sequences = data.get("sequences", {})
    return poses, home, sequences


def calculate_transformation(original: np.ndarray, offset: np.ndarray) -> np.ndarray:

    def flip_to_180_repr(rx, ry, rz):
        """Convert XYZ Euler to the equivalent representation where Ry is near ±180."""
        rx_f = rx + 180 if rx < 0 else rx - 180
        ry_f = 180 - ry
        if ry_f > 180: ry_f -= 360   # normalize to [-180, 180]
        rz_f = rz + 180 if rz < 0 else rz - 180
        return rx_f, ry_f, rz_f

    logger.debug(f"Got original array: `{original}` and offset: `{offset}`")

    ideal_flat = R.from_euler('XYZ', [90, 180, 90], degrees=True)

    actual_flat = R.from_euler('XYZ', offset, degrees=True)

    rot_correction = actual_flat * ideal_flat.inv()

    target_ideal = R.from_euler('XYZ', original, degrees=True)

    target_actual = rot_correction * target_ideal

    rx, ry, rz = target_actual.as_euler('XYZ', degrees=True)
    rx, ry, rz = flip_to_180_repr(rx, ry, rz)
    return np.array([rx, ry, rz], dtype=np.float32)

def update_offsets(pose: np.ndarray, type: int):
    global _cell_type_1_offset, _cell_type_2_offset, _cell_type_3_offset
    if type == 1:
        _cell_type_1_offset = pose[2] + CELL_TYPE_1_OFFSET
        logger.debug(f"Current offset for cell type 1: {_cell_type_1_offset}")
    elif type == 2:
        _cell_type_2_offset = pose[2] + CELL_TYPE_2_OFFSET
        logger.debug(f"Current offset for cell type 2: {_cell_type_2_offset}")
    elif type == 3:
        _cell_type_3_offset = pose[2] + CELL_TYPE_3_OFFSET
        logger.debug(f"Current offset for cell type 3: {_cell_type_3_offset}")

async def run_sequence(
    steps: list,
    poses: dict,
    sequences: dict,
    command_queue: mp.Queue,
    result_queue: mp.Queue,
    camera: CognexCamera,
    visited: set | None = None
) -> None:
    """Execute a named sequence of pose moves, air toggles, and camera-detected moves."""
    global _cell_type_1_offset, _cell_type_2_offset, _cell_type_3_offset, _pickup_up

    if visited is None:
        visited = set()

    loop = asyncio.get_running_loop()

    for step in steps:
        step_type = step.get("type")

        if step_type == "sequence":
            name = step["name"]
            if name not in sequences:
                logger.error(f"Sequence '{name}' not found")
                raise SystemExit(1)
            if name in visited:
                logger.error(f"Circular reference detected in sequence '{name}' is already in teh call chain {visited}")
                raise SystemExit(1)
            logger.info(f"Running sub-sequence: '{name}'")
            await run_sequence(sequences[name], poses, sequences, command_queue, result_queue, camera, visited | {name})

        elif step_type == "pose":
            name = step["name"]
            if name not in poses:
                logger.error(f"Pose '{name}' not found in loaded poses")
                raise SystemExit(1)
            await execute_poses(poses[name], command_queue, result_queue)

        elif step_type == "camera":
            # Trigger camera and retry loop

            # First move camera to correct offset
            if step["cell_type"] == 1:
                offset = _cell_type_1_offset
            elif step["cell_type"] == 2:
                offset = _cell_type_2_offset
            elif step["cell_type"] == 3:
                offset = _cell_type_3_offset
            else:
                raise ValueError(f"Unknown cell type: {step['cell_type']}")

            name = step["name"]
            if name not in poses:
                logger.error(f"Pose `{name}` not found in loaded poses")
                raise SystemExit(1)
            pose = copy.deepcopy(poses[name])

            coords = pose["pose_array"]

            logger.debug(f"Z offset for cell type {step['cell_type']}: {offset - coords[2]}")

            coords[2] = offset # Set Z offset to camera coordinates

            pose["pose_array"] = coords

            await execute_poses(pose, command_queue, result_queue)

            pose_array = None
            while pose_array is None:
                logger.info("Triggering camera...")

                results = await camera.scan_scan(step["cell_type"])


                pose_array = None if any(v is None for v in results) or any(np.isnan(v) for v in results) else np.array(results, dtype=np.float32)

                if pose_array is None:
                    logger.warning("Camera returned no detection.")
                    raw = await loop.run_in_executor(
                        None, _prompt, "No object detected. [r]escan or [q]uit? "
                    )
                    choice = raw.strip().lower()
                    if choice in ("q", "quit", "exit"):
                        logger.info("Quit requested during camera step.")
                        raise asyncio.CancelledError
                    # Any other input → retry

            logger.debug(f"Got rotation: {pose_array[-3:]}")

            if step["cell_type"] == 1:
                offset = TYPE_1_OFFSET
            elif step["cell_type"] == 2:
                offset = TYPE_2_OFFSET
            elif step["cell_type"] == 3:
                offset = TYPE_3_OFFSET
            else:
                raise ValueError(f"Unknown cell type: {step['cell_type']}")

            rot_correction = calculate_transformation(pose_array[-3:], offset)

            logger.debug(f"Rot correction: {rot_correction}")
            pose_array[-3:] = rot_correction
            pose_array[2] = -70.75 # Set fixed Z offset TODO: Make global variable


            camera_pose = {
                "name": step.get("name", "camera_pose"),
                "move_type": "linear",
                "pose_array": list(pose_array.copy()),
            }
            logger.info(f"Camera pose: {camera_pose['pose_array']}")
            await execute_poses(camera_pose, command_queue, result_queue)

            # move to actual pickup

            pose_array[2] = pose_array[2] - FORCE_Z_AXIS_DOWN

            camera_pose = {
                "name": step.get("name", "camera_pose"),
                "move_type": "force",
                "pose_array": list(pose_array),
                "max_force": MAX_FORCE_BOX,
            }

            await execute_poses(camera_pose, command_queue, result_queue)

            logger.debug(f"Camera pose move down: {camera_pose['pose_array']}")

            pose_array[2] = pose_array[2] + (FORCE_Z_AXIS_DOWN * 2)

            _pickup_up = pose_array.copy() # copy the pickup to be used by the camera pickup funciton



            if _current_force_pose is not None:
                update_offsets(_current_force_pose, step["cell_type"])

        elif step_type == "camera_pickup" or step_type == "place_down_up":
            if _pickup_up is not None:
                pickup_pose = _pickup_up.copy()

                pickup = {
                    "name": "pickup above",
                    "move_type": "linear",
                    "pose_array": list(pickup_pose),
                }

                await execute_poses(pickup, command_queue, result_queue)



        elif step_type == "wait":
            seconds = step.get("seconds", 0)
            logger.info(f"Waiting {seconds}s...")
            await asyncio.sleep(seconds)

        elif step_type == "air":
            raw_index = step.get("index", 1) # TODO: Fix this
            if raw_index == 1:
                index = drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Index_1
            else:
                index = drfl.GPIO_CTRLBOX_DIGITAL_INDEX(raw_index)
            output = step.get("output", None)
            await toggle_air(command_queue, result_queue, index=index, output=output)
            state = "ON" if output else ("OFF" if output is False else "TOGGLE")
            logger.info(f"Air set: index={raw_index} → {state}")

        elif step_type == "wait_for_user_input":
            message = step.get("message", "Press Enter to continue...")
            await loop.run_in_executor(None, _prompt, message)

        elif step_type == "place_down":
            name = step["name"]

            if name not in poses:
                logger.error(f"Pose `{name}` not found in loaded poses")
                raise SystemExit(1)

            pose = copy.deepcopy(poses[name])

            await execute_poses(pose, command_queue, result_queue) # move to above place down

            coords = pose["pose_array"]
            coords[2] = coords[2] - FORCE_Z_AXIS_DOWN # lower position.

            pose_force = {
                "name": "force down",
                "pose_array": list(coords),
                "move_type": "force",
                "max_force": MAX_FORCE_PLACE_DOWN,
            }

            await execute_poses(pose_force, command_queue, result_queue) # move to place down

            coords = pose["pose_array"]

            coords[2] = coords[2] + (FORCE_Z_AXIS_DOWN * 2)

            _pickup_up = coords.copy()

        elif step_type == "calibrate":
            # move to start calibrate
            # move to end calibrate

            name = step["name"]
            if name not in poses:
                logger.error(f"Pose `{name}` not found in loaded poses")
                raise SystemExit(1)
            pose = copy.deepcopy(poses[name])
            await execute_poses(pose, command_queue, result_queue) # moving to start of the calibration pose

            coords = pose["pose_array"]
            coords[2] = coords[2] - FORCE_Z_AXIS_DOWN # lower position.

            pose["pose_array"] = coords
            pose["move_type"] = "force"
            logger.info(f"new pose: {pose['pose_array']}")

            await asyncio.sleep(0.5)
            await execute_poses(pose, command_queue, result_queue) # moving to end of the calibration pose

            logger.info(f"pose is: {_current_force_pose}")

            if _current_force_pose is not None:
                update_offsets(_current_force_pose, step["cell_type"])

        elif step["type"] == "paper":
            scan_name = step["scan_name"]
            pickup_name = step["pickup_name"]
            bin_name = step["bin_name"]
            cell_type = step["cell_type"]

            if scan_name not in poses:
                logger.error(f"Pose `{scan_name}` not found in loaded poses")
                raise SystemExit(1)

            if pickup_name not in poses:
                logger.error(f"Pose `{pickup_name}` not found in loaded poses")
                raise SystemExit(1)

            if bin_name not in poses:
                logger.error(f"Pose `{bin_name}` not found in loaded poses")
                raise SystemExit(1)

            scan_pose = poses[scan_name]
            pickup_pose = poses[pickup_name]
            bin_pose = poses[bin_name]

            force_array = copy.deepcopy(pickup_pose["pose_array"])

            place_array = copy.deepcopy(pickup_pose["pose_array"])

            force_array[2] = force_array[2] - FORCE_Z_AXIS_DOWN

            place_array[2] = place_array[2] + FORCE_Z_AXIS_DOWN

            force_pose = {
                "move_type": "force",
                "pose_array": list(force_array),
                "name": "force pickup down",
                "max_force": MAX_FORCE_BOX,
            }

            place_pose = {
                "move_type": "linear",
                "pose_array": list(place_array),
                "name": "Pickup above"
            }


            await execute_poses(scan_pose, command_queue, result_queue)

            loop_counter: int = 0
            retry_counter: int = 0
            while True:
                result, pose_array = await camera.scan_box(cell_type)

                if result == PickupType.Foam or result == PickupType.Paper:
                    retry_counter = 0
                    await execute_poses(pickup_pose, command_queue, result_queue)

                    await execute_poses(force_pose, command_queue, result_queue)

                    await toggle_air(command_queue, result_queue, index=drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Index_1, output=True)

                    await asyncio.sleep(0.8)

                    await execute_poses(pickup_pose, command_queue, result_queue)

                    await execute_poses(place_pose, command_queue, result_queue)

                    # TODO: safe new scan hight
                    if _current_force_pose is not None:
                        update_offsets(_current_force_pose, step["cell_type"])

                    await execute_poses(bin_pose, command_queue, result_queue)

                    await toggle_air(command_queue, result_queue, index=drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Index_1, output=False)

                    await execute_poses(scan_pose, command_queue, result_queue)
                elif result == PickupType.No_detect:
                    logger.info(f"No detect, got {result.name}, retrying...")

                    if retry_counter == 5:
                        logger.warning(f"Retry counter has reached {retry_counter}, giving up.")
                        break
                    else:
                        retry_counter += 1
                    continue
                else:
                    logger.info(f"No foam or paper detected, got {result.name}")
                    break

                if loop_counter == 5:
                    logger.warning(f"Paper loop has run {loop_counter} times.")

                loop_counter += 1

        else:
            logger.warning(f"Unknown step type '{step_type}', skipping")

def _prompt(message: str) -> str:
    """Blocking stdin prompt, safe to run in an executor."""
    return input(message)


async def main() -> None:
    loop = asyncio.get_event_loop()
    main_task = asyncio.current_task()

    def _handle_sigint():
        if _currently_moving:
            logger.info("SIGINT received, waiting for movement to finish...")  # TODO: Send immediate stop command.
        main_task.cancel()

    loop.add_signal_handler(signal.SIGINT, _handle_sigint)

    filepath = sys.argv[1] if len(sys.argv) > 1 else "poses_V2.json"
    poses, home, sequences = await load_config(filepath)

    command_queue: mp.Queue = mp.Queue()
    result_queue: mp.Queue = mp.Queue()

    camera_1 = CognexCamera(ip=IP_CAMERA_TCP,
        username="admin",
        password="",
    )

    try:
        await camera_1.connect()
    except Exception as e:
        logger.error(f"Failed to connect to camera (TCP): {e}")
        # raise SystemExit(1)

    logger.info(f"Connected to TCP camera: {IP_CAMERA_TCP}.")

    worker = mp.Process(
        target=robot_worker,
        args=(command_queue, result_queue, IP_DOOSAN, PORT_DOOSAN, DOOSAN_SPEED, DOOSAN_ACCELERATION, DOOSAN_LIN_SPEED, DOOSAN_LIN_ACCELERATION),
        daemon=True,
    )
    worker.start()

    result = await loop.run_in_executor(None, result_queue.get)
    if result != "ready":
        logger.error(f"Worker failed during setup: {result}")
        worker.terminate()
        raise SystemExit(1)

    logger.info("--- Ready to Move ---")

    # ── Wait for user to start ──────────────────────────────────────────────
    await loop.run_in_executor(None, _prompt, "Press Enter to start...")

    # ── Move to home ────────────────────────────────────────────────────────
    if home:
        if home not in poses:
            logger.error(f"Home pose '{home}' not found in loaded poses")
            command_queue.put(None)
            worker.join()
            raise SystemExit(1)
        logger.info(f"Moving to home position: '{home}'")
        await execute_poses(poses[home], command_queue, result_queue)
    else:
        logger.warning("No home position defined in config, skipping.")

    # ── Main pick-and-place loop ─────────────────────────────────────────────
    available = sorted(sequences.keys())
    while True:
        menu = "\n".join(f"  [{i+1}]: {name}" for i, name in enumerate(available))
        print(f"\n{menu}\n  [q]: quit")
        raw = await loop.run_in_executor(None, _prompt, "Select: ")
        choice = raw.strip().lower()

        if choice in ("q", "quit", "exit"):
            logger.info("Quit requested.")
            break

        if choice.isdigit() and 1 <= int(choice) <= len(available):
            dest = available[int(choice) - 1]
        else:
            logger.warning(f"Invalid selection '{choice}'.")
            continue

        logger.info(f"Starting sequence → '{dest}'")
        try:
            await run_sequence(sequences[dest], poses, sequences, command_queue, result_queue, camera_1)
        except SystemExit:
            # execute_poses already shut down the worker on error
            raise

        logger.info(f"Sequence '{dest}' complete.")

        # ── Return to home ───────────────────────────────────────────────────
        if home:
            logger.info(f"Returning to home position: '{home}'")
            await execute_poses(poses[home], command_queue, result_queue)

    # ── Graceful shutdown ────────────────────────────────────────────────────
    command_queue.put(None)
    worker.join()
    logger.info("Connection closed")  # TODO: close Cognex camera


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        logger.info("Keyboard interrupt, exiting...")
        sys.exit(0)
    except Exception as e:
        logger.error(f"System error: {e}")
        sys.exit(1)
