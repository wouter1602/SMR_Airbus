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
POLL_INTERVAL = 0.01  # seconds

# Tool configuration
TOOL_SHAPE_NAME = "Tool_Shape"
TOOL_NAME = "Tool#20"
# TOOL_WEIGHT = 4.430 #kg
# TOOL_CENTER = np.array([23.610, -4.710, 73.710], dtype=np.float32) # Cz, Cy, Cz
# TOOL_INERTIA = np.array([0.00, 0.00, 0.00, 0.00, 0.00, 0.00], dtype=np.float32) # Ixx, Iyy, Izz, Ixy, Iyz, Izx
TOOL_WEIGHT = 4.660 #kg
TOOL_CENTER = np.array([21.670, -2.260, 51.880], dtype=np.float32) # Cz, Cy, Cz
TOOL_INERTIA = np.array([0.00, 0.00, 0.00, 0.00, 0.00, 0.00], dtype=np.float32) # Ixx, Iyy, Izz, Ixy, Iyz, Izx


MAX_FORCE = 5.5 #5.5

# Compliance control — stiffness per axis [X, Y, Z, Rx, Ry, Rz]
# COMPLIANCE_STIFFNESS = np.array([500.0, 500.0, 500.0, 100.0, 100.0, 100.0], dtype=np.float32)
COMPLIANCE_STIFFNESS = np.array([2500.0, 2500.0, 1500.0, 200.0, 200.0, 200.0], dtype=np.float32)

# Force probe — apply 5N in -Z direction only
FORCE_PROBE_FD  = np.array([0.0, 0.0, 5.0, 0.0, 0.0, 0.0], dtype=np.float32)  # desired force vector
FORCE_PROBE_DIR = np.array([0,   0,    1,   0,   0,   0  ], dtype=np.uint8)     # 1 = axis active


class RobotController:

    def __init__(
        self,
        ip_address: str,
        port: int,
        speed: float = 20.0,
        acceleration: float = 10.0,
        lin_speed: float = 60.0,
        lin_acceleration: float = 35.0,
        force_speed: float = 5.0,
        force_acceleration: float = 5.0,
    ) -> None:
        self.ip_address = ip_address
        self.port = port
        self.speed = speed
        self.acceleration = acceleration
        self.lin_speed = lin_speed
        self.lin_acceleration = lin_acceleration
        self.force_speed = force_speed
        self.force_acceleration = force_acceleration

        self.robot = drfl.CDRFLEx()

        # State flags updated by callbacks
        self.get_control_access: bool = False
        self.is_in_standby: bool = False
        self.is_in_safe_off: bool = False

        self.move_handlers = {
            "joint": self._amovej,
            "linear": self._amovel,
        }

        self.in_force_mode: bool = False

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

    def _amovej(self,
        pose: np.ndarray,
        speed: float | None = None,
        acc: float | None = None
    ) -> bool:
        return self.robot.amovej(
            pos=pose,
            vel=speed if speed is not None else self.speed,
            acc=acc if acc is not None else self.acceleration,
            time=0.0,
            move_mode=drfl.MOVE_MODE.Absolute,
            blending_type=drfl.BLENDING_SPEED_TYPE.Duplicate,
        )

    def _amovel(self,
        pose: np.ndarray,
        speed: float | None = None,
        acc: float | None = None
    ) -> bool:
        velocity = np.array([speed if speed is not None else self.lin_speed, 0], dtype=np.float32)
        acceleration = np.array([acc if acc is not None else self.lin_acceleration, 0], dtype=np.float32)

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

    def _enable_compliance(self) -> bool:
        """
        Enable task compliance control with the configured stifness.
        """
        return self.robot.task_compliance_ctrl(
            fTargetStiffness=COMPLIANCE_STIFFNESS,
            eForceReference=drfl.COORDINATE_SYSTEM.Base,
            time=0.0,
        )

    def _disable_compliance(self) -> bool:
        """
        Disable task compliance control.
        """
        self.in_force_mode = False
        return self.robot.release_compliance_ctrl()

    def _amove_force(self, pose: np.ndarray) -> bool:

        time.sleep(0.5)
        self.in_force_mode = True
        return self._amovel(pose, speed=self.force_speed, acc=self.force_acceleration)

    # --- Check for movement ---
    def is_moving(self, timeout: float) -> bool:

        once: bool = False
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

            if self.in_force_mode:
                force: drfl.ROBOT_FORCE = self.robot.get_tool_force(
                    targetRef=drfl.COORDINATE_SYSTEM.Base
                )
                # logger.debug(f"Current Z-force: {force._fForce[2]} N")

                if force._fForce[2] > MAX_FORCE:
                    self.robot.stop(
                        stop_type=drfl.STOP_TYPE.Slow
                    )
                    logger.warning(f"Robot exceeded too much force in Z axis: {force._fForce[2]} N")
                    return True


            time.sleep(POLL_INTERVAL)

    # ── Setup / teardown ──

    def connect(self) -> None:
        """
        Open connection and bring robot to ready state. Raises RuntimeError on failure.
        """
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
                tool = self.robot.get_tool()
                logger.debug(f"failed to set tool: {TOOL_NAME}, current tool: {tool}")
        else:
            logger.debug(f"Failed to create tool with name: {TOOL_NAME}")
            if self.robot.set_tool(TOOL_NAME):
                logger.debug(f"Setting existing tool: {TOOL_NAME}")
            else:
                logger.debug(f"Failed to create and assign tool with name: {TOOL_NAME}")
                logger.debug("Continuing with existing tool")

        tool = self.robot.get_tool()
        logger.debug(f"Current tool: {tool}")

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

            if len(command) == 2:
                move_type, pose_array = command
                output = None
            elif len(command) == 3:
                move_type, pose_array, output = command
            else:
                continue

            if move_type == "toggle_air":

                if output is None:

                    if self.robot.get_digital_output(pose_array):
                        self.robot.set_digital_output(pose_array, False)
                    else:
                        self.robot.set_digital_output(pose_array, True)
                else:
                    self.robot.set_digital_output(pose_array, output)
                result_queue.put("done")
                continue

            if move_type == "force":
                if not self._enable_compliance():
                    result_queue.put("error:Failed to enable compliance control")
                    continue

                if not self._amove_force(pose_array):
                    self._disable_compliance()
                    result_queue.put("error:Force move command failed")
                    continue

                if not self.is_moving(MOVE_TIMEOUT):
                    self._disable_compliance()
                    result_queue.put("error:Force movement timed out")
                    continue

                contact_pose: drfl.ROBOT_POSE = self.robot.get_current_pose(
                    drfl.ROBOT_SPACE.Task
                    )
                # contact_pose: drfl.ROBOT_TASK_POSE = self.robot.get_current_posx(drfl.COORDINATE_SYSTEM.Base)
                self._disable_compliance()
                result_queue.put(("force_done", list(contact_pose._fPosition)))
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
    speed: float = 20,
    acceleration: float = 10,
    lin_speed: float = 60,
    lin_acceleration: float = 35,
) -> None:
    """Entry point for mp.Process — instantiates and runs the RobotController."""
    controller = RobotController(ip_address, port,
        speed=speed, acceleration=acceleration,
        lin_speed=lin_speed, lin_acceleration=lin_acceleration)
    controller.run(command_queue, result_queue)
