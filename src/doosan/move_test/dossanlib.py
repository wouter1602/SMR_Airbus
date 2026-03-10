#!/usr/bin/env python3
#

import doosan_drfl
import time

def main():
    print ("Initializing Doosan Robot...")

    # Create the robot object directly
    # pybind11 calls `new CDRFLEx()` in C++ under the hood
    robot = doosan_drfl.CDRFLEx()

    ip_address = "192.168.8.50"
    port = 12345

    print(f"Connecting to {ip_address}:{port}...")
    success = robot.open_connection(ip_address, port)

    if success:
        print("Connected successfully!")

        # Call methods cleanly
        state = robot.get_robot_state()
        print(f"Robot State: {state}")

        status = robot.set_digital_output(doosan_drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Gpio_ctrlbox_digital_index_1, True)

        print(f"Io status: {status}")

        status_1 = robot.get_digital_output(doosan_drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Gpio_ctrlbox_digital_index_1)

        print(f"Current status of output 1 is: {status_1}")

        time.sleep(10)

        status = robot.set_digital_output(doosan_drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Gpio_ctrlbox_digital_index_1, False)

        status_1 = robot.get_digital_output(doosan_drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Gpio_ctrlbox_digital_index_1)

        print(f"Current status of output 1 is: {status_1}")

    else:
        print("Connection failed.")

    # close connection

if __name__ == "__main__":
    main()
