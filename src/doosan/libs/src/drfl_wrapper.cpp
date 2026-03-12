#include <cstring>
#define DRCF_VERSION 2

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "../API-DRFL/include/DRFL.h"
//
namespace py = pybind11;

PYBIND11_MODULE(doosan_drfl, m){
    m.doc() = "Python binding for Doosan Robotics API-DRFL using pybind11";

    /*********
     * ENUMS *
     *********/
    // Bind status enum
    py::enum_<ROBOT_STATE>(m, "ROBOT_STATE")
        .value("State_initializing", ROBOT_STATE::STATE_INITIALIZING)
        .value("State_standby", ROBOT_STATE::STATE_STANDBY)
        .value("State_moving", ROBOT_STATE::STATE_MOVING)
        .value("State_safe_off", ROBOT_STATE::STATE_SAFE_OFF)
        .value("State_teaching", ROBOT_STATE::STATE_TEACHING)
        .value("State_safe_stop", ROBOT_STATE::STATE_SAFE_STOP)
        .value("State_emergency_stop", ROBOT_STATE::STATE_EMERGENCY_STOP)
        .value("State_homming", ROBOT_STATE::STATE_HOMMING)
        .value("State_recovery", ROBOT_STATE::STATE_RECOVERY)
        .value("State_safe_stop2", ROBOT_STATE::STATE_SAFE_STOP2)
        .value("State_safe_off2", ROBOT_STATE::STATE_SAFE_OFF2)
        .value("State_reserved1", ROBOT_STATE::STATE_RESERVED1)
        .value("State_reserved2", ROBOT_STATE::STATE_RESERVED2)
        .value("State_reserved3", ROBOT_STATE::STATE_RESERVED3)
        .value("State_reserved4", ROBOT_STATE::STATE_RESERVED4)
        .value("State_not_ready", ROBOT_STATE::STATE_NOT_READY)
        .value("State_last", ROBOT_STATE::STATE_LAST)
        .export_values();

    // used to change the robot control mode.
    py::enum_<ROBOT_CONTROL>(m, "ROBOT_CONTROL")
        .value("Control_init_config", ROBOT_CONTROL::CONTROL_INIT_CONFIG)
        .value("Control_enable_operation", ROBOT_CONTROL::CONTROL_ENABLE_OPERATION)
        .value("Control_reset_safet_stop", ROBOT_CONTROL::CONTROL_RESET_SAFET_STOP)
        .value("Control_reset_safe_stop", ROBOT_CONTROL::CONTROL_RESET_SAFE_STOP)
        .value("Control_reset_safet_off", ROBOT_CONTROL::CONTROL_RESET_SAFET_OFF)
        .value("Control_reset_safe_off", ROBOT_CONTROL::CONTROL_RESET_SAFE_OFF)
        .value("Control_servo_on", ROBOT_CONTROL::CONTROL_SERVO_ON)
        .value("Control_recovery_safe_stop", ROBOT_CONTROL::CONTROL_RECOVERY_SAFE_STOP)
        .value("Control_recovery_safe_off", ROBOT_CONTROL::CONTROL_RECOVERY_SAFE_OFF)
        .value("Control_recovery_backdrive", ROBOT_CONTROL::CONTROL_RECOVERY_BACKDRIVE)
        .value("Control_reset_recovery", ROBOT_CONTROL::CONTROL_RESET_RECOVERY)
        .value("Control_last", ROBOT_CONTROL::CONTROL_LAST)
        .export_values();

    // Set the speed of monitoring
    py::enum_<MONITORING_SPEED>(m, "MONITORING_SPEED")
        .value("Speed_normal_mode", MONITORING_SPEED::SPEED_NORMAL_MODE)
        .value("Speed_reduced_mode", MONITORING_SPEED::SPEED_REDUCED_MODE)
        .export_values();

    // Set the robot system
    py::enum_<ROBOT_SYSTEM>(m, "ROBOT_SYSTEM")
        .value("Robot_system_real", ROBOT_SYSTEM::ROBOT_SYSTEM_REAL)
        .value("Robot_system_virtual", ROBOT_SYSTEM::ROBOT_SYSTEM_VIRTUAL)
        .value("Robot_system_last", ROBOT_SYSTEM::ROBOT_SYSTEM_LAST)
        .export_values();

    // Set or read the mode the robot is in
    py::enum_<ROBOT_MODE>(m, "ROBOT_MODE")
        .value("Robot_mode_manual", ROBOT_MODE::ROBOT_MODE_MANUAL)
        .value("Robot_mode_autonomous", ROBOT_MODE::ROBOT_MODE_AUTONOMOUS)
        .value("Robot_mode_recovery", ROBOT_MODE::ROBOT_MODE_RECOVERY)
        .value("Robot_mode_backdrive", ROBOT_MODE::ROBOT_MODE_BACKDRIVE)
        .value("Robot_mode_measure", ROBOT_MODE::ROBOT_MODE_MEASURE)
        .value("Robot_mode_intialize", ROBOT_MODE::ROBOT_MODE_INITIALIZE)
        .value("robot_mode_last", ROBOT_MODE::ROBOT_MODE_LAST)
        .export_values();

    //
    py::enum_<ROBOT_SPACE>(m, "ROBOT_SPACE")
        .value("Robot_space_joint", ROBOT_SPACE::ROBOT_SPACE_JOINT)
        .value("Robot_space_task", ROBOT_SPACE::ROBOT_SPACE_TASK)
        .export_values();

    //
    py::enum_<SAFE_STOP_RESET_TYPE>(m, "SAFE_STOP_RESET_TYPE")
        .value("Safe_stop_reset_type_default", SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_DEFAULT)
        .value("Safe_stop_reset_type_program_stop", SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_PROGRAM_STOP)
        .value("Safe_stop_reset_type_program_resume", SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_PROGRAM_RESUME)
        .export_values();

    //
    py::enum_<MANAGE_ACCESS_CONTROL>(m, "MANAGE_ACCESS_CONTROL")
        .value("Manage_access_control_force_request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_FORCE_REQUEST)
        .value("Manage_access_control_request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_REQUEST)
        .value("Manage_access_control_response_yes", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_YES)
        .value("Manage_access_control_response_no",
            MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_NO)
        .export_values();

    // Shows the change of control right in the robot controller
    py::enum_<MONITORING_ACCESS_CONTROL>(m, "MONITORING_ACCESS_CONTROL")
        .value("Request", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_REQUEST)
        .value("Deny", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_DENY)
        .value("Grant", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_GRANT)
        .value("Loss", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_LOSS)
        .value("Last", MONITORING_ACCESS_CONTROL::MONITORING_ACCESS_CONTROL_LAST)
        .export_values();

    // Defines coordinate system used by the robot.
    py::enum_<COORDINATE_SYSTEM>(m, "COORDINATE_SYSTEM")
        .value("Coordinate_system_base", COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE)
        .value("Coordinate_system_tool", COORDINATE_SYSTEM::COORDINATE_SYSTEM_TOOL)
        .value("Coordinate_system_world", COORDINATE_SYSTEM::COORDINATE_SYSTEM_WORLD)
        .value("Coordinate_system_user_min", COORDINATE_SYSTEM::COORDINATE_SYSTEM_USER_MIN)
        .value("Coordinate_system_user_max", COORDINATE_SYSTEM::COORDINATE_SYSTEM_USER_MAX)
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

    // Each axis of robot with the standard of joint space coordinate system in the robot controller
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

    //bind GPIO control box digital enums
    py::enum_<GPIO_CTRLBOX_DIGITAL_INDEX>(m, "GPIO_CTRLBOX_DIGITAL_INDEX")
        .value("Index_1", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_1)
        .value("Index_2", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_2)
        .value("Index_3", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_3)
        .value("Index_4", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_4)
        .value("Index_5", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_5)
        .value("Index_6", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_6)
        .value("Index_7", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_7)
        .value("Index_8", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_8)
        .value("Index_9", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_9)
        .value("Index_10", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_10)
        .value("Index_11", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_11)
        .value("Index_12", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_12)
        .value("Index_13", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_13)
        .value("Index_14", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_14)
        .value("Index_15", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_15)
        .value("Index_16", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_16)
        .value("Index_17", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_17)
        .value("Index_18", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_18)
        .value("Index_19", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_19)
        .value("Index_20", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_20)
        .value("Index_21", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_21)
        .value("Index_22", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_22)
        .value("Index_23", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_23)
        .value("Index_24", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_24)
        .value("Index_25", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_25)
        .value("Index_26", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_26)
        .value("Index_27", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_27)
        .value("Index_28", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_28)
        .value("Index_29", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_29)
        .value("Index_30", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_30)
        .value("Index_31", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_31)
        .value("Index_32", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_32)
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
        .value("Discrete_inputs", MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_DISCRETE_INPUTS)
        .value("Coils", MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_COILS)
        .value("Input_register", MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_INPUT_REGISTER)
        .value("Holding_register", MODBUS_REGISTER_TYPE::MODBUS_REGISTER_TYPE_HOLDING_REGISTER)
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

    // Enum to set pop up responce
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

    // Enum to indicat the state of the workpiece of the flange
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

    /**************
     * ATTRABUTES *
     **************/

    m.attr("SPEED_MODE") = m.attr("MONITORING_SPEED");


    /***********
     * STRUCTS *
     ***********/

    py::class_<SYSTEM_VERSION>(m, "SYSTEM_VERSION")
        .def(py::init<>()) // Default constructor
        .def_property("Smart_tp_version", [](const SYSTEM_VERSION &v) {return std::string(v._szSmartTp);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szSmartTp, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szSmartTp[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("SController_version", [](const SYSTEM_VERSION &v) {return std::string(v._szController);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szController, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szController[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("Interpreter_version", [](const SYSTEM_VERSION &v) {return std::string(v._szInterpreter);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szInterpreter, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szInterpreter[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("Inverter_version", [](const SYSTEM_VERSION &v) {return std::string(v._szInverter);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szInverter, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szInverter[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("Safety_board_version", [](const SYSTEM_VERSION &v) {return std::string(v._szSafetyBoard);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szSafetyBoard, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szSafetyBoard[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("Robot_serial", [](const SYSTEM_VERSION &v) {return std::string(v._szRobotSerial);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szRobotSerial, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szRobotSerial[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("Robot_model", [](const SYSTEM_VERSION &v) {return std::string(v._szRobotModel);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szRobotModel, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szRobotModel[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("JTS_board_version", [](const SYSTEM_VERSION &v) {return std::string(v._szJTSBoard);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szJTSBoard, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szJTSBoard[MAX_SYMBOL_SIZE - 1] = '\0';
            })
        .def_property("Flange_board_version", [](const SYSTEM_VERSION &v) {return std::string(v._szFlangeBoard);},
            [](SYSTEM_VERSION &v, const std::string &val) {
                strncpy(v._szFlangeBoard, val.c_str(), MAX_SYMBOL_SIZE - 1);
                v._szFlangeBoard[MAX_SYMBOL_SIZE - 1] = '\0';
            })
            // Optional: Add a __repr__ for easier debugging
        .def("__repr__", [](const SYSTEM_VERSION &v) {
                return "<SystemVersion: " + std::string(v._szRobotModel) + ">";
            });

    py::class_<ROBOT_POSE>(m, "ROBOT_POSE")
        .def(py::init<>())
        .def("get_position_array", [](ROBOT_POSE &v) {
            return py::array_t<float>(NUM_JOINT, v._fPosition);
        })
        .def("set_position_array", [](ROBOT_POSE &v, py::array_t<float> arr) {
            auto buf = arr.request();
            if (buf.size != NUM_JOINT) {
                throw std::runtime_error("Array size must match NUM_JOINT");
            }
            std::memcpy(v._fPosition, buf.ptr, buf.size * sizeof(float));
        });

    py::class_<ROBOT_VEL>(m, "ROBOT_VEL")
        .def(py::init<>())
        .def("get_velocity_array", [](ROBOT_VEL &v) {
            return py::array_t<float>(NUM_JOINT, v._fVelocity);
        })
        .def("set_velocity_array", [](ROBOT_VEL &v, py::array_t<float> arr) {
            auto buf = arr.request();
            if (buf.size != NUM_JOINT) {
                throw std::runtime_error("Array size must match NUM_JOINT");
            }
            std::memcpy(v._fVelocity, buf.ptr, buf.size * sizeof(float));
        });

        py::class_<ROBOT_FORCE>(m, "ROBOT_FORCE")
            .def(py::init<>())
            .def("get_force_array", [](ROBOT_FORCE &v) {
                return py::array_t<float>(NUM_JOINT, v._fForce);
            })
            .def("set_force_array", [](ROBOT_FORCE &v, py::array_t<float> arr) {
                auto buf = arr.request();
                if (buf.size != NUM_JOINT) {
                    throw std::runtime_error("Array size must match NUM_JOINT");
                }
                std::memcpy(v._fForce, buf.ptr, buf.size * sizeof(float));
            });

    py::class_<ROBOT_TASK_POSE>(m, "ROBOT_TASK_POSE")
        .def(py::init<>())
        .def("get_target_pos_array", [](ROBOT_TASK_POSE &v) {
            return py::array_t<float>(NUM_TASK, v._fTargetPos);
        })
        .def("set_target_pos_array", [](ROBOT_TASK_POSE &v, py::array_t<float> arr) {
            auto buf = arr.request();
            if (buf.size != NUM_TASK) {
                throw std::runtime_error("Array size must match NUM_TASK");
            }
            std::memcpy(v._fTargetPos, buf.ptr, buf.size * sizeof(float));
        })
        .def_readwrite("target_sol", &ROBOT_TASK_POSE::_iTargetSol);

    py::class_<LOG_ALARM>(m, "LOG_ALARM")
        .def(py::init<>())
        .def_readwrite("level", &LOG_ALARM::_iLevel)
        .def_readwrite("group", &LOG_ALARM::_iGroup)
        .def_readwrite("index", &LOG_ALARM::_iIndex)
        // 2D Array: Convert to list of strings for reading
        .def_property("params",
            [](const LOG_ALARM &v) {
                std::vector<std::string> result;
                result.reserve(3);
                for (int i = 0; i < 3; ++i) {
                    result.emplace_back(v._szParam[i]); // C-string to std::string
                }
                return result;
            },
            // 2D Array: Convert list of strings to 2D char array for writing
            [](LOG_ALARM &v, const std::vector<std::string> &val) {
                if (val.size() > 3) {
                    throw std::runtime_error("Cannot set more than 3 parameters");
                }
                for (size_t i = 0; i < val.size(); ++i) {
                    // Safe copy with null termination
                    strncpy(v._szParam[i], val[i].c_str(), MAX_STRING_SIZE - 1);
                    v._szParam[i][MAX_STRING_SIZE - 1] = '\0';
                }
                // Clear remaining slots if fewer than 3 were provided
                for (size_t i = val.size(); i < 3; ++i) {
                    v._szParam[i][0] = '\0';
                }
            });

    //bind the CDRFLEx class
    py::class_<DRAFramework::CDRFLEx>(m, "CDRFLEx")
        .def(py::init<>()) // Binds the default constructor

        // Bind methods: .def("python_method_name", &CppClass::CppMethod, "Docstring")
        // Opens intial connection
        .def("open_connection", &DRAFramework::CDRFLEx::open_connection,
             py::arg("ip"), py::arg("port") = 12345,
             "Connect to the Doosan robot controller")

        // Close made connection
        .def("close_connection", &DRAFramework::CDRFLEx::close_connection,
            "Disconnect the Doosan robot controller")

        // Set control mode of the robot
        .def("manage_access_control", &DRAFramework::CDRFLEx::manage_access_control,
            py::arg("access_control"),
            "Manage acces control")

        /*****************
         * GET FUNCTIONS *
         *****************/
        // Get robot state.
        .def("get_robot_state", &DRAFramework::CDRFLEx::get_robot_state,
             "Retrieve the current state of the robot")

        // Get system version
        .def("get_system_version", &DRAFramework::CDRFLEx::get_system_version,
            py::arg("pVersion"),
            "Retrieve the system version")

        // Get library version
        .def("get_library_version", &DRAFramework::CDRFLEx::get_library_version,
            "Retrieve the library version")

        // Get robot mode
        .def("get_robot_mode", &DRAFramework::CDRFLEx::get_robot_mode,
            "Retrieve the robot mode")

        // Get robot speed mode
        .def("get_robot_speed_mode", &DRAFramework::CDRFLEx::get_robot_speed_mode,
            "Retrieve the robot speed mode")

        // Get program state
        .def("get_program_state", &DRAFramework::CDRFLEx::get_program_state,
            "Retrieve the program state")

        // Get robot system
        .def("get_robot_system", &DRAFramework::CDRFLEx::get_robot_system,
            "Retrieve the robot system")

        // Get current pose
        .def("get_current_pose", &DRAFramework::CDRFLEx::get_current_pose,
            py::arg("space_type") = ROBOT_SPACE::ROBOT_SPACE_JOINT,
            "Retrieve the current pose")

        // Get current posj
        .def("get_current_posj", &DRAFramework::CDRFLEx::get_current_posj,
            "Retrieve the current pose in joint space")

        // Get desired posj
        .def("get_desired_posj", &DRAFramework::CDRFLEx::get_desired_posj,
            "Retrieve the desired pose in joint space")

        // Get current joint velocity
        .def("get_current_velj", &DRAFramework::CDRFLEx::get_current_velj,
            "Retrieve the current joint velocity")

        // Get current pos x
        .def("get_current_posx", &DRAFramework::CDRFLEx::get_current_posx,
            py::arg("coodType") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
            "Retrieve the current position in base coordinate system")

        // Get desired pos x
        .def("get_desired_posx", &DRAFramework::CDRFLEx::get_desired_posx,
            py::arg("coodType") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
            "Retrieve the desired position in base coordinate system")

        // Get current tool flange pos x
        .def("get_current_tool_flange_posx",
            &DRAFramework::CDRFLEx::get_current_tool_flange_posx,
            "Retrieve the current tool flange position in base coordinate system")

        // Get current vel x
        .def("get_current_velx", &DRAFramework::CDRFLEx::get_current_velx,
            "Retrieve the current velocity in base coordinate system")

        // Get desired vel x
        .def("get_desired_velx", &DRAFramework::CDRFLEx::get_desired_velx,
            "Retrieve the desired velocity in base coordinate system")

        // Get joint torque
        .def("get_joint_torque", &DRAFramework::CDRFLEx::get_joint_torque,
            "Retrieve the joint torque")

        // Get current control space
        .def("get_control_space", &DRAFramework::CDRFLEx::get_control_space,
            "Retrieve the current control space")

        // Get external torque
        .def("get_external_torque", &DRAFramework::CDRFLEx::get_external_torque,
            "Retrieve the external torque")

        // Get tool force
        .def("get_tool_force", &DRAFramework::CDRFLEx::get_tool_force,
            py::arg("target_ref") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
            "Retrieve the tool force")

        // Get current solution space
        .def("get_current_solution_space", &DRAFramework::CDRFLEx::get_current_solution_space,
            "Retrieve the current solution space")

        // Get last alarm
        .def("get_last_alarm", &DRAFramework::CDRFLEx::get_last_alarm,
            "Retrieve the last alarm")

        // Get solution space
        .def("get_solution_space",
            [](DRAFramework::CDRFLEx& self, const py::array_t<float>& target_pos) -> unsigned char {
                // 1. Get the buffer info
                auto buf = target_pos.request();

                // 2. CRITICAL CHECK: Enforce size constraint
                if (buf.size != NUM_JOINT) {
                    // Raise a Python exception instead of crashing
                    throw py::value_error(
                        "Input array size (" + std::to_string(buf.size) +
                        ") does not match expected NUM_JOINT (" +
                        std::to_string(NUM_JOINT) + ")."
                    );
                }

                // 3. Safe to proceed now
                float* ptr = static_cast<float*>(buf.ptr);
                return self.get_solution_space(ptr);
            },
            py::arg("target_pos"),
            "Function for calculating solution space")

        // Get orientation error
        .def("get_orientation_error",
            [](DRAFramework::CDRFLEx& self,
                const py::array_t<float>& pos1,
                const py::array_t<float>& pos2,
                TASK_AXIS axis) -> float {
                // Validate both arrays
                auto buf1 = pos1.request();
                auto buf2 = pos2.request();

                if (buf1.size != NUM_TASK) {
                    throw py::value_error(
                        "pos1 size (" + std::to_string(buf1.size) +
                        ") does not match NUM_TASK (" +
                        std::to_string(NUM_TASK) + ")"
                    );
                }

                if (buf2.size != NUM_TASK) {
                    throw py::value_error(
                        "pos2 size (" + std::to_string(buf2.size) +
                        ") does not match NUM_TASK (" +
                        std::to_string(NUM_TASK) + ")"
                    );
                }

                // Call original method
                return self.get_orientation_error(
                    static_cast<float*>(buf1.ptr),
                    static_cast<float*>(buf2.ptr),
                    axis
                );
            },
            py::arg("position1"),
            py::arg("position2"),
            py::arg("axis"),
            "Get the orientation error between two positions along a given axis")

        // Get control mode
        .def("get_control_mode", &DRAFramework::CDRFLEx::get_control_mode,
            "Get the current control mode")

        // get current rot matrix
        .def("get_current_rotm",
            [](DRAFramework::CDRFLEx& self, COORDINATE_SYSTEM eTargetRef = COORDINATE_SYSTEM_BASE) {
                // Call the original function
                float (*ptr)[3] = self.get_current_rotm(eTargetRef);

                // Copy the data into a numpy array
                // IMPORTANT: We copy because the original pointer may become invalid
                py::array_t<float> result(3);
                auto buf = result.request();
                float* result_ptr = static_cast<float*>(buf.ptr);

                // Copy 3 elements
                for (int i = 0; i < 3; i++) {
                    result_ptr[i] = (*ptr)[i];
                }

                return result;
            },
            py::arg("eTargetRef") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
            "Get the current rotation matrix")

        /*****************
         * SET FUNCTIONS *
         *****************/

        // Set robot mode
        .def("set_robot_mode", &DRAFramework::CDRFLEx::set_robot_mode,
            py::arg("robot_mode"),
            "Set the robot mode")

        // Set robot control mode
        .def("set_robot_control", &DRAFramework::CDRFLEx::set_robot_control,
            py::arg("robot_control"),
            "Set the robot control mode")

        // Set robot system
        .def("set_robot_system", &DRAFramework::CDRFLEx::set_robot_system,
            py::arg("system_system"),
            "Set the robot system version")

        // Set robot speed mode
        .def("set_robot_speed_mode", &DRAFramework::CDRFLEx::set_robot_speed_mode,
            py::arg("speed_mode"),
            "Set the robot speed mode")

        // Set safe stop reset type
        .def("set_safe_stop_reset_type", &DRAFramework::CDRFLEx::set_safe_stop_reset_type,
            py::arg("reset_type") = SAFE_STOP_RESET_TYPE::SAFE_STOP_RESET_TYPE_DEFAULT,
            "Set the safe stop reset type")

        // Example of binding a method with arguments float*, float, float, float, MOVE_MODE eMoveMode = MOVE_MODE_ABSOLUTE, float fBlendingRadius = 0.f, BLENDING_SPEED_TYPE eBlendingType = BLENDING_SPEED_TYPE_DUPLICATE
        //
        // float fTargetPos[NUM_JOINT],
        // float fTargetVel,
        // float fTargetAcc,
        // float fTargetTime = 0.f,
        // MOVE_MODE eMoveMode = MOVE_MODE_ABSOLUTE,
        // float fBlendingRadius = 0.f,
        // BLENDING_SPEED_TYPE eBlendingType = BLENDING_SPEED_TYPE_DUPLICATE
        .def("movej", py::overload_cast<float*, float, float, float, MOVE_MODE, float, BLENDING_SPEED_TYPE>(&DRAFramework::CDRFLEx::movej),
             py::arg("pos"), py::arg("vel"), py::arg("acc"), py::arg("time"), py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE, py::arg("r") = 0.0, py::arg("blend_speed_type") = BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
             "Joint space motion")

        /*
         * Basic I/O functions
         */

        // Set ctrlbox output status
        .def("set_digital_output", &DRAFramework::CDRFLEx::set_digital_output,
            py::arg("GPIO_index"), py::arg("set"),
            "Set ctrlbox output status")

        // Get ctrlbox input status
        .def("get_digital_output", &DRAFramework::CDRFLEx::get_digital_output,
            py::arg("GPIO_index"),
            "Get ctrlbox output status");

}
