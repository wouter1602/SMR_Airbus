#!/usr/bin/env python3

import time
import numpy as np
import doosan_drfl as drfl

IP_ADDRESS = "192.168.8.50"

robot = drfl.CDRFLEx()

get_control_access = False
is_standby = False


def on_monitoring_access_control(access: drfl.MONITORING_ACCESS_CONTROL) -> None:
    global get_control_access
    print(f"[on_monitoring_access_control] Control Access: {access}")
    if access == drfl.MONITORING_ACCESS_CONTROL.Grant:
        get_control_access = True
        print("Got Control Access!!")
    elif access == drfl.MONITORING_ACCESS_CONTROL.Loss:
        get_control_access = False


def on_monitoring_state(state: drfl.ROBOT_STATE) -> None:
    global is_standby
    print(f"[on_monitoring_state] Robot State: {state}")
    if state == drfl.ROBOT_STATE.Standby:
        is_standby = True
        print("Successfully Servo on!!")
    else:
        is_standby = False

def main():
    robot.set_on_monitoring_access_control(on_monitoring_access_control)
    robot.set_on_monitoring_state(on_monitoring_state)

    ret = robot.open_connection(IP_ADDRESS)
    print(f"open_connection return value: {ret}")
    if not ret:
        print(f"Cannot open connection to robot @ {IP_ADDRESS}")
        raise SystemExit(1)

    robot.setup_monitoring_version(1)

    version = drfl.SYSTEM_VERSION()
    robot.get_system_version(version)
    print(f"Controller (DRCF) version: {version._szController}")
    print(f"Library version: {robot.get_library_version()}")

    for _ in range(10):
        if not get_control_access:
            robot.manage_access_control(drfl.MANAGE_ACCESS_CONTROL.Force_request)
            time.sleep(1.0)
            continue
            if not is_standby:
                robot.set_robot_control(drfl.ROBOT_CONTROL.Servo_on)
                time.sleep(1.0)
                continue
                break

    if not (get_control_access and is_standby):
        print("Failed setting intended states")
        raise SystemExit(1)

    if not robot.set_robot_system(drfl.ROBOT_SYSTEM.Virtual):
        raise SystemExit(1)

    if not robot.set_robot_mode(drfl.ROBOT_MODE.Autonomous):
        raise SystemExit(1)

    input("Press Enter to continue...")

    target_pos = np.array([0., 0., 30., 0., 0., 0.], dtype=np.float32)
    robot.movej(target_pos, vel=50.0, acc=50.0)

    target_pos[2] = 0.
    robot.movej(target_pos, vel=50.0, acc=50.0)

    robot.close_connection()

if __name__ == "__main__":
    main()
