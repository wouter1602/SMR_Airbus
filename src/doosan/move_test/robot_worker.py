"""
    Robot controller class.
    Owns the robot connection and executes move commands received via command_queue.
    Sends "ready", "done", or "error:<msg>" strings via result_queue.
    Shuts down cleanly when it receives None on command_queue.
"""

import doosan_drfl as drfl
import multiprocessing as mp
import logging
import time
import numpy as np

logger = logging.getLogger(__name__)

MOVE_TIMEOUT = 60.0   # seconds
POLL_INTERVAL = 0.05  # seconds

# Tool configuration
TOOL_SHAPE_NAME = "Tool_Shape"
TOOL_NAME = "Tool#20"
TOOL_WEIGHT = 4.660 #kg
TOOL_CENTER = np.array([21.670, -2.260, 51.880], dtype=np.float32) # Cz, Cy, Cz
TOOL_INERTIA = np.array([0.00, 0.00, 0.00, 0.00, 0.00, 0.00], dtype=np.float32) # Ixx, Iyy, Izz, Ixy, Iyz, Izx


MAX_FORCE = 30.0


class RobotController:

    def __init__(
        self,
        ip_address: str,
        port: int,
        speed: float = 20.0,
        acceleration: float = 10.0,
        lin_speed: float = 60.0,
        lin_acceleration: float = 35.0,
    ) -> None:
        self.ip_address = ip_address
        self.port = port
        self.speed = speed
        self.acceleration = acceleration
        self.lin_speed = lin_speed
        self.lin_acceleration = lin_acceleration

        self.robot = drfl.CDRFLEx()

        # State flags updated by callbacks
        self.get_control_access: bool = False
        self.is_in_standby: bool = False
        self.is_in_safe_off: bool = False

        self.move_handlers = {
            "joint": self._amovej,
            "linear": self._amovel,
        }

    # ── Callbacks ──

    def _on_monitoring_access_control(self, access: drfl.MONITORING_ACCESS_CONTROL) -> None:
        access_str = drfl.MONITORING_ACCESS_CONTROL(access).name
        logger.debug(f"[on_monitoring_access_control] {access_str} [{access}]")
        if access == drfl.MONITORING_ACCESS_CONTROL.Grant:
            logger.info("Access granted!")
            self.get_control_access = True
        elif access == drfl.MONITORING_ACCESS_CONTROL.Loss:
            logger.info("Access lost!")
            self.get_control_access = False

    def _on_monitoring_state(self, state: drfl.ROBOT_STATE) -> None:
        state_str = drfl.ROBOT_STATE(state).name
        logger.debug(f"[on_monitoring_state] {state_str} [{state}]")
        self.is_in_standby = (state == drfl.ROBOT_STATE.Standby)
        self.is_in_safe_off = (state == drfl.ROBOT_STATE.Safe_off)

    # ── Move commands ──

    def _amovej(self, pose: np.ndarray) -> bool:
        return self.robot.amovej(
            pos=pose,
            vel=self.speed,
            acc=self.acceleration,
            time=0.0,
            move_mode=drfl.MOVE_MODE.Absolute,
            blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate,
        )

    def _amovel(self, pose: np.ndarray) -> bool:
        velocity = np.array([self.lin_speed, 0], dtype=np.float32)
        acceleration = np.array([self.lin_acceleration, 0], dtype=np.float32)
        return self.robot.amovel(
            pos=pose,
            vel=velocity,
            acc=acceleration,
            time=0.0,
            move_mode=drfl.MOVE_MODE.Absolute,
            move_reference=drfl.MOVE_REFERENCE.Base,
            blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate,
            app_type=drfl.DR_MV_APP.NoApp,
        )

    # --- Check for movement ---
    def is_moving(self, timeout: float) -> bool:

        once: bool = False
        counter: int = 0
        time.sleep(0.5)

        now = time.time()

        while True:
            result: int = self.robot.check_motion()
            if result == 0:
                logger.info("Robot is done moving!")
                return True
            if time.time() - now > timeout:
                logger.info("Movement timed out")
                return False
            if not once:
                logger.info(f"Robot is still moving: {result}")
                once = True

            force: drfl.ROBOT_FORCE = self.robot.get_tool_force(
                targetRef=drfl.COORDINATE_SYSTEM.Base
            )

            if force._fForce[3] > MAX_FORCE:
                logger.info(f"Force exceeded: {force._fForce[3]} > {MAX_FORCE}")
                self.robot.stop(
                    stop_type=drfl.STOP_TYPE.Quick
                )
                return True

            if counter >= 1 / POLL_INTERVAL:
                position: drfl.ROBOT_POSE = self.robot.get_current_pose(drfl.ROBOT_SPACE.Joint)

                force_: drfl.ROBOT_FORCE = self.robot.get_tool_force(
                    targetRef=drfl.COORDINATE_SYSTEM.Base
                )

                # logger.info(f"Current pose is: {position._fPosition}")
                logger.info(f"Current force is: {force_._fForce}")
                counter = 0
            else:
                counter = counter + 1
            time.sleep(POLL_INTERVAL)

    # ── Setup / teardown ──

    def connect(self) -> None:
        """Open connection and bring robot to ready state. Raises RuntimeError on failure."""
        self.robot.set_on_monitoring_access_control(self._on_monitoring_access_control)
        self.robot.set_on_monitoring_state(self._on_monitoring_state)

        logger.debug(f"Connecting to robot at {self.ip_address}:{self.port}")
        if not self.robot.open_connection(self.ip_address, port=self.port):
            raise RuntimeError(f"Cannot open connection to robot at {self.ip_address}:{self.port}")

        # Wait for standby or safe off
        start = time.time()
        while not self.is_in_standby and not self.is_in_safe_off:
            if time.time() - start > MOVE_TIMEOUT:
                raise RuntimeError("Timed out waiting for robot ready state")
            time.sleep(POLL_INTERVAL)

        self.robot.setup_monitoring_version(1)

        version: drfl.SYSTEM_VERSION = drfl.SYSTEM_VERSION()
        self.robot.get_system_version(version)
        logger.info(f"Controller (DRCF) version: {version._szController}")
        logger.info(f"Library version: {self.robot.get_library_version()}")

        # Set TCP options
        success_setting_toolname =  self.robot.add_tool(
            strSymbol=TOOL_NAME,
            fWeight=TOOL_WEIGHT,
            fCog=TOOL_CENTER,
            fInertia=TOOL_INERTIA,
        )

        if success_setting_toolname:
            if self.robot.set_tool(TOOL_NAME):
                logger.debug(f"Set tool: {TOOL_NAME}")
            else:
                logger.debug(f"failed to set tool: {TOOL_NAME}")
        else:
            logger.debug(f"Failed to create tool with name: {TOOL_NAME}")

        if self.robot.set_tool_shape(TOOL_SHAPE_NAME):
            logger.debug(f"Set tool shape: {TOOL_SHAPE_NAME}")
        else:
            logger.debug(f"Failed to set tool shape: {TOOL_SHAPE_NAME}")


        # Get access control and set Servo On
        for attempt in range(10):
            logger.debug(f"Attempt {attempt}: gaining access control and setting servo on")
            if not self.get_control_access:
                self.robot.manage_access_control(drfl.MANAGE_ACCESS_CONTROL.Force_request)
                time.sleep(1.0)
                continue
            if not self.is_in_standby:
                self.robot.set_robot_control(drfl.ROBOT_CONTROL.Servo_on)
                time.sleep(2.0)
                continue
            break


        if not (self.get_control_access and self.is_in_standby):
            raise RuntimeError(
                f"Failed to reach intended state — "
                f"access={self.get_control_access}, standby={self.is_in_standby}"
            )

        if not self.robot.set_robot_mode(drfl.ROBOT_MODE.Autonomous):
            raise RuntimeError("Failed setting robot mode to Autonomous")

        if not self.robot.set_robot_system(drfl.ROBOT_SYSTEM.Real):
            raise RuntimeError("Failed setting robot system to Real")



        # if not self.robot.set_tool(TOOL_NAME):
        #     raise RuntimeWarning(f"Failed to load tool with name: {TOOL_NAME}")
        # if not self.robot.set_tool_shape(TOOL_SHAPE_NAME):
        #     raise RuntimeWarning(f"Failed setting robot tool shape: '{TOOL_SHAPE_NAME}'")
        # if not self.robot.set_workpiece_weight(TOOL_WEIGHT_NAME):
        #     raise RuntimeWarning(f"Failed setting robot tool weight: '{TOOL_WEIGHT_NAME}'")

    def disconnect(self) -> None:
        self.robot.del_tool(TOOL_NAME) #Remove tool NEEDS ROBOT_MODE MANUAL
        self.robot.close_connection()
        logger.info("Connection closed")

    # ── Command loop ──

    def run(self, command_queue: mp.Queue, result_queue: mp.Queue) -> None:
        try:
            self.connect()
        except RuntimeError as e:
            result_queue.put(f"error:{e}")
            return

        result_queue.put("ready")

        while True:
            command = command_queue.get()

            if command is None:  # Shutdown signal
                logger.info("Shutdown command received")
                break


            move_type, pose_array = command

            if move_type == "toggle_air":

                if self.robot.get_digital_output(pose_array):
                    self.robot.set_digital_output(pose_array, False)
                else:
                    self.robot.set_digital_output(pose_array, True)
                result_queue.put("done")
                continue

            handler = self.move_handlers.get(move_type)

            if handler is None:
                result_queue.put(f"error:Unknown move type: {move_type}")
                continue

            if not handler(pose_array):
                result_queue.put("error:Move command failed")
                continue

            if not self.is_moving(MOVE_TIMEOUT):
                result_queue.put("error: Movement timed out")
                continue

            result_queue.put("done")

        self.disconnect()


def robot_worker(
    command_queue: mp.Queue,
    result_queue: mp.Queue,
    ip_address: str,
    port: int,
) -> None:
    """Entry point for mp.Process — instantiates and runs the RobotController."""
    controller = RobotController(ip_address, port)
    controller.run(command_queue, result_queue)
