/**
 * @brief Wraps the API-DRFL library to use in Python
 *
 *
 *
 */

#define DRCF_VERSION 2 // Change this to 3 if you're using a newer controller.

#include "../API-DRFL/include/DRFL.h"
#include "./cdrflex_bindings.hpp"
#include "./drfl_structs.hpp"
#include <cstring>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <stdexcept>
#include <sys/types.h>
//
namespace py = pybind11;
using arr_f = py::array_t<float, py::array::c_style | py::array::forcecast>;

PYBIND11_MODULE(doosan_drfl, m) {
  m.doc() = "Python binding for Doosan Robotics API-DRFL using pybind11";

  /*********
   * ENUMS *
   *********/
  // Bind status enum
  py::enum_<ROBOT_STATE>(m, "ROBOT_STATE")
      .value("Initializing", ROBOT_STATE::STATE_INITIALIZING)
      .value("Standby", ROBOT_STATE::STATE_STANDBY)
      .value("Moving", ROBOT_STATE::STATE_MOVING)
      .value("Safe_off", ROBOT_STATE::STATE_SAFE_OFF)
      .value("Teaching", ROBOT_STATE::STATE_TEACHING)
      .value("Safe_stop", ROBOT_STATE::STATE_SAFE_STOP)
      .value("Emergency_stop", ROBOT_STATE::STATE_EMERGENCY_STOP)
      .value("Homming", ROBOT_STATE::STATE_HOMMING)
      .value("Recovery", ROBOT_STATE::STATE_RECOVERY)
      .value("Safe_stop2", ROBOT_STATE::STATE_SAFE_STOP2)
      .value("Safe_off2", ROBOT_STATE::STATE_SAFE_OFF2)
      .value("Reserved1", ROBOT_STATE::STATE_RESERVED1)
      .value("Reserved2", ROBOT_STATE::STATE_RESERVED2)
      .value("Reserved3", ROBOT_STATE::STATE_RESERVED3)
      .value("Reserved4", ROBOT_STATE::STATE_RESERVED4)
      .value("Not_ready", ROBOT_STATE::STATE_NOT_READY)
      .value("Last", ROBOT_STATE::STATE_LAST)
      .export_values();

  // used to change the robot control mode.
  py::enum_<ROBOT_CONTROL>(m, "ROBOT_CONTROL")
      .value("Init_config", ROBOT_CONTROL::CONTROL_INIT_CONFIG)
      .value("Operation", ROBOT_CONTROL::CONTROL_ENABLE_OPERATION)
      .value("Reset_safet_stop", ROBOT_CONTROL::CONTROL_RESET_SAFET_STOP)
      .value("Reset_safe_stop", ROBOT_CONTROL::CONTROL_RESET_SAFE_STOP)
      .value("Reset_safet_off", ROBOT_CONTROL::CONTROL_RESET_SAFET_OFF)
      .value("Reset_safe_off", ROBOT_CONTROL::CONTROL_RESET_SAFE_OFF)
      .value("Servo_on", ROBOT_CONTROL::CONTROL_SERVO_ON)
      .value("Recovery_safe_stop", ROBOT_CONTROL::CONTROL_RECOVERY_SAFE_STOP)
      .value("Recovery_safe_off", ROBOT_CONTROL::CONTROL_RECOVERY_SAFE_OFF)
      .value("Recovery_backdrive", ROBOT_CONTROL::CONTROL_RECOVERY_BACKDRIVE)
      .value("Reset_recovery", ROBOT_CONTROL::CONTROL_RESET_RECOVERY)
      .value("Last", ROBOT_CONTROL::CONTROL_LAST)
      .export_values();

  // Set the speed of monitoring
  py::enum_<MONITORING_SPEED>(m, "MONITORING_SPEED")
      .value("Normal_mode", MONITORING_SPEED::SPEED_NORMAL_MODE)
      .value("Reduced_mode", MONITORING_SPEED::SPEED_REDUCED_MODE)
      .export_values();

  // Set the robot system
  py::enum_<ROBOT_SYSTEM>(m, "ROBOT_SYSTEM")
      .value("Real", ROBOT_SYSTEM::ROBOT_SYSTEM_REAL)
      .value("Virtual", ROBOT_SYSTEM::ROBOT_SYSTEM_VIRTUAL)
      .value("Last", ROBOT_SYSTEM::ROBOT_SYSTEM_LAST)
      .export_values();

  // Set or read the mode the robot is in
  py::enum_<ROBOT_MODE>(m, "ROBOT_MODE")
      .value("Manual", ROBOT_MODE::ROBOT_MODE_MANUAL)
      .value("Autonomous", ROBOT_MODE::ROBOT_MODE_AUTONOMOUS)
      .value("Recovery", ROBOT_MODE::ROBOT_MODE_RECOVERY)
      .value("Backdrive", ROBOT_MODE::ROBOT_MODE_BACKDRIVE)
      .value("Measure", ROBOT_MODE::ROBOT_MODE_MEASURE)
      .value("Intialize", ROBOT_MODE::ROBOT_MODE_INITIALIZE)
      .value("Last", ROBOT_MODE::ROBOT_MODE_LAST)
      .export_values();

  //
  py::enum_<ROBOT_SPACE>(m, "ROBOT_SPACE")
      .value("Joint", ROBOT_SPACE::ROBOT_SPACE_JOINT)
      .value("Task", ROBOT_SPACE::ROBOT_SPACE_TASK)
      .export_values();

  //
  py::enum_<SAFE_STOP_RESET_TYPE>(m, "SAFE_STOP_RESET_TYPE")
      .value("Default", SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_DEFAULT)
      .value("Stop", SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_PROGRAM_STOP)
      .value("Resume",
             SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_PROGRAM_RESUME)
      .export_values();

  //
  py::enum_<MANAGE_ACCESS_CONTROL>(m, "MANAGE_ACCESS_CONTROL")
      .value("Force_request",
             MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_FORCE_REQUEST)
      .value("Request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_REQUEST)
      .value("Response_yes",
             MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_YES)
      .value("Response_no",
             MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_NO)
      .export_values();

  // Shows the change of control right in the robot controller
  py::enum_<MONITORING_ACCESS_CONTROL>(m, "MONITORING_ACCESS_CONTROL")
      .value("Request",
             MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_REQUEST)
      .value("Deny", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_DENY)
      .value("Grant",
             MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_GRANT)
      .value("Loss", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_LOSS)
      .value("Last", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_LAST)
      .export_values();

  // Defines coordinate system used by the robot.
  py::enum_<COORDINATE_SYSTEM>(m, "COORDINATE_SYSTEM")
      .value("Coordinate_system_base",
             COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE)
      .value("Coordinate_system_tool",
             COORDINATE_SYSTEM::COORDINATE_SYSTEM_TOOL)
      .value("Coordinate_system_world",
             COORDINATE_SYSTEM::COORDINATE_SYSTEM_WORLD)
      .value("Coordinate_system_user_min",
             COORDINATE_SYSTEM::COORDINATE_SYSTEM_USER_MIN)
      .value("Coordinate_system_user_max",
             COORDINATE_SYSTEM::COORDINATE_SYSTEM_USER_MAX)
      .export_values();

  // Each axis that executes jog control in the robot controller
  py::enum_<JOG_AXIS>(m, "JOG_AXIS")
      .value("Joint_1", JOG_AXIS::JOG_AXIS_JOINT_1)
      .value("Joint_2", JOG_AXIS::JOG_AXIS_JOINT_2)
      .value("Joint_3", JOG_AXIS::JOG_AXIS_JOINT_3)
      .value("Joint_4", JOG_AXIS::JOG_AXIS_JOINT_4)
      .value("Joint_5", JOG_AXIS::JOG_AXIS_JOINT_5)
      .value("Joint_6", JOG_AXIS::JOG_AXIS_JOINT_6)
      .value("Task_X", JOG_AXIS::JOG_AXIS_TASK_X)
      .value("Task_Y", JOG_AXIS::JOG_AXIS_TASK_Y)
      .value("Task_Z", JOG_AXIS::JOG_AXIS_TASK_Z)
      .value("Task_RX", JOG_AXIS::JOG_AXIS_TASK_RX)
      .value("Task_RY", JOG_AXIS::JOG_AXIS_TASK_RY)
      .value("Task_RZ", JOG_AXIS::JOG_AXIS_TASK_RZ)
      .export_values();

  // Each axis of robot with the standard of joint space coordinate system in
  // the robot controller
  py::enum_<JOINT_AXIS>(m, "JOINT_AXIS")
      .value("Axis_1", JOINT_AXIS::JOINT_AXIS_1)
      .value("Axis_2", JOINT_AXIS::JOINT_AXIS_2)
      .value("Axis_3", JOINT_AXIS::JOINT_AXIS_3)
      .value("Axis_4", JOINT_AXIS::JOINT_AXIS_4)
      .value("Axis_5", JOINT_AXIS::JOINT_AXIS_5)
      .value("Axis_6", JOINT_AXIS::JOINT_AXIS_6)
      .export_values();

  // Motion command axis type enumerated value
  py::enum_<TASK_AXIS>(m, "TASK_AXIS")
      .value("Axis_X", TASK_AXIS::TASK_AXIS_X)
      .value("Axis_Y", TASK_AXIS::TASK_AXIS_Y)
      .value("Axis_Z", TASK_AXIS::TASK_AXIS_Z)
      .export_values();

  // Force command axis type enumerated value
  py::enum_<FORCE_AXIS>(m, "FORCE_AXIS")
      .value("Axis_X", FORCE_AXIS::FORCE_AXIS_X)
      .value("Axis_Y", FORCE_AXIS::FORCE_AXIS_Y)
      .value("Axis_Z", FORCE_AXIS::FORCE_AXIS_Z)
      .value("Axis_A", FORCE_AXIS::FORCE_AXIS_A)
      .value("Axis_B", FORCE_AXIS::FORCE_AXIS_B)
      .value("Axis_C", FORCE_AXIS::FORCE_AXIS_C)
      .export_values();

  //
  py::enum_<MOVE_REFERENCE>(m, "MOVE_REFERENCE")
      .value("Base", MOVE_REFERENCE::MOVE_REFERENCE_BASE)
      .value("Tool", MOVE_REFERENCE::MOVE_REFERENCE_TOOL)
      .value("World", MOVE_REFERENCE::MOVE_REFERENCE_WORLD)
      .value("User_min", MOVE_REFERENCE::MOVE_REFERENCE_USER_MIN)
      .value("User_max", MOVE_REFERENCE::MOVE_REFERENCE_USER_MAX)
      .export_values();

  // Move command mode type
  py::enum_<MOVE_MODE>(m, "MOVE_MODE")
      .value("Absolute", MOVE_MODE::MOVE_MODE_ABSOLUTE)
      .value("Relative", MOVE_MODE::MOVE_MODE_RELATIVE)
      .export_values();

  // Force control mode type
  py::enum_<FORCE_MODE>(m, "FORCE_MODE")
      .value("Absolute", FORCE_MODE::FORCE_MODE_ABSOLUTE)
      .value("Relative", FORCE_MODE::FORCE_MODE_RELATIVE)
      .export_values();

  // Blending velocity type
  py::enum_<BLENDING_SPEED_TYPE>(m, "BLENDING_SPEED_TYPE")
      .value("Duplicate", BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE)
      .value("Override", BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_OVERRIDE)
      .export_values();

  // Motion stop type
  py::enum_<STOP_TYPE>(m, "STOP_TYPE")
      .value("Quick_STO", STOP_TYPE::STOP_TYPE_QUICK_STO)
      .value("Quick", STOP_TYPE::STOP_TYPE_QUICK)
      .value("Slow", STOP_TYPE::STOP_TYPE_SLOW)
      .value("Hold", STOP_TYPE::STOP_TYPE_HOLD)
      .value("Emergency", STOP_TYPE::STOP_TYPE_EMERGENCY)
      .export_values();

  // MoveB blending type
  py::enum_<MOVEB_BLENDING_TYPE>(m, "MOVEB_BLENDING_TYPE")
      .value("Line", MOVEB_BLENDING_TYPE::MOVEB_BLENDING_TYPE_LINE)
      .value("Circle", MOVEB_BLENDING_TYPE::MOVEB_BLENDING_TYPE_CIRLCE)
      .export_values();

  // Spline velocity options
  py::enum_<SPLINE_VELOCITY_OPTION>(m, "SPLINE_VELOCITY_OPTION")
      .value("Default", SPLINE_VELOCITY_OPTION::SPLINE_VELOCITY_OPTION_DEFAULT)
      .value("Const", SPLINE_VELOCITY_OPTION::SPLINE_VELOCITY_OPTION_CONST)
      .export_values();

  // bind GPIO control box digital enums
  py::enum_<GPIO_CTRLBOX_DIGITAL_INDEX>(m, "GPIO_CTRLBOX_DIGITAL_INDEX")
      .value("Index_1",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_1)
      .value("Index_2",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_2)
      .value("Index_3",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_3)
      .value("Index_4",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_4)
      .value("Index_5",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_5)
      .value("Index_6",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_6)
      .value("Index_7",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_7)
      .value("Index_8",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_8)
      .value("Index_9",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_9)
      .value("Index_10",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_10)
      .value("Index_11",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_11)
      .value("Index_12",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_12)
      .value("Index_13",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_13)
      .value("Index_14",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_14)
      .value("Index_15",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_15)
      .value("Index_16",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_16)
      .value("Index_17",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_17)
      .value("Index_18",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_18)
      .value("Index_19",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_19)
      .value("Index_20",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_20)
      .value("Index_21",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_21)
      .value("Index_22",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_22)
      .value("Index_23",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_23)
      .value("Index_24",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_24)
      .value("Index_25",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_25)
      .value("Index_26",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_26)
      .value("Index_27",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_27)
      .value("Index_28",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_28)
      .value("Index_29",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_29)
      .value("Index_30",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_30)
      .value("Index_31",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_31)
      .value("Index_32",
             GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_32)
      .export_values();

  // bind GPIO control box analog enums.
  py::enum_<GPIO_CTRLBOX_ANALOG_INDEX>(m, "GPIO_CTRLBOX_ANALOG_INDEX")
      .value("Index_1", GPIO_CTRLBOX_ANALOG_INDEX::GPIO_CTRLBOX_ANALOG_INDEX_1)
      .value("Index_2", GPIO_CTRLBOX_ANALOG_INDEX::GPIO_CTRLBOX_ANALOG_INDEX_2)
      .export_values();

  // Set GPIO analog type
  py::enum_<GPIO_ANALOG_TYPE>(m, "GPIO_ANALOG_TYPE")
      .value("Current", GPIO_ANALOG_TYPE::GPIO_ANALOG_TYPE_CURRENT)
      .value("Voltage", GPIO_ANALOG_TYPE::GPIO_ANALOG_TYPE_VOLTAGE)
      .export_values();

  // bind GPIO tools digital enums.
  py::enum_<GPIO_TOOL_DIGITAL_INDEX>(m, "GPIO_TOOL_DIGITAL_INDEX")
      .value("Index_1", GPIO_TOOL_DIGITAL_INDEX::GPIO_TOOL_DIGITAL_INDEX_1)
      .value("Index_2", GPIO_TOOL_DIGITAL_INDEX::GPIO_TOOL_DIGITAL_INDEX_2)
      .value("Index_3", GPIO_TOOL_DIGITAL_INDEX::GPIO_TOOL_DIGITAL_INDEX_3)
      .value("Index_4", GPIO_TOOL_DIGITAL_INDEX::GPIO_TOOL_DIGITAL_INDEX_4)
      .value("Index_5", GPIO_TOOL_DIGITAL_INDEX::GPIO_TOOL_DIGITAL_INDEX_5)
      .value("Index_6", GPIO_TOOL_DIGITAL_INDEX::GPIO_TOOL_DIGITAL_INDEX_6)
      .export_values();

  // Modbus register type enums
  py::enum_<MODBUS_REGISTER_TYPE>(m, "MODBUS_REGISTER_TYPE")
      .value("Discrete_inputs",
             MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_DISCRETE_INPUTS)
      .value("Coils", MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_COILS)
      .value("Input_register",
             MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_INPUT_REGISTER)
      .value("Holding_register",
             MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_HOLDING_REGISTER)
      .export_values();

  // Execution state of the program enum
  py::enum_<DRL_PROGRAM_STATE>(m, "DRL_PROGRAM_STATE")
      .value("Play", DRL_PROGRAM_STATE::DRL_PROGRAM_STATE_PLAY)
      .value("Stop", DRL_PROGRAM_STATE::DRL_PROGRAM_STATE_STOP)
      .value("Hold", DRL_PROGRAM_STATE::DRL_PROGRAM_STATE_HOLD)
      .value("Last", DRL_PROGRAM_STATE::DRL_PROGRAM_STATE_LAST)
      .export_values();

  // Program response why it stopped.
  py::enum_<PROGRAM_STOP_CAUSE>(m, "PROGRAM_STOP_CAUSE")
      .value("Normal", PROGRAM_STOP_CAUSE::PROGRAM_STOP_CAUSE_NORMAL)
      .value("Force", PROGRAM_STOP_CAUSE::PROGRAM_STOP_CAUSE_FORCE)
      .value("Error", PROGRAM_STOP_CAUSE::PROGRAM_STOP_CAUSE_ERROR)
      .value("Last", PROGRAM_STOP_CAUSE::PROGRAM_STOP_CAUSE_LAST)
      .export_values();

  py::enum_<CONTROL_MODE>(m, "CONTROL_MODE")
      .value("position", CONTROL_MODE::CONTROL_MODE_POSITION)
      .value("torque", CONTROL_MODE::CONTROL_MODE_TORQUE)
      .export_values();

  // Enum for datatype
  py::enum_<DATA_TYPE>(m, "DATA_TYPE")
      .value("Bool", DATA_TYPE::DATA_TYPE_BOOL)
      .value("Int", DATA_TYPE::DATA_TYPE_INT)
      .value("Float", DATA_TYPE::DATA_TYPE_FLOAT)
      .value("String", DATA_TYPE::DATA_TYPE_STRING)
      .value("PosJ", DATA_TYPE::DATA_TYPE_POSJ)
      .value("PosX", DATA_TYPE::DATA_TYPE_POSX)
      .value("Unknown", DATA_TYPE::DATA_TYPE_UNKNOWN)
      .export_values();

  // Enum for variable type
  py::enum_<VARIABLE_TYPE>(m, "VARIABLE_TYPE")
      .value("Install", VARIABLE_TYPE::VARIABLE_TYPE_INSTALL)
      .value("Global", VARIABLE_TYPE::VARIABLE_TYPE_GLOBAL)
      .export_values();

  // Enum for sub program
  py::enum_<SUB_PROGRAM>(m, "SUB_PROGRAM")
      .value("Delete", SUB_PROGRAM::SUB_PROGRAM_DELETE)
      .value("Save", SUB_PROGRAM::SUB_PROGRAM_SAVE)
      .export_values();

  // Enum for singularity avoidance
  py::enum_<SINGULARITY_AVOIDANCE>(m, "SINGULARITY_AVOIDANCE")
      .value("Avoid", SINGULARITY_AVOIDANCE::SINGULARITY_AVOIDANCE_AVOID)
      .value("Stop", SINGULARITY_AVOIDANCE::SINGULARITY_AVOIDANCE_STOP)
      .value("Vel", SINGULARITY_AVOIDANCE::SINGULARITY_AVOIDANCE_VEL)
      .export_values();

  // Enum to set log level
  py::enum_<MESSAGE_LEVEL>(m, "MESSAGE_LEVEL")
      .value("Info", MESSAGE_LEVEL::MESSAGE_LEVEL_INFO)
      .value("Warn", MESSAGE_LEVEL::MESSAGE_LEVEL_WARN)
      .value("Alarm", MESSAGE_LEVEL::MESSAGE_LEVEL_ALARM)
      .export_values();

  // Enum to set pop up response
  py::enum_<POPUP_RESPONSE>(m, "POPUP_RESPONSE")
      .value("Stop", POPUP_RESPONSE::POPUP_RESPONSE_STOP)
      .value("Resume", POPUP_RESPONSE::POPUP_RESPONSE_RESUME)
      .export_values();

  // Enum to move home
  py::enum_<MOVE_HOME>(m, "MOVE_HOME")
      .value("Mechanic", MOVE_HOME::MOVE_HOME_MECHANIC)
      .value("User", MOVE_HOME::MOVE_HOME_USER)
      .export_values();

  // Enum about the byte size
  py::enum_<BYTE_SIZE>(m, "BYTE_SIZE")
      .value("FiveBites", BYTE_SIZE::BYTE_SIZE_FIVEBITES)
      .value("SixBites", BYTE_SIZE::BYTE_SIZE_SIXBITS)
      .value("SevenBites", BYTE_SIZE::BYTE_SIZE_SEVENBITS)
      .value("EightBites", BYTE_SIZE::BYTE_SIZE_EIGHTBITS)
      .export_values();

  // Enum to set stopbits for serial communication
  py::enum_<STOP_BITS>(m, "STOP_BITS")
      .value("One", STOP_BITS::STOPBITS_ONE)
      .value("Two", STOP_BITS::STOPBITS_TWO)
      .export_values();

  // Enum to set parity for serial communication
  py::enum_<PARITY_CHECK>(m, "PARITY_CHECK")
      .value("none", PARITY_CHECK::PARITY_CHECK_NONE)
      .value("even", PARITY_CHECK::PARITY_CHECK_EVEN)
      .value("odd", PARITY_CHECK::PARITY_CHECK_ODD)
      .export_values();

  // Enum to set release mode
  py::enum_<RELEASE_MODE>(m, "RELEASE_MODE")
      .value("Stop", RELEASE_MODE::RELEASE_MODE_STOP)
      .value("Resume", RELEASE_MODE::RELEASE_MODE_RESUME)
      .value("Release", RELEASE_MODE::RELEASE_MODE_RELEASE)
      .value("Reset", RELEASE_MODE::RELEASE_MODE_RESET)
      .export_values();

  // Enum to set safety mode
  py::enum_<SAFETY_MODE>(m, "SAFETY_MODE")
      .value("Manual", SAFETY_MODE::SAFETY_MODE_MANUAL)
      .value("Autonomous", SAFETY_MODE::SAFETY_MODE_AUTONOMOUS)
      .value("Recovery", SAFETY_MODE::SAFETY_MODE_RECOVERY)
      .value("Backdrive", SAFETY_MODE::SAFETY_MODE_BACKDRIVE)
      .value("Measure", SAFETY_MODE::SAFETY_MODE_MEASURE)
      .value("Initialize", SAFETY_MODE::SAFETY_MODE_INITIALIZE)
      .value("Last", SAFETY_MODE::SAFETY_MODE_LAST)
      .export_values();

  // Enum to get safety state
  py::enum_<SAFETY_STATE>(m, "SAFETY_STATE")
      .value("BP_start", SAFETY_STATE::SAFETY_STATE_BP_START)
      .value("BP_init", SAFETY_STATE::SAFETY_STATE_BP_INIT)
      .value("VD_STO", SAFETY_STATE::SAFETY_STATE_VD_STO)
      .value("VD_SOS", SAFETY_STATE::SAFETY_STATE_VD_SOS)
      .value("JH_SOS", SAFETY_STATE::SAFETY_STATE_JH_SOS)
      .value("JH_MOVE", SAFETY_STATE::SAFETY_STATE_JH_MOVE)
      .value("HG_MOVE", SAFETY_STATE::SAFETY_STATE_HG_MOVE)
      .value("RV_SOS", SAFETY_STATE::SAFETY_STATE_RV_SOS)
      .value("RV_MOVE", SAFETY_STATE::SAFETY_STATE_RV_MOVE)
      .value("RV_BACK", SAFETY_STATE::SAFETY_STATE_RV_BACK)
      .value("RV_HG_MOVE", SAFETY_STATE::SAFETY_STATE_RV_HG_MOVE)
      .value("SW_SOS", SAFETY_STATE::SAFETY_STATE_SW_SOS)
      .value("SW_RUN", SAFETY_STATE::SAFETY_STATE_SW_RUN)
      .value("CW_SOS", SAFETY_STATE::SAFETY_STATE_CW_SOS)
      .value("CW_RUN", SAFETY_STATE::SAFETY_STATE_CW_RUN)
      .value("CM_RUN", SAFETY_STATE::SAFETY_STATE_CM_RUN)
      .value("AM_RUN", SAFETY_STATE::SAFETY_STATE_AM_RUN)
      .value("DRL_JH_SOS", SAFETY_STATE::SAFETY_STATE_DRL_JH_SOS)
      .value("DRL_HG_MOVE", SAFETY_STATE::SAFETY_STATE_DRL_HG_MOVE)
      .value("Last", SAFETY_STATE::SAFETY_STATE_LAST)
      .export_values();

  // Enum to get safety mode event
  py::enum_<SAFETY_MODE_EVENT>(m, "SAFETY_MODE_EVENT")
      .value("Enter", SAFETY_MODE_EVENT::SAFETY_MODE_EVENT_ENTER)
      .value("Move", SAFETY_MODE_EVENT::SAFETY_MODE_EVENT_MOVE)
      .value("Stop", SAFETY_MODE_EVENT::SAFETY_MODE_EVENT_STOP)
      .value("Last", SAFETY_MODE_EVENT::SAFETY_MODE_EVENT_LAST)
      .export_values();

  // Enum to set COG reference
  py::enum_<COG_REFERENCE>(m, "COG_REFERENCE")
      .value("TCP", COG_REFERENCE::COG_REFERENCE_TCP)
      .value("Flange", COG_REFERENCE::COG_REFERENCE_FLANGE)
      .export_values();

  // Enum to indicate the state of the workpiece of the flange
  py::enum_<ADD_UP>(m, "ADD_UP")
      .value("Replace", ADD_UP::ADD_UP_REPLACE)
      .value("Add", ADD_UP::ADD_UP_ADD)
      .value("Remove", ADD_UP::ADD_UP_REMOVE)
      .export_values();

  // Enum to set output type
  py::enum_<OUTPUT_TYPE>(m, "OUTPUT_TYPE")
      .value("PNP", OUTPUT_TYPE::OUTPUT_TYPE_PNP)
      .value("NPN", OUTPUT_TYPE::OUTPUT_TYPE_NPN)
      .export_values();

  // Enum to set log level
  py::enum_<LOG_LEVEL>(m, "LOG_LEVEL")
      .value("Sys_info", LOG_LEVEL::LOG_LEVEL_SYSINFO)
      .value("Sys_warn", LOG_LEVEL::LOG_LEVEL_SYSWARN)
      .value("Sys error", LOG_LEVEL::LOG_LEVEL_SYSERROR)
      .value("Sys_last", LOG_LEVEL::LOG_LEVEL_LAST)
      .export_values();

  // Enum for log group
  py::enum_<LOG_GROUP>(m, "LOG_GROUP")
      .value("SystemFMK", LOG_GROUP::LOG_GROUP_SYSTEMFMK)
      .value("MotionLIB", LOG_GROUP::LOG_GROUP_MOTIONLIB)
      .value("SmartTP", LOG_GROUP::LOG_GROUP_SMARTTP)
      .value("Inverter", LOG_GROUP::LOG_GROUP_INVERTER)
      .value("SafetyController", LOG_GROUP::LOG_GROUP_SAFETYCONTROLLER)
      .value("Last", LOG_GROUP::LOG_GROUP_LAST)
      .export_values();

  // Enum for DR_MV_APP
  py::enum_<DR_MV_APP>(m, "DR_MV_APP")
      .value("None", DR_MV_APP::DR_MV_APP_NONE)
      .value("Weld", DR_MV_APP::DR_MV_APP_WELD)
      .export_values();

  py::enum_<MOVE_ORIENTATION>(m, "MOVE_ORIENTATION")
      .value("Teach", MOVE_ORIENTATION::DR_MV_ORI_TEACH)
      .value("Fixed", MOVE_ORIENTATION::DR_MV_ORI_FIXED)
      .value("Radial", MOVE_ORIENTATION::DR_MV_ORI_RADIAL)
      .value("Intent", MOVE_ORIENTATION::DR_MV_ORI_INTENT)
      .export_values();

  py::enum_<SPIRAL_DIR>(m, "SPIRAL_DIR")
      .value("Outward", SPIRAL_DIR::DR_SPIRAL_OUTWARD)
      .value("Inward", SPIRAL_DIR::DR_SPIRAL_INWARD)
      .export_values();

  py::enum_<ROT_DIR>(m, "ROT_DIR")
      .value("Forward", ROT_DIR::DR_ROT_FORWARD)
      .value("Reverse", ROT_DIR::DR_ROT_REVERSE)
      .export_values();

  py::enum_<PATH_MODE>(m, "PATH_MODE")
      .value("DPOS", PATH_MODE::PATH_MODE_DPOS)
      .value("DVEL", PATH_MODE::PATH_MODE_DVEL)
      .export_values();

  py::enum_<DR_SERVOJ_TYPE>(m, "DR_SERVOJ_TYPE")
      .value("Override", DR_SERVOJ_TYPE::DR_SERVO_OVERRIDE)
      .value("Queue", DR_SERVOJ_TYPE::DR_SERVO_QUEUE)
      .export_values();

  /**************
   * ATTRABUTES *
   **************/

  m.attr("SPEED_MODE") = m.attr("MONITORING_SPEED");

  /***********
   * STRUCTS *
   ***********/
  bind_drfl_structs(m);

  // bind the CDRFLEx class
  py::class_<DRAFramework::CDRFLEx> c(m, "CDRFLEx");
  c.def(py::init<>());
  // Connection functions
  bOpenConnection(c);
  bCloseConnection(c);

  // Robot property functions
  bGetSystemVersion(c);
  bGetLibraryVersion(c);
  bGetRobotMode(c);
  bSetRobotMode(c);
  bGetRobotState(c);
  bSetRobotControl(c);
  bSetRobotSystem(c);
  bGetRobotSpeedMode(c);
  bSetRobotSpeedMode(c);
  bGetProgramState(c);
  bGetRobotSystem(c);
  bSetSafeStopResetType(c);
  bGetCurrentPose(c);
  bGetcurrentPosj(c);
  bGetDesiredPosj(c);
  bGetCurrentVelj(c);
  bGetCurrentPosx(c);
  bGetDesiredPosx(c);
  bGetCurrentToolFlangePosx(c);
  bGetCurrentVelx(c);
  bGetDesiredVelx(c);
  bGetJointTorque(c);
  bGetControlSpace(c);
  bGetExternalTorque(c);
  bGetToolForce(c);
  bGetCurrentSolutionSpace(c);
  bGetLastalarm(c);
  bGetSolutionSpace(c);
  bGetOrientationError(c);
  bGetControlMode(c);
  bGetCurrentRotm(c);
  bGetSafetyConfiguration(c);

  // Robot callback functions
  bSetOnMonitoringState(c);
  bSetOnMonitoringData(c);
  bSetOnMonitoringDataEx(c);
  bSetOnMonitoringCtrlIo(c);
  bSetOnMonitoringCtrlIoEx(c);
  bSetOnMonitoringModbus(c);
  bSetOnLogAlarm(c);
  bSetOnTpPopup(c);
  bSetOnTpLog(c);
  bSetOnTpProgress(c);
  bSetOnTpGetUserInput(c);
  bSetOnMonitoringAccessControl(c);
  bSetOnHommingCompleted(c);
  bSetOnTpInitializingCompleted(c);
  bSetOnMonitoringSpeedMode(c);
  bSetOnMasteringNeed(c);
  bSetOnProgramStopped(c);
  bSetOnDisconnected(c);
  bSetOnMonitoringRobotSystem(c);
  bSetOnMonitoringSafetyState(c);

  // Robot control right functions
  bManageAccessControl(c);

  // Basic cotrol function
  bJog(c);
  bMoveHome(c);

  // Robot motion functions
  bMovej(c);
  bMovel(c);
  bMovejx(c);
  bMovec(c);
  bMovesj(c);
  bMovesx(c);
  bMoveb(c);
  bMoveSpiral(c);
  bMovePeriodic(c);
  bAmovej(c);
  bAmovel(c);
  bAmovec(c);
  bAmovesj(c);
  bAmovesx(c);
  bAmoveb(c);
  bAmoveSpiral(c);
  bAmovePeriodic(c);
  bStop(c);
  bTrans(c);
  bFkin(c);
  bIkine(c);
  bSetRefCoord(c);
  bCheckMotion(c);
  bEnableAlterMotion(c);
  bAlterMotion(c);
  bDisableAlterMotion(c);
  bServoj(c);
  bServol(c);
  bSpeedj(c);
  bSpeedl(c);

  // Robot settings functions
  bAddTool(c);
  bDelTool(c);
  bSetTool(c);
  bGetTool(c);
  bAddTcp(c);
  bDelTcp(c);
  bSetTcp(c);
  bGetTcp(c);
  bSetToolShape(c);
  bGetWorkpieceWeight(c);
  bResetWorkpieceWeight(c);
  bSetSingularityHandling(c);
  bSetupMonitoringVersion(c);
  bConfigProgramWatchVariable(c);
  bSetUserHome(c);
  bServoOff(c);
  bReleaseProtectiveStop(c);
  bChangeCollisionSensitivity(c);
  bSetSafetyMode(c);
  bSetAutoServoOff(c);
  bSetWorkpieceWeight(c);

  // Robot I/O functions
  bSetToolDigitalOutput(c);
  bGetToolDigitalOutput(c);
  bGetToolDigitalInput(c);
  bSetDigitalOutput(c);
  bGetDigitalOutput(c);
  bGetDigitalInput(c);
  bSetModeAnalogInput(c);
  bSetModeAnalogOutput(c);
  bSetAnalogOutput(c);
  bGetAnalogInput(c);
  bAddModbusSignal(c);
  bDelModbusSignal(c);
  bSetModbusOutput(c);
  bGetModbusInput(c);
  bFlangeSerialOpen(c);
  bFlangeSerialClose(c);
  bFlangeSerialWrite(c);
  bGetToolAnalogInput(c);
  bSetToolDigitalOutputLevel(c);
  bSetToolDigitalOutputType(c);
  bSetModeToolAnalogInput(c);

  // Robot program control functions
  bDrlStart(c);
  bDrlStop(c);
  bDrlPause(c);
  bDrlResume(c);
  bChangeOperationSpeed(c);
  bSaveSubProgram(c);
  bTpPopupResponse(c);
  bTpGetUserInputResponse(c);

  // Miscellaneous functions
  bParallelAxis(c);
  bAlignAxis(c);
  bIsDoneBoltTightening(c);
  bTaskComplianceCtrl(c);
  bReleaseComplianceCtrl(c);
  bSetStiffnessx(c);
  bCalcCoord(c);
  bSetUserCartCoord(c);
  bOverwriteUserCartCoord(c);
  bGetUserCartCoord(c);
  bSetDesiredForce(c);
  bReleaseForce(c);
  bCheckPositionConditionAbs(c);
  bCheckPositionConditionRel(c);
  bCheckPositionCondition(c);
  bCheckForceCondition(c);
  bCheckOrientationCondition(c);
  bCoordTransform(c);
  bSetPalletizingMode(c);
  bQueryModbusDataList(c);
}
