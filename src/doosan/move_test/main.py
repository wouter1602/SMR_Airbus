#!/usr/bin/env python
"""
    Files loads different positions from a JSON file and gives the user the option to move the robot to each position.

"""

from asyncio.tasks import wait_for
import doosan_drfl as drfl
import time
import logging
import asyncio
import numpy as np
import sys
from pathlib import Path
import json
from enum import Enum
import signal
import threading

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)


IP_ADDRESS = "192.168.8.50"
PORT = 12345

SPEED = 5.0 # deg/s
LIN_SPEED = 20.0 # m/s
ACCELERATION = 5.0 # deg/s2
LIN_ACCELERATION = 20.0 # m/s2
MOVE_TIMEOUT = 60.0 # seconds
POLL_INTERVAL = 0.05 # seconds

robot = drfl.CDRFLEx()

class Motion(Enum):
    NO_MOTION = 0
    CALCULATING = 1
    MOVING = 2

"""
    Global variables for the callback functions.
"""

get_control_acces: bool = False
is_in_standby: bool = False
is_in_safe_off: bool = False
_waiting_on_mwait: bool = False

"""
    Callback functions
"""

def on_monitoring_access_control(access: drfl.MONITORING_ACCESS_CONTROL) -> None:
    global get_control_acces
    access_in_string = drfl.MONITORING_ACCESS_CONTROL(access).name
    logger.debug(f"[on_monitoring_access_control] access: {access_in_string} [{access}]")

    if access == drfl.MONITORING_ACCESS_CONTROL.Grant:
        logger.info("[on_monitoring_access_control] access granted !")
        get_control_acces = True

    elif access == drfl.MONITORING_ACCESS_CONTROL.Loss:
        logger.info("[on_monitoring_access_control] access lost !")
        get_control_acces = False


def on_monitoring_state(state: drfl.ROBOT_STATE) -> None:
    global is_in_standby, is_in_safe_off
    state_in_string = drfl.ROBOT_STATE(state).name
    logger.debug(f"[on_monitoring_state] state: {state_in_string} [{state}]")

    if state == drfl.ROBOT_STATE.Standby:
        is_in_standby = True
    else:
        is_in_standby = False

    if state == drfl.ROBOT_STATE.Safe_off:
        is_in_safe_off = True
    else:
        is_in_safe_off = False


# TODO: calculate timeout based on expected movement time
async def wait_for_standby(timeout: float) -> bool:
    start_time = time.time()
    while not is_in_standby:
        await asyncio.sleep(POLL_INTERVAL)
        if time.time() - start_time > timeout:
            return False
    return True


"""
    Loading pose json
"""

async def load_poses(filepath: str | Path) -> list:
    with open(filepath, "r") as f:
        poses = json.load(f)
    return poses

def amovej_wrapper(pose: np.ndarray) -> bool:
    return robot.amovej(pos = pose,
        vel=SPEED,
        acc=ACCELERATION,
        time=0.0,
        move_mode = drfl.MOVE_MODE.Absolute,
        blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate)


def amovel_wrapper(pose: np.ndarray) -> bool:
    velocity = np.array([LIN_SPEED, 0], dtype=np.float32)
    acceleration = np.array([LIN_ACCELERATION, 0], dtype=np.float32)

    return robot.movel(pos=pose,
        vel=velocity,
        acc=acceleration,
        time=0.0,
        move_mode=drfl.MOVE_MODE.Absolute,
        move_reference=drfl.MOVE_REFERENCE.Base,
        blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate,
        app_type=drfl.DR_MV_APP.NoApp)

MOVE_HANDLERS = {
    "joint": amovej_wrapper,
    "linear": amovel_wrapper,
}

async def test_function() -> None:
    i = 0
    while i < 20:
        logger.info(f"Test function running... [{i}]")
        await asyncio.sleep(0.5)
        i += 1

async def execute_poses(pose: dict) -> None:
    move_type = pose["move_type"]
    handler = MOVE_HANDLERS.get(move_type)


    if handler is None:
        logger.warning(f"Wrong move type: {move_type}")
        return

    logger.debug(f"Executing pose: {pose['name']} [{pose['move_type']}]")

    pose_array = np.array(pose["pose_array"], dtype=np.float32)

    motion_complete = asyncio.Event()
    loop = asyncio.get_running_loop()

    def _blocking_motion():
        if not handler(pose_array):
            logger.error(f"Failed to execute {move_type} command")
            robot.close_connection()
            raise SystemExit(1)
        time.sleep(0.5)

        while True:
            if robot.check_motion() == 0:
                break
            time.sleep(0.1)
        # robot.mwait()
        loop.call_soon_threadsafe(motion_complete.set)

    thread = threading.Thread(target=_blocking_motion, daemon=True)
    thread.start()

    async def _wait_for_event():
        await asyncio.wait_for(motion_complete.wait(), timeout=MOVE_TIMEOUT)

    try:
        await asyncio.gather(
            _wait_for_event(),
            test_function(),
        )
    except asyncio.TimeoutError:
        logger.error("Move timed out waiting for motion complete")
        robot.close_connection()
        raise SystemExit(1)

    logger.info("Target Reached!")
    await asyncio.sleep(POLL_INTERVAL)

async def prompt_pose(poses: list) -> None:
    while True:
        print("\n Select pose:")
        for i, pose in enumerate(poses, start=1):
            print(f"    [{i}] {pose['name']} [{pose['move_type']}] {pose['pose_array']}")
        print("    [q] quit")

        choice = await asyncio.get_event_loop().run_in_executor(None, input, "> ")
        choice = choice.strip()

        if choice.lower() == "q":
            logger.info("Received quit command")
            break
        if choice.isdigit() and 1 <= int(choice) <= len(poses):
            await execute_poses(poses[int(choice) - 1])
        else:
            logger.info(F"Invalid input. Enter a number between 1 and {len(poses)}, or 'q'.")

def get_motion_info() -> Motion:
    global robot
    try:
        motion_state = robot.check_motion()
        logger.debug(f"Motion state: {motion_state}")
        return Motion(motion_state)
    except Exception as e:
        logger.error(f"Failed to get motion info: {e}")
        raise

async def wait_for_motion_complete(poll_interval: float = 0.1, timeout: float = 10.0) -> None:
    global _waiting_on_mwait
    loop = asyncio.get_running_loop()

    await asyncio.sleep(0.8)

    async def _poll():
        while True:
            # Runs the blocking C++ call in a thread, freeing the event loop
            state = await loop.run_in_executor(None, get_motion_info)
            if state == Motion.NO_MOTION:
                return
            await asyncio.sleep(poll_interval)

    await asyncio.wait_for(_poll(), timeout=timeout)

async def main():
    loop = asyncio.get_running_loop()
    main_task = asyncio.current_task()

    def _handle_sigint():
        if _waiting_on_mwait:
            logger.info("SIGINT received while waiting on mwait(), cancelling...")
        main_task.cancel()

    loop.add_signal_handler(signal.SIGINT, _handle_sigint)

    filepath = sys.argv[1] if len(sys.argv) > 1 else "poses.json"
    poses = await load_poses(filepath)
    # Setup callbacks
    robot.set_on_monitoring_access_control(on_monitoring_access_control)
    robot.set_on_monitoring_state(on_monitoring_state)

    # Connect to the robot
    logger.debug(f"Connecting to robot at {IP_ADDRESS}:{PORT}")
    ret = robot.open_connection(IP_ADDRESS, port=PORT)
    logger.info(f"Open_connection result: {ret}")

    if not ret:
        logger.error(f"Cannot open connection to robot at {IP_ADDRESS}:{PORT}")
        raise SystemError(1)

    # Wait for robot to enter standby or safe off state (This means it is completely ready)
    while not is_in_standby and not is_in_safe_off:
        start_time = time.time()
        if time.time() - start_time > MOVE_TIMEOUT:
            break # TODO: throw exception
        await asyncio.sleep(POLL_INTERVAL) # Sleep for POLL_INTERVAL seconds before checking again

    robot.setup_monitoring_version(1)

    # Version check
    version: drfl.SYSTEM_VERSION = drfl.SYSTEM_VERSION()
    robot.get_system_version(version)
    logger.info(f"Controller (DRCF) version: {version._szController}")
    logger.info(f"Library version: {robot.get_library_version()}")

    # Get access control and set Servo On
    for _ in range(10):
        logger.debug(f"Atempt {_} gaining access control and setting servo on")
        if not get_control_acces:
            robot.manage_access_control(drfl.MANAGE_ACCESS_CONTROL.Force_request)
            await asyncio.sleep(1.0)
            continue
        if not is_in_standby:
            robot.set_robot_control(drfl.ROBOT_CONTROL.Servo_on)
            await asyncio.sleep(2.0)
            continue
        break

    if not (get_control_acces and is_in_standby):
        logger.error("Failed setting inteneded state (access control or robot control)")
        logger.error(f"get_control_acces: {get_control_acces}, is_in_standby: {is_in_standby}")
        raise SystemExit(1)

    # set robot mode (Autonomous)
    if not robot.set_robot_mode(drfl.ROBOT_MODE.Autonomous):
        logger.error("Failed setting robot mode to Autonomous")
        raise SystemExit(1)

    # set system in real
    if not robot.set_robot_system(drfl.ROBOT_SYSTEM.Real):
        logger.error("Failed setting robot system to Real")
        raise SystemExit(1)

    logger.info("---- Ready to Move ----")

    await prompt_pose(poses)

    robot.close_connection()
    logger.info("Connection closed")
    sys.exit(0)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        logger.info("Keyboard interrupt, exiting...")
        time.sleep(0.1)
        robot.close_connection()
        sys.exit(0)
    except Exception as e:
        logger.error(f"System error: {e}")
        robot.close_connection()
        sys.exit(1)
