#!/usr/bin/env python3
"""
Minimal motion sample translated from C++ to Python using doosan_drfl wrapper.
Equivalent to 1_minimal_motion_sample.cpp from Doosan Robotics API-DRFL
"""

import time
import numpy as np
from doosan_drfl import (
    ROBOT_POSE,
    CDRFLEx,
    ROBOT_MODE,
    ROBOT_CONTROL,
    ROBOT_SYSTEM,
    MOVE_MODE,
    BLENDING_SPEED_TYPE,
    COORDINATE_SYSTEM,
    JOG_AXIS,
    MOVE_REFERENCE,
    MANAGE_ACCESS_CONTROL,
)

def main():
    # Create robot interface instance
    robot = CDRFLEx()

    try:
        # Step 1: Connect to the robot controller
        # Default IP: 192.168.137.100, Port: 12345
        print("Connecting to robot controller...")
        if not robot.open_connection(ip="192.168.8.50", port=12345):
            print("Failed to connect to robot controller")
            return False

        print("Connected successfully!")

        # Step 2: Wait for connection to stabilize
        time.sleep(1.0)

        # Step 3: Take control of the robot
        print("Taking control of the robot...")
        if not robot.manage_access_control(MANAGE_ACCESS_CONTROL.Force_request):
            print("Failed to take control")
            return False

        time.sleep(1.0)

        # Step 4: Set robot mode to Autonomous
        print("Setting robot mode to Autonomous...")
        if not robot.set_robot_mode(ROBOT_MODE.Autonomous):
            print("Failed to set robot mode")
            return False

        # Step 4: Enable operation (turn on servos)
        print("Enabling robot operation (servo on)...")
        if not robot.set_robot_control(ROBOT_CONTROL.Servo_on):
            print("Failed to enable operation")
            return False

        # Wait for servo to turn on
        time.sleep(2.0)

        # Step 5: Execute a simple joint motion (movej)
        # Target position in joint space (radians): [0, 0, 0, 0, 0, 0]
        # This moves all joints to zero position
        target_position = np.array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0], dtype=np.float32)
        velocity = 1.0  # rad/s
        acceleration = 1.0  # rad/s²

        print(f"Executing movej to position: {target_position}")
        if not robot.movej(
            pos=target_position,
            vel=velocity,
            acc=acceleration,
            time=0.0,  # 0 means use default time calculation
            move_mode=MOVE_MODE.Absolute,
            blending_radius=0.0,
            blending_type=BLENDING_SPEED_TYPE.Duplicate
        ):
            print("Failed to execute movej command")
            return False

        print("Motion command sent successfully!")

        # Wait for motion to complete
        print("Waiting for motion to complete...")
        time.sleep(5.0)  # Adjust based on expected motion duration

        # Optional: Check current position
        current_pose: ROBOT_POSE = robot.get_current_posj()
        print(f"Current joint position: {current_pose._fPosition}")

        # Step 6: Disable operation (turn off servos) - optional
        # Uncomment if you want to turn off servos after motion
        # print("Disabling robot operation...")
        # robot.set_robot_control(ROBOT_CONTROL.Reset_recovery)

        print("Sample completed successfully!")

    except Exception as e:
        print(f"An error occurred: {e}")
        return False

    finally:
        # Step 7: Close connection
        print("Closing connection...")
        robot.close_connection()

        time.sleep(1.0)
        print("Connection closed.")

    return True

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1)
