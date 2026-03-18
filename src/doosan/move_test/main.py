#!/usr/bin/env python
"""
    This file will go trough all the different moves for the robot to perfom.

    1. one Camera view
    2.

"""

import doosan_drfl as drfl
import time
import logging
import asyncio
import numpy as np
import sys

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)

IP_ADDRESS = "192.168.8.50"
PORT = 12345

SPEED = 5.0 # deg/s
ACCELERATION = 5.0 # deg/s2
MOVE_TIMEOUT = 10.0 # seconds
POLL_INTERVAL = 0.05 # seconds

robot = drfl.CDRFLEx()

"""
    Global variables for the callback functions.
"""

get_control_acces: bool = False
is_in_standby: bool = False

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

    # match access:
    #     case access.Grant:
    #         get_control_acces = True
    #         logger.info("[on_monitoring_access_control] access granted !")
    #         pass
    #     case access.Loss:
    #         get_control_acces = False
    #         logger.info("[on_monitoring_access_control] access lost !")
    #         pass
    #     case access.Request | access.Last:
    #         logger.debug(f"[on_monitoring_access_control] access request: {access_in_string} [{access}]")
    #         pass
    #     case access.Deny:
    #         logger.warning("[on_monitoring_access_control] access denied !")
    #         raise ValueError("Robot denied acces !")
    #     case _:
    #         logger.warning(f"[on_monitoring_access_control] unknown access: {access_in_string} [{access}]")
    #         raise ValueError(f"Unknown access: {access_in_string} [{access}]")

def on_monitoring_state(state: drfl.ROBOT_STATE) -> None:
    global is_in_standby
    state_in_string = drfl.ROBOT_STATE(state).name
    logger.debug(f"[on_monitoring_state] state: {state_in_string} [{state}]")

    if state == drfl.ROBOT_STATE.Standby:
        is_in_standby = True
    else:
        is_in_standby = False

# TODO: calculate timeout based on expected movement time
async def wait_for_standby(timeout: float) -> bool:
    start_time = time.time()
    while not is_in_standby:
        await asyncio.sleep(POLL_INTERVAL)
        if time.time() - start_time > timeout:
            return False
    return True


async def main():

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

    await asyncio.sleep(1.0) # TODO: wait for mode.standby or timeout.

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

    input("press any button for the first pose")

    target_pose = np.array([-69.47, 16.29, 90.08, 96.02, 70.34, -107.47], dtype=np.float32)
    logger.info(f"Sending amovej to target: {target_pose}")

    if not robot.amovej(
        pos=target_pose,
        vel=SPEED,
        acc=ACCELERATION,
        time=0.0,
        move_mode = drfl.MOVE_MODE.Absolute,
        blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate
    ):
        logger.error("Failed to execute amovej command")
        robot.close_connection()
        raise SystemExit(1)

    if not await wait_for_standby(MOVE_TIMEOUT):
        logger.error("Move timed out waiting for Standby state")
        robot.close_connection()
        raise SystemExit(1)


    logger.info("Target Reached!")

    robot.close_connection()
    logger.info("Connection closed")
    sys.exit(0)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt, exiting...")
        robot.close_connection()
        sys.exit(0)
    except Exception as e:
        logger.error(f"System error: {e}")
        robot.close_connection()
        sys.exit(1)
