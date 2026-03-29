#!/usr/bin/env python3

import multiprocessing as mp
import logging
import asyncio
import signal
import numpy as np
import sys
from pathlib import Path
import json

import doosan_drfl as drfl
from robot_worker import robot_worker, MOVE_TIMEOUT, POLL_INTERVAL
from cognix import CognexCamera

# Setup varables
IP_DOOSAN = "192.168.0.50"
PORT_DOOSAN = 12345
IP_CAMERA_TCP = "192.168.0.12"

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)

_currently_moving: bool = False


async def wait_for_motion_complete(
    result_queue: mp.Queue,
    timeout: float = MOVE_TIMEOUT,
) -> None:
    global _currently_moving
    loop = asyncio.get_running_loop()
    _currently_moving = True

    try:
        result = await asyncio.wait_for(
            loop.run_in_executor(None, result_queue.get),
            timeout=timeout,
        )
    finally:
        _currently_moving = False

    if result != "done":
        raise RuntimeError(result)

async def execute_poses(
    pose: dict,
    command_queue: mp.Queue,
    result_queue: mp.Queue
) -> None:
    move_type = pose["move_type"]
    if move_type not in ("joint", "linear"):
        logger.warning(f"Wrong move type: {move_type}")
        return

    logger.debug(f"Executing pose: {pose['name']} [{move_type}]")
    pose_array = np.array(pose['pose_array'], dtype=np.float32)
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


async def run_sequence(
    steps: list,
    poses: dict,
    command_queue: mp.Queue,
    result_queue: mp.Queue,
    camera: CognexCamera,
) -> None:
    """Execute a named sequence of pose moves, air toggles, and camera-detected moves."""
    loop = asyncio.get_running_loop()

    for step in steps:
        step_type = step.get("type")

        if step_type == "pose":
            name = step["name"]
            if name not in poses:
                logger.error(f"Pose '{name}' not found in loaded poses")
                raise SystemExit(1)
            await execute_poses(poses[name], command_queue, result_queue)

        elif step_type == "camera":
            # Trigger camera and retry loop
            pose_array = None
            while pose_array is None:
                logger.info("Triggering camera...")
                if step["name"] == "Scan type 1":
                    cells = ["U38", "U39", "U40", "U41", "U42", "U43"]
                elif step["name"] == "Scan type 2":
                    cells = ["T38", "T39", "T40", "T41", "T42", "T43"]
                elif step["name"] == "Scan type 3":
                    cells = ["S38", "S39", "S40", "S41", "S42", "S43"]
                else:
                    raise ValueError(f"Unknown scan type: {step['name']}")


                results = await camera.trigger_and_read(cells)

                def to_float(v):
                    try:
                        return float(v)
                    except ValueError:
                        return None

                float_results = [to_float(results[c]) for c in cells]
                pose_array = None if any(v is None for v in float_results) else np.array(float_results, dtype=np.float32)

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

            camera_pose = {
                "name": step.get("name", "camera_pose"),
                "move_type": "linear",
                "pose_array": list(pose_array),
            }
            logger.info(f"Camera pose: {camera_pose['pose_array']}")
            await execute_poses(camera_pose, command_queue, result_queue)

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
        raise SystemExit(1)

    logger.info(f"Connected to TCP camera: {IP_CAMERA_TCP}.")

    worker = mp.Process(
        target=robot_worker,
        args=(command_queue, result_queue, IP_DOOSAN, PORT_DOOSAN),
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
            await run_sequence(sequences[dest], poses, command_queue, result_queue, camera_1)
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
