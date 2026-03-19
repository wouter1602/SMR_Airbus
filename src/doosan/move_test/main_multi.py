#!/usr/bin/env python
"""
    File loads different positions from a JSON file and gives the user the option to move the robot to each position.
    The robot runs in a separate process to work around the C++ library's GIL-holding behavior.
"""

import doosan_drfl as drfl
import multiprocessing as mp
import time
import logging
import asyncio
import signal
import numpy as np
import sys
from pathlib import Path
import json
from enum import Enum

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)


IP_ADDRESS = "192.168.8.50"
PORT = 12345

SPEED = 5.0           # deg/s
LIN_SPEED = 20.0      # mm/s
ACCELERATION = 5.0    # deg/s2
LIN_ACCELERATION = 20.0  # mm/s2
MOVE_TIMEOUT = 10.0   # seconds
POLL_INTERVAL = 0.05  # seconds

_waiting_on_mwait: bool = False


class Motion(Enum):
    NO_MOTION = 0
    CALCULATING = 1
    MOVING = 2


# ─────────────────────────────────────────────
#  Robot worker (runs in a separate process)
# ─────────────────────────────────────────────

def robot_worker(command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    """
    Owns the robot connection entirely.
    Receives (move_type, pose_array) tuples from command_queue.
    Sends "ready", "done", or "error:<msg>" strings via result_queue.
    Shuts down cleanly when it receives None.
    """
    worker_logger = logging.getLogger("robot_worker")

    robot = drfl.CDRFLEx()

    # ── Local state for callbacks ──
    get_control_access: bool = False
    is_in_standby: bool = False
    is_in_safe_off: bool = False

    def on_monitoring_access_control(access: drfl.MONITORING_ACCESS_CONTROL) -> None:
        nonlocal get_control_access
        access_str = drfl.MONITORING_ACCESS_CONTROL(access).name
        worker_logger.debug(f"[on_monitoring_access_control] {access_str} [{access}]")
        if access == drfl.MONITORING_ACCESS_CONTROL.Grant:
            worker_logger.info("Access granted!")
            get_control_access = True
        elif access == drfl.MONITORING_ACCESS_CONTROL.Loss:
            worker_logger.info("Access lost!")
            get_control_access = False

    def on_monitoring_state(state: drfl.ROBOT_STATE) -> None:
        nonlocal is_in_standby, is_in_safe_off
        state_str = drfl.ROBOT_STATE(state).name
        worker_logger.debug(f"[on_monitoring_state] {state_str} [{state}]")
        is_in_standby = (state == drfl.ROBOT_STATE.Standby)
        is_in_safe_off = (state == drfl.ROBOT_STATE.Safe_off)

    # ── Move wrappers ──
    def amovej_wrapper(pose: np.ndarray) -> bool:
        return robot.amovej(
            pos=pose,
            vel=SPEED,
            acc=ACCELERATION,
            time=0.0,
            move_mode=drfl.MOVE_MODE.Absolute,
            blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate,
        )

    def amovel_wrapper(pose: np.ndarray) -> bool: # Look here
        velocity = np.array([LIN_SPEED, 0], dtype=np.float32)
        acceleration = np.array([LIN_ACCELERATION, 0], dtype=np.float32)
        return robot.amovel(
            pos=pose,
            vel=velocity,
            acc=acceleration,
            time=0.0,
            move_mode=drfl.MOVE_MODE.Absolute,
            move_reference=drfl.MOVE_REFERENCE.Base,
            eBlendingType=drfl.BLENDING_SPEED_TYPE.Duplicate,
            eAppType=drfl.DR_MV_APP.NoApp,
        )

    MOVE_HANDLERS = {
        "joint": amovej_wrapper,
        "linear": amovel_wrapper,
    }

    # ── Setup ──
    robot.set_on_monitoring_access_control(on_monitoring_access_control)
    robot.set_on_monitoring_state(on_monitoring_state)

    worker_logger.debug(f"Connecting to robot at {IP_ADDRESS}:{PORT}")
    if not robot.open_connection(IP_ADDRESS, port=PORT):
        result_queue.put("error:Cannot open connection to robot")
        return

    # Wait for standby or safe off
    start = time.time()
    while not is_in_standby and not is_in_safe_off:
        if time.time() - start > MOVE_TIMEOUT:
            result_queue.put("error:Timed out waiting for robot ready state")
            robot.close_connection()
            break # TODO check what is needed
            return
        time.sleep(POLL_INTERVAL)

    robot.setup_monitoring_version(1)

    version: drfl.SYSTEM_VERSION = drfl.SYSTEM_VERSION()
    robot.get_system_version(version)
    worker_logger.info(f"Controller (DRCF) version: {version._szController}")
    worker_logger.info(f"Library version: {robot.get_library_version()}")

    # Get access control and set Servo On
    for attempt in range(10):
        worker_logger.debug(f"Attempt {attempt}: gaining access control and setting servo on")
        if not get_control_access:
            robot.manage_access_control(drfl.MANAGE_ACCESS_CONTROL.Force_request)
            time.sleep(1.0)
            continue
        if not is_in_standby:
            robot.set_robot_control(drfl.ROBOT_CONTROL.Servo_on)
            time.sleep(2.0)
            continue
        break

    if not (get_control_access and is_in_standby):
        result_queue.put(
            f"error:Failed to reach intended state — "
            f"access={get_control_access}, standby={is_in_standby}"
        )
        robot.close_connection()
        return

    if not robot.set_robot_mode(drfl.ROBOT_MODE.Autonomous):
        result_queue.put("error:Failed setting robot mode to Autonomous")
        robot.close_connection()
        return

    if not robot.set_robot_system(drfl.ROBOT_SYSTEM.Real):
        result_queue.put("error:Failed setting robot system to Real")
        robot.close_connection()
        return

    result_queue.put("ready")

    # ── Command loop ──
    while True:
        command = command_queue.get()  # Blocks freely — we're in our own process

        if command is None:  # Shutdown signal
            worker_logger.info("Shutdown command received")
            break

        move_type, pose_array = command
        handler = MOVE_HANDLERS.get(move_type)

        if handler is None:
            result_queue.put(f"error:Unknown move type: {move_type}")
            continue

        if not handler(pose_array):
            result_queue.put("error:Move command failed")
            continue

        success = robot.mwait()
        if not success:
            result_queue.put("error:mwait() failed")
            continue

        result_queue.put("done")

    robot.close_connection()
    worker_logger.info("Connection closed")


# ─────────────────────────────────────────────
#  Async helpers (main process)
# ─────────────────────────────────────────────

async def test_function() -> None:
    """Proof-of-concept parallel task — runs while the robot is moving."""
    i = 0
    while True:
        logger.info(f"[test_function] running... [{i}]")
        await asyncio.sleep(0.5)
        i += 1


async def wait_for_motion_complete(
    result_queue: mp.Queue,
    timeout: float = MOVE_TIMEOUT,
) -> None:
    """
    Waits for the worker to post "done" (or an error) on result_queue.
    Runs the blocking queue.get() in an executor so the event loop stays free.
    """
    global _waiting_on_mwait
    loop = asyncio.get_running_loop()
    _waiting_on_mwait = True

    try:
        result = await asyncio.wait_for(
            loop.run_in_executor(None, result_queue.get),
            timeout=timeout,
        )
    finally:
        _waiting_on_mwait = False

    if result != "done":
        raise RuntimeError(result)


async def execute_poses(pose: dict, command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    move_type = pose["move_type"]
    if move_type not in ("joint", "linear"):
        logger.warning(f"Wrong move type: {move_type}")
        return

    logger.debug(f"Executing pose: {pose['name']} [{move_type}]")
    pose_array = np.array(pose["pose_array"], dtype=np.float32)
    command_queue.put((move_type, pose_array))

    try:
        await asyncio.gather(
            wait_for_motion_complete(result_queue, timeout=MOVE_TIMEOUT),
            test_function(),
        )
    except asyncio.TimeoutError:
        logger.error("Move timed out waiting for motion complete")
        command_queue.put(None)  # Shut down worker
        raise SystemExit(1)
    except RuntimeError as e:
        logger.error(f"Motion error: {e}")
        command_queue.put(None)
        raise SystemExit(1)

    logger.info("Target Reached!")
    await asyncio.sleep(POLL_INTERVAL)


async def load_poses(filepath: str | Path) -> list:
    with open(filepath, "r") as f:
        return json.load(f)


async def prompt_pose(poses: list, command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    loop = asyncio.get_running_loop()
    while True:
        print("\nSelect pose:")
        for i, pose in enumerate(poses, start=1):
            print(f"    [{i}] {pose['name']} [{pose['move_type']}] {pose['pose_array']}")
        print("    [q] quit")

        choice = await loop.run_in_executor(None, input, "> ")
        choice = choice.strip()

        if choice.lower() == "q":
            logger.info("Received quit command")
            break
        elif choice.isdigit() and 1 <= int(choice) <= len(poses):
            await execute_poses(poses[int(choice) - 1], command_queue, result_queue)
        else:
            logger.info(f"Invalid input. Enter a number between 1 and {len(poses)}, or 'q'.")


# ─────────────────────────────────────────────
#  Main
# ─────────────────────────────────────────────

async def main() -> None:
    loop = asyncio.get_running_loop()
    main_task = asyncio.current_task()

    def _handle_sigint():
        if _waiting_on_mwait:
            logger.info("SIGINT received, waiting for mwait() to finish...")
        main_task.cancel()

    loop.add_signal_handler(signal.SIGINT, _handle_sigint)

    filepath = sys.argv[1] if len(sys.argv) > 1 else "poses.json"
    poses = await load_poses(filepath)

    command_queue: mp.Queue = mp.Queue()
    result_queue: mp.Queue = mp.Queue()

    worker = mp.Process(
        target=robot_worker,
        args=(command_queue, result_queue),
        daemon=True,
    )
    worker.start()

    # Wait for worker to finish setup
    result = await loop.run_in_executor(None, result_queue.get)
    if result != "ready":
        logger.error(f"Worker failed during setup: {result}")
        worker.terminate()
        raise SystemExit(1)

    logger.info("---- Ready to Move ----")

    await prompt_pose(poses, command_queue, result_queue)

    command_queue.put(None)  # Graceful shutdown
    worker.join()
    logger.info("Connection closed")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        logger.info("Keyboard interrupt, exiting...")
        sys.exit(0)
    except Exception as e:
        logger.error(f"System error: {e}")
        sys.exit(1)
