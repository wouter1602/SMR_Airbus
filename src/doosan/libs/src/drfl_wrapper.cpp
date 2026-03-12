#define DRCF_VERSION 2


#include <cstring>
#include <sys/types.h>
#include <stdexcept>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include "../API-DRFL/include/DRFL.h"
//
namespace py = pybind11;

// ─────────────────────────────────────────────────────────────────────────────
// Helper: make a NumPy array that is a *copy* of a fixed-size C array member.
// Use this for read access when you want a detached NumPy array.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T, std::size_t N>
py::array_t<T> array_copy(const T (&arr)[N]) {
    auto result = py::array_t<T>(N);
    std::memcpy(result.mutable_data(), arr, sizeof(T) * N);
    return result;
}

// 2-D version
template <typename T, std::size_t R, std::size_t C>
py::array_t<T> array2d_copy(const T (&arr)[R][C]) {
    auto result = py::array_t<T>({(py::ssize_t)R, (py::ssize_t)C});
    std::memcpy(result.mutable_data(), &arr[0][0], sizeof(T) * R * C);
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: set a fixed-size C array member from a NumPy array, with bounds check.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T, std::size_t N>
void array_set(T (&arr)[N], py::array_t<T> src) {
    auto info = src.request();
    if (info.size != static_cast<py::ssize_t>(N))
        throw std::out_of_range(
            "Array size mismatch: expected " + std::to_string(N) +
            ", got " + std::to_string(info.size));
    std::memcpy(arr, info.ptr, sizeof(T) * N);
}

template <typename T, std::size_t R, std::size_t C>
void array2d_set(T (&arr)[R][C], py::array_t<T> src) {
    auto info = src.request();
    if (info.size != static_cast<py::ssize_t>(R * C))
        throw std::out_of_range(
            "2D array size mismatch: expected " + std::to_string(R * C) +
            ", got " + std::to_string(info.size));
    std::memcpy(&arr[0][0], info.ptr, sizeof(T) * R * C);
}

// ─────────────────────────────────────────────────────────────────────────────
// Macro shortcuts
//
// Root cause of the two compiler errors:
//
//  ERROR 1 – STR_PROP / ARRAY_PROP used `auto &s` in the SETTER lambda.
//    pybind11's strip_function_object needs to call decltype(&Lambda::operator())
//    to deduce the function signature.  When operator() is a template (because
//    of `auto` parameters) that decltype is ambiguous → "cannot resolve address
//    of overloaded function".
//    FIX: every lambda parameter must be a fully concrete, non-deduced type.
//    We achieve this with a Struct template parameter on the macro + an explicit
//    cast inside, but the cleanest approach (zero overhead, works with pybind11)
//    is to use typed free-function helpers and wrap them in non-generic lambdas
//    via ARRAY_PROP_T / STR_PROP_T macros that take the struct type explicitly.
//
//  ERROR 2 – ARRAY_PROP setter used `decltype(s.member)` in the lambda
//    parameter type.  When multiple structs have a member with the same name
//    (e.g. `_fTargetPos`, `_iOverride`) the compiler generates lambdas whose
//    mangled names are identical → "mangling conflicts with a previous mangle".
//    FIX: pass the concrete element type (float, unsigned char, …) as an
//    explicit macro argument instead of deducing it from the member.
//
// The macros below require the concrete struct type (S) and element type (T)
// to be supplied explicitly, which gives every lambda a unique, fully-specified
// non-template operator() and avoids all mangling collisions.
// ─────────────────────────────────────────────────────────────────────────────

// 1-D array property: ARRAY_PROP(StructType, member, ElementType)
#define ARRAY_PROP(S, member, T) \
    .def_property(#member, \
        [](const S &s) { return array_copy(s.member); }, \
        [](S &s, py::array_t<T> v) { array_set(s.member, v); })

// 2-D array property: ARRAY2D_PROP(StructType, member, ElementType)
#define ARRAY2D_PROP(S, member, T) \
    .def_property(#member, \
        [](const S &s) { return array2d_copy(s.member); }, \
        [](S &s, py::array_t<T> v) { array2d_set(s.member, v); })

// String (char[]) property: STR_PROP(StructType, member)
#define STR_PROP(S, member) \
    .def_property(#member, \
        [](const S &s) { return std::string(s.member); }, \
        [](S &s, const std::string &v) { \
            std::strncpy(s.member, v.c_str(), sizeof(s.member) - 1); \
            s.member[sizeof(s.member) - 1] = '\0'; \
        })

/* Callback defines */

static std::function<void(const LPMONITORING_DATA_EX)>        g_onMonitoringDataEx;
static std::function<void(const LPMONITORING_CTRLIO_EX)>      g_onMonitoringCtrlIOEx;
static std::function<void(const LPMONITORING_CTRLIO_EX2)>     g_onMonitoringCtrlIOEx2;
static std::function<void(LPMESSAGE_POPUP)>                   g_onTpPopup;
static std::function<void(const std::string&)>                g_onTpLog;   // char[256] mapped to string
static std::function<void(LPMESSAGE_INPUT)>                   g_onTpGetUserInput;
static std::function<void(LPMESSAGE_PROGRESS)>                g_onTpProgress;
static std::function<void(const LPRT_OUTPUT_DATA_LIST)>       g_onRTMonitoringData;
static std::function<void(const SAFETY_STATE)>                g_onMonitoringSafetyState;
static std::function<void(const ROBOT_SYSTEM)>                g_onMonitoringRobotSystem;
static std::function<void(unsigned char)>                     g_onMonitoringSafetyStopType;
static std::function<void(const LPROBOT_WELDING_DATA)>        g_onMonitoringWeldingData;
static std::function<void(const LPMONITORING_ALALOG_WELDING)> g_onMonitoringAnalogWeldingData;
static std::function<void(const LPMONITORING_DIGITAL_WELDING)>g_onMonitoringDigitalWeldingData;
static std::function<void(const ROBOT_STATE)>                 g_onMonitoringState;
static std::function<void(const LPMONITORING_DATA)>           g_onMonitoringData;
static std::function<void(const LPMONITORING_CTRLIO)>         g_onMonitoringCtrlIO;
static std::function<void(const LPMONITORING_MODBUS)>         g_onMonitoringModbus;
static std::function<void(LPLOG_ALARM)>                       g_onLogAlarm;
static std::function<void(const MONITORING_ACCESS_CONTROL)>   g_onMonitoringAccessControl;
static std::function<void()>                                  g_onHommingCompleted;
static std::function<void()>                                  g_onTpInitializingCompleted;
static std::function<void(const PROGRAM_STOP_CAUSE)>          g_onProgramStopped;
static std::function<void(const MONITORING_SPEED)>            g_onMonitoringSpeedMode;
static std::function<void()>                                  g_onMasteringNeed;
static std::function<void()>                                  g_onDisconnected;

// --- C-compatible trampolines (passed to the robot SDK) ---
// Each trampoline forwards native C callbacks into the stored std::function.

static void trampoline_MonitoringDataEx(const LPMONITORING_DATA_EX data)
    { if (g_onMonitoringDataEx) g_onMonitoringDataEx(data); }

static void trampoline_MonitoringCtrlIOEx(const LPMONITORING_CTRLIO_EX data)
    { if (g_onMonitoringCtrlIOEx) g_onMonitoringCtrlIOEx(data); }

static void trampoline_MonitoringCtrlIOEx2(const LPMONITORING_CTRLIO_EX2 data)
    { if (g_onMonitoringCtrlIOEx2) g_onMonitoringCtrlIOEx2(data); }

static void trampoline_TpPopup(LPMESSAGE_POPUP data)
    { if (g_onTpPopup) g_onTpPopup(data); }

static void trampoline_TpLog(const char msg[256])
    { if (g_onTpLog) g_onTpLog(std::string(msg, strnlen(msg, 256))); }

static void trampoline_TpGetUserInput(LPMESSAGE_INPUT data)
    { if (g_onTpGetUserInput) g_onTpGetUserInput(data); }

static void trampoline_TpProgress(LPMESSAGE_PROGRESS data)
    { if (g_onTpProgress) g_onTpProgress(data); }

static void trampoline_RTMonitoringData(const LPRT_OUTPUT_DATA_LIST data)
    { if (g_onRTMonitoringData) g_onRTMonitoringData(data); }

static void trampoline_MonitoringSafetyState(const SAFETY_STATE state)
    { if (g_onMonitoringSafetyState) g_onMonitoringSafetyState(state); }

static void trampoline_MonitoringRobotSystem(const ROBOT_SYSTEM system)
    { if (g_onMonitoringRobotSystem) g_onMonitoringRobotSystem(system); }

static void trampoline_MonitoringSafetyStopType(const unsigned char type)
    { if (g_onMonitoringSafetyStopType) g_onMonitoringSafetyStopType(type); }

static void trampoline_MonitoringWeldingData(const LPROBOT_WELDING_DATA data)
    { if (g_onMonitoringWeldingData) g_onMonitoringWeldingData(data); }

static void trampoline_MonitoringAnalogWeldingData(const LPMONITORING_ALALOG_WELDING data)
    { if (g_onMonitoringAnalogWeldingData) g_onMonitoringAnalogWeldingData(data); }

static void trampoline_MonitoringDigitalWeldingData(const LPMONITORING_DIGITAL_WELDING data)
    { if (g_onMonitoringDigitalWeldingData) g_onMonitoringDigitalWeldingData(data); }

static void trampoline_MonitoringState(const ROBOT_STATE state)
    { if (g_onMonitoringState) g_onMonitoringState(state); }

static void trampoline_MonitoringData(const LPMONITORING_DATA data)
    { if (g_onMonitoringData) g_onMonitoringData(data); }

static void trampoline_MonitoringCtrlIO(const LPMONITORING_CTRLIO data)
    { if (g_onMonitoringCtrlIO) g_onMonitoringCtrlIO(data); }

static void trampoline_MonitoringModbus(const LPMONITORING_MODBUS data)
    { if (g_onMonitoringModbus) g_onMonitoringModbus(data); }

static void trampoline_LogAlarm(LPLOG_ALARM data)
    { if (g_onLogAlarm) g_onLogAlarm(data); }

static void trampoline_MonitoringAccessControl(const MONITORING_ACCESS_CONTROL ctrl)
    { if (g_onMonitoringAccessControl) g_onMonitoringAccessControl(ctrl); }

static void trampoline_HommingCompleted()
    { if (g_onHommingCompleted) g_onHommingCompleted(); }

static void trampoline_TpInitializingCompleted()
    { if (g_onTpInitializingCompleted) g_onTpInitializingCompleted(); }

static void trampoline_ProgramStopped(const PROGRAM_STOP_CAUSE cause)
    { if (g_onProgramStopped) g_onProgramStopped(cause); }

static void trampoline_MonitoringSpeedMode(const MONITORING_SPEED speed)
    { if (g_onMonitoringSpeedMode) g_onMonitoringSpeedMode(speed); }

static void trampoline_MasteringNeed()
    { if (g_onMasteringNeed) g_onMasteringNeed(); }

static void trampoline_Disconnected()
    { if (g_onDisconnected) g_onDisconnected(); }


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
        .value("Force_request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_FORCE_REQUEST)
        .value("Request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_REQUEST)
        .value("Response_yes", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_YES)
        .value("Response_no", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_NO)
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

    /**************
     * ATTRABUTES *
     **************/

    m.attr("SPEED_MODE") = m.attr("MONITORING_SPEED");

    /************
     * Calbacks *
     ************/

    m.def("set_on_monitoring_data_ex", [](py::object cb) {
            g_onMonitoringDataEx = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_DATA_EX)>>();
            DRAFramework::TOnMonitoringDataCB(cb.is_none() ? nullptr : trampoline_MonitoringDataEx);
        }, py::arg("callback"), "Register OnMonitoringDataEx callback (or None to clear).");

        m.def("set_on_monitoring_ctrl_io_ex", [](py::object cb) {
            g_onMonitoringCtrlIOEx = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_CTRLIO_EX)>>();
            DRAFramework::TOnMonitoringCtrlIOExCB(cb.is_none() ? nullptr : trampoline_MonitoringCtrlIOEx);
        }, py::arg("callback"));

        m.def("set_on_monitoring_ctrl_io_ex2", [](py::object cb) {
            g_onMonitoringCtrlIOEx2 = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_CTRLIO_EX2)>>();
            DRAFramework::TOnMonitoringCtrlIOEx2CB(cb.is_none() ? nullptr : trampoline_MonitoringCtrlIOEx2);
        }, py::arg("callback"));

        m.def("set_on_tp_popup", [](py::object cb) {
            g_onTpPopup = cb.is_none() ? nullptr : cb.cast<std::function<void(LPMESSAGE_POPUP)>>();
            DRAFramework::TOnTpPopupCB(cb.is_none() ? nullptr : trampoline_TpPopup);
        }, py::arg("callback"));

        // char[256] is exposed as a plain Python str
        m.def("set_on_tp_log", [](py::object cb) {
            g_onTpLog = cb.is_none() ? nullptr : cb.cast<std::function<void(const std::string&)>>();
            DRAFramework::TOnTpLogCB(cb.is_none() ? nullptr : trampoline_TpLog);
        }, py::arg("callback"), "Log message is delivered as a Python str (max 256 chars).");

        m.def("set_on_tp_get_user_input", [](py::object cb) {
            g_onTpGetUserInput = cb.is_none() ? nullptr : cb.cast<std::function<void(LPMESSAGE_INPUT)>>();
            DRAFramework::TOnTpGetUserInputCB(cb.is_none() ? nullptr : trampoline_TpGetUserInput);
        }, py::arg("callback"));

        m.def("set_on_tp_progress", [](py::object cb) {
            g_onTpProgress = cb.is_none() ? nullptr : cb.cast<std::function<void(LPMESSAGE_PROGRESS)>>();
            DRAFramework::TOnTpProgressCB(cb.is_none() ? nullptr : trampoline_TpProgress);
        }, py::arg("callback"));

        m.def("set_on_rt_monitoring_data", [](py::object cb) {
            g_onRTMonitoringData = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPRT_OUTPUT_DATA_LIST)>>();
            DRAFramework::TOnRTMonitoringDataCB(cb.is_none() ? nullptr : trampoline_RTMonitoringData);
        }, py::arg("callback"));

        m.def("set_on_monitoring_safety_state", [](py::object cb) {
            g_onMonitoringSafetyState = cb.is_none() ? nullptr : cb.cast<std::function<void(const SAFETY_STATE)>>();
            DRAFramework::TOnMonitoringSafetyStateCB(cb.is_none() ? nullptr : trampoline_MonitoringSafetyState);
        }, py::arg("callback"));

        m.def("set_on_monitoring_robot_system", [](py::object cb) {
            g_onMonitoringRobotSystem = cb.is_none() ? nullptr : cb.cast<std::function<void(const ROBOT_SYSTEM)>>();
            DRAFramework::TOnMonitoringRobotSystemCB(cb.is_none() ? nullptr : trampoline_MonitoringRobotSystem);
        }, py::arg("callback"));

        m.def("set_on_monitoring_safety_stop_type", [](py::object cb) {
            g_onMonitoringSafetyStopType = cb.is_none() ? nullptr : cb.cast<std::function<void(unsigned char)>>();
            DRAFramework::TOnMonitoringSafetyStopTypeCB(cb.is_none() ? nullptr : trampoline_MonitoringSafetyStopType);
        }, py::arg("callback"), "Stop type is delivered as a Python int (0-255).");

        m.def("set_on_monitoring_welding_data", [](py::object cb) {
            g_onMonitoringWeldingData = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPROBOT_WELDING_DATA)>>();
            DRAFramework::TOnMonitoringWeldingDataCB(cb.is_none() ? nullptr : trampoline_MonitoringWeldingData);
        }, py::arg("callback"));

        m.def("set_on_monitoring_analog_welding_data", [](py::object cb) {
            g_onMonitoringAnalogWeldingData = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_ALALOG_WELDING)>>();
            DRAFramework::TOnMonitoringAnalogWeldingDataCB(cb.is_none() ? nullptr : trampoline_MonitoringAnalogWeldingData);
        }, py::arg("callback"));

        m.def("set_on_monitoring_digital_welding_data", [](py::object cb) {
            g_onMonitoringDigitalWeldingData = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_DIGITAL_WELDING)>>();
            DRAFramework::TOnMonitoringDigitalWeldingDataCB(cb.is_none() ? nullptr : trampoline_MonitoringDigitalWeldingData);
        }, py::arg("callback"));

        m.def("set_on_monitoring_state", [](py::object cb) {
            g_onMonitoringState = cb.is_none() ? nullptr : cb.cast<std::function<void(const ROBOT_STATE)>>();
            DRAFramework::TOnMonitoringStateCB(cb.is_none() ? nullptr : trampoline_MonitoringState);
        }, py::arg("callback"));

        m.def("set_on_monitoring_data", [](py::object cb) {
            g_onMonitoringData = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_DATA)>>();
            DRAFramework::TOnMonitoringDataCB(cb.is_none() ? nullptr : trampoline_MonitoringData);
        }, py::arg("callback"));

        m.def("set_on_monitoring_ctrl_io", [](py::object cb) {
            g_onMonitoringCtrlIO = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_CTRLIO)>>();
            DRAFramework::TOnMonitoringCtrlIOCB(cb.is_none() ? nullptr : trampoline_MonitoringCtrlIO);
        }, py::arg("callback"));

        m.def("set_on_monitoring_modbus", [](py::object cb) {
            g_onMonitoringModbus = cb.is_none() ? nullptr : cb.cast<std::function<void(const LPMONITORING_MODBUS)>>();
            DRAFramework::TOnMonitoringModbusCB(cb.is_none() ? nullptr : trampoline_MonitoringModbus);
        }, py::arg("callback"));

        m.def("set_on_log_alarm", [](py::object cb) {
            g_onLogAlarm = cb.is_none() ? nullptr : cb.cast<std::function<void(LPLOG_ALARM)>>();
            DRAFramework::TOnLogAlarmCB(cb.is_none() ? nullptr : trampoline_LogAlarm);
        }, py::arg("callback"));

        m.def("set_on_monitoring_access_control", [](py::object cb) {
            g_onMonitoringAccessControl = cb.is_none() ? nullptr : cb.cast<std::function<void(const MONITORING_ACCESS_CONTROL)>>();
            DRAFramework::TOnMonitoringAccessControlCB(cb.is_none() ? nullptr : trampoline_MonitoringAccessControl);
        }, py::arg("callback"));

        // No-argument callbacks
        m.def("set_on_homming_completed", [](py::object cb) {
            g_onHommingCompleted = cb.is_none() ? nullptr : cb.cast<std::function<void()>>();
            DRAFramework::TOnHommingCompletedCB(cb.is_none() ? nullptr : trampoline_HommingCompleted);
        }, py::arg("callback"));

        m.def("set_on_tp_initializing_completed", [](py::object cb) {
            g_onTpInitializingCompleted = cb.is_none() ? nullptr : cb.cast<std::function<void()>>();
            DRAFramework::TOnTpInitializingCompletedCB(cb.is_none() ? nullptr : trampoline_TpInitializingCompleted);
        }, py::arg("callback"));

        m.def("set_on_program_stopped", [](py::object cb) {
            g_onProgramStopped = cb.is_none() ? nullptr : cb.cast<std::function<void(const PROGRAM_STOP_CAUSE)>>();
            DRAFramework::TOnProgramStoppedCB(cb.is_none() ? nullptr : trampoline_ProgramStopped);
        }, py::arg("callback"));

        m.def("set_on_monitoring_speed_mode", [](py::object cb) {
            g_onMonitoringSpeedMode = cb.is_none() ? nullptr : cb.cast<std::function<void(const MONITORING_SPEED)>>();
            DRAFramework::TOnMonitoringSpeedModeCB(cb.is_none() ? nullptr : trampoline_MonitoringSpeedMode);
        }, py::arg("callback"));

        m.def("set_on_mastering_need", [](py::object cb) {
            g_onMasteringNeed = cb.is_none() ? nullptr : cb.cast<std::function<void()>>();
            DRAFramework::TOnMasteringNeedCB(cb.is_none() ? nullptr : trampoline_MasteringNeed);
        }, py::arg("callback"));

        m.def("set_on_disconnected", [](py::object cb) {
            g_onDisconnected = cb.is_none() ? nullptr : cb.cast<std::function<void()>>();
            DRAFramework::TOnDisconnectedCB(cb.is_none() ? nullptr : trampoline_Disconnected);
        }, py::arg("callback"));

    /***********
     * STRUCTS *
     ***********/

    // ─────────────────────────────────────────────────────────────────────────
    // SYSTEM_VERSION
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<SYSTEM_VERSION>(m, "SYSTEM_VERSION")
        .def(py::init<>())
        STR_PROP(SYSTEM_VERSION, _szSmartTp)
        STR_PROP(SYSTEM_VERSION, _szController)
        STR_PROP(SYSTEM_VERSION, _szInterpreter)
        STR_PROP(SYSTEM_VERSION, _szInverter)
        STR_PROP(SYSTEM_VERSION, _szSafetyBoard)
        STR_PROP(SYSTEM_VERSION, _szRobotSerial)
        STR_PROP(SYSTEM_VERSION, _szRobotModel)
        STR_PROP(SYSTEM_VERSION, _szJTSBoard)
        STR_PROP(SYSTEM_VERSION, _szFlangeBoard);

        // ─────────────────────────────────────────────────────────────────────────
        // ROBOT_MONITORING_JOINT
        // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_JOINT>(m, "ROBOT_MONITORING_JOINT")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualPos, float)
        ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualAbs, float)
        ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualVel, float)
        ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualErr, float)
        ARRAY_PROP(ROBOT_MONITORING_JOINT, _fTargetPos, float)
        ARRAY_PROP(ROBOT_MONITORING_JOINT, _fTargetVel, float);

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_TASK
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_TASK>(m, "ROBOT_MONITORING_TASK")
        .def(py::init<>())
        .def_property("_fActualPos",
            [](const ROBOT_MONITORING_TASK &s) { return array2d_copy(s._fActualPos); },
            [](ROBOT_MONITORING_TASK &s, py::array_t<float> v) { array2d_set(s._fActualPos, v); })
        ARRAY_PROP(ROBOT_MONITORING_TASK, _fActualVel,  float)
        ARRAY_PROP(ROBOT_MONITORING_TASK, _fActualErr,  float)
        ARRAY_PROP(ROBOT_MONITORING_TASK, _fTargetPos,  float)
        ARRAY_PROP(ROBOT_MONITORING_TASK, _fTargetVel,  float)
        .def_readwrite("_iSolutionSpace", &ROBOT_MONITORING_TASK::_iSolutionSpace)
        .def_property("_fRotationMatrix",
            [](const ROBOT_MONITORING_TASK &s) { return array2d_copy(s._fRotationMatrix); },
            [](ROBOT_MONITORING_TASK &s, py::array_t<float> v) { array2d_set(s._fRotationMatrix, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_WORLD
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_WORLD>(m, "ROBOT_MONITORING_WORLD")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_MONITORING_WORLD, _fActualW2B, float)
        .def_property("_fActualPos",
            [](const ROBOT_MONITORING_WORLD &s) { return array2d_copy(s._fActualPos); },
            [](ROBOT_MONITORING_WORLD &s, py::array_t<float> v) { array2d_set(s._fActualPos, v); })
        ARRAY_PROP(ROBOT_MONITORING_WORLD, _fActualVel,  float)
        ARRAY_PROP(ROBOT_MONITORING_WORLD, _fActualETT,  float)
        ARRAY_PROP(ROBOT_MONITORING_WORLD, _fTargetPos,  float)
        ARRAY_PROP(ROBOT_MONITORING_WORLD, _fTargetVel,  float)
        .def_property("_fRotationMatrix",
            [](const ROBOT_MONITORING_WORLD &s) { return array2d_copy(s._fRotationMatrix); },
            [](ROBOT_MONITORING_WORLD &s, py::array_t<float> v) { array2d_set(s._fRotationMatrix, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_USER
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_USER>(m, "ROBOT_MONITORING_USER")
        .def(py::init<>())
        .def_readwrite("_iActualUCN", &ROBOT_MONITORING_USER::_iActualUCN)
        .def_readwrite("_iParent",    &ROBOT_MONITORING_USER::_iParent)
        .def_property("_fActualPos",
            [](const ROBOT_MONITORING_USER &s) { return array2d_copy(s._fActualPos); },
            [](ROBOT_MONITORING_USER &s, py::array_t<float> v) { array2d_set(s._fActualPos, v); })
        ARRAY_PROP(ROBOT_MONITORING_USER, _fActualVel, float)
        ARRAY_PROP(ROBOT_MONITORING_USER, _fActualETT, float)
        ARRAY_PROP(ROBOT_MONITORING_USER, _fTargetPos, float)
        ARRAY_PROP(ROBOT_MONITORING_USER, _fTargetVel, float)
        .def_property("_fRotationMatrix",
            [](const ROBOT_MONITORING_USER &s) { return array2d_copy(s._fRotationMatrix); },
            [](ROBOT_MONITORING_USER &s, py::array_t<float> v) { array2d_set(s._fRotationMatrix, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_TORQUE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_TORQUE>(m, "ROBOT_MONITORING_TORQUE")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fDynamicTor, float)
        ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fActualJTS,  float)
        ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fActualEJT,  float)
        ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fActualETT,  float);

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_STATE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_STATE>(m, "ROBOT_MONITORING_STATE")
        .def(py::init<>())
        .def_readwrite("_iActualMode",  &ROBOT_MONITORING_STATE::_iActualMode)
        .def_readwrite("_iActualSpace", &ROBOT_MONITORING_STATE::_iActualSpace);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_CONTROL  (typedef of _ROBOT_MONITORING_DATA)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_CONTROL>(m, "MONITORING_CONTROL")
        .def(py::init<>())
        .def_readwrite("_tState",  &MONITORING_CONTROL::_tState)
        .def_readwrite("_tJoint",  &MONITORING_CONTROL::_tJoint)
        .def_readwrite("_tTask",   &MONITORING_CONTROL::_tTask)
        .def_readwrite("_tTorque", &MONITORING_CONTROL::_tTorque);

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_DATA_EX  /  MONITORING_CONTROL_EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_DATA_EX>(m, "ROBOT_MONITORING_DATA_EX")
        .def(py::init<>())
        .def_readwrite("_tState",  &ROBOT_MONITORING_DATA_EX::_tState)
        .def_readwrite("_tJoint",  &ROBOT_MONITORING_DATA_EX::_tJoint)
        .def_readwrite("_tTask",   &ROBOT_MONITORING_DATA_EX::_tTask)
        .def_readwrite("_tTorque", &ROBOT_MONITORING_DATA_EX::_tTorque)
        .def_readwrite("_tWorld",  &ROBOT_MONITORING_DATA_EX::_tWorld)
        .def_readwrite("_tUser",   &ROBOT_MONITORING_DATA_EX::_tUser);
    // Alias
    m.attr("MONITORING_CONTROL_EX") = m.attr("ROBOT_MONITORING_DATA_EX");

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_MISC
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_MISC>(m, "MONITORING_MISC")
        .def(py::init<>())
        .def_readwrite("_dSyncTime", &MONITORING_MISC::_dSyncTime)
        ARRAY_PROP(MONITORING_MISC, _iActualDI, unsigned char)
        ARRAY_PROP(MONITORING_MISC, _iActualDO, unsigned char)
        ARRAY_PROP(MONITORING_MISC, _iActualBK, unsigned char)
        ARRAY_PROP(MONITORING_MISC, _iActualBT, unsigned int)
        ARRAY_PROP(MONITORING_MISC, _fActualMC, float)
        ARRAY_PROP(MONITORING_MISC, _fActualMT, float);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_COCKPIT
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_COCKPIT>(m, "MONITORING_COCKPIT")
        .def(py::init<>())
        ARRAY_PROP(MONITORING_COCKPIT, _iActualBS, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_DATA
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_DATA>(m, "MONITORING_DATA")
        .def(py::init<>())
        .def_readwrite("_tCtrl", &MONITORING_DATA::_tCtrl)
        .def_readwrite("_tMisc", &MONITORING_DATA::_tMisc);

    // ─────────────────────────────────────────────────────────────────────────
    // FLANGE_VERSION
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<FLANGE_VERSION>(m, "FLANGE_VERSION")
        .def(py::init<>())
        .def_readwrite("BoardNo",       &FLANGE_VERSION::BoardNo)
        .def_readwrite("PacketType",    &FLANGE_VERSION::PacketType)
        .def_readwrite("res",           &FLANGE_VERSION::res)
        .def_readwrite("iFlangeHwVer",  &FLANGE_VERSION::iFlangeHwVer);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_FLANGE_IO_CONFIG
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_FLANGE_IO_CONFIG>(m, "MONITORING_FLANGE_IO_CONFIG")
        .def(py::init<>())
        ARRAY_PROP(MONITORING_FLANGE_IO_CONFIG, _iActualAI, float)
        .def_readwrite("_iX1Rs485FAIPinMux",     &MONITORING_FLANGE_IO_CONFIG::_iX1Rs485FAIPinMux)
        .def_readwrite("_iX2Rs485FAIPinMux",     &MONITORING_FLANGE_IO_CONFIG::_iX2Rs485FAIPinMux)
        .def_readwrite("_iX1DOBjtType",          &MONITORING_FLANGE_IO_CONFIG::_iX1DOBjtType)
        .def_readwrite("_iX2DOBjtType",          &MONITORING_FLANGE_IO_CONFIG::_iX2DOBjtType)
        .def_readwrite("_iVoutLevel",            &MONITORING_FLANGE_IO_CONFIG::_iVoutLevel)
        .def_readwrite("_iFAI0Mode",             &MONITORING_FLANGE_IO_CONFIG::_iFAI0Mode)
        .def_readwrite("_iFAI1Mode",             &MONITORING_FLANGE_IO_CONFIG::_iFAI1Mode)
        .def_readwrite("_iFAI2Mode",             &MONITORING_FLANGE_IO_CONFIG::_iFAI2Mode)
        .def_readwrite("_iFAI3Mode",             &MONITORING_FLANGE_IO_CONFIG::_iFAI3Mode)
        .def_readwrite("_szX1DataLength",        &MONITORING_FLANGE_IO_CONFIG::_szX1DataLength)
        .def_readwrite("_szX1Parity",            &MONITORING_FLANGE_IO_CONFIG::_szX1Parity)
        .def_readwrite("_szX1StopBit",           &MONITORING_FLANGE_IO_CONFIG::_szX1StopBit)
        .def_readwrite("_szX2DataLength",        &MONITORING_FLANGE_IO_CONFIG::_szX2DataLength)
        .def_readwrite("_szX2Parity",            &MONITORING_FLANGE_IO_CONFIG::_szX2Parity)
        .def_readwrite("_szX2StopBit",           &MONITORING_FLANGE_IO_CONFIG::_szX2StopBit)
        .def_readwrite("_iServoSafetyMode",      &MONITORING_FLANGE_IO_CONFIG::_iServoSafetyMode)
        .def_readwrite("_iInterruptSafetyMode",  &MONITORING_FLANGE_IO_CONFIG::_iInterruptSafetyMode)
        .def_property("_szX1Baudrate",
            [](const MONITORING_FLANGE_IO_CONFIG &s) { return array_copy(s._szX1Baudrate); },
            [](MONITORING_FLANGE_IO_CONFIG &s, py::array_t<unsigned char> v) { array_set(s._szX1Baudrate, v); })
        .def_property("_szX2Baudrate",
            [](const MONITORING_FLANGE_IO_CONFIG &s) { return array_copy(s._szX2Baudrate); },
            [](MONITORING_FLANGE_IO_CONFIG &s, py::array_t<unsigned char> v) { array_set(s._szX2Baudrate, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_SENSOR
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_SENSOR>(m, "ROBOT_MONITORING_SENSOR")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_MONITORING_SENSOR, _fActualFTS, float)
        ARRAY_PROP(ROBOT_MONITORING_SENSOR, _fActualCS,  float)
        ARRAY_PROP(ROBOT_MONITORING_SENSOR, _fActualACS, float);

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_MONITORING_AMODEL  /  MONITORING_AMODEL
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_MONITORING_AMODEL>(m, "ROBOT_MONITORING_AMODEL")
        .def(py::init<>())
        .def_readwrite("_tSensor",      &ROBOT_MONITORING_AMODEL::_tSensor)
        .def_readwrite("_fSingularity", &ROBOT_MONITORING_AMODEL::_fSingularity);
    m.attr("MONITORING_AMODEL") = m.attr("ROBOT_MONITORING_AMODEL");

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_FORCECONTROL
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_FORCECONTROL>(m, "MONITORING_FORCECONTROL")
        .def(py::init<>())
        ARRAY_PROP(MONITORING_FORCECONTROL, _iActualBS,         unsigned char)
        ARRAY_PROP(MONITORING_FORCECONTROL, _fActualCS,         float)
        .def_readwrite("_fSingularity",          &MONITORING_FORCECONTROL::_fSingularity)
        ARRAY_PROP(MONITORING_FORCECONTROL, _fToolActualETT,    float)
        ARRAY_PROP(MONITORING_FORCECONTROL, _iForceControlMode, unsigned char)
        .def_readwrite("_iReferenceCoord",       &MONITORING_FORCECONTROL::_iReferenceCoord)
        .def_readwrite("_iAutoAccMode",          &MONITORING_FORCECONTROL::_iAutoAccMode)
        ARRAY_PROP(MONITORING_FORCECONTROL, _fActualHDT,        float)
        .def_readwrite("_iSingularHandlingMode", &MONITORING_FORCECONTROL::_iSingularHandlingMode)
        .def_readwrite("_isMoving",              &MONITORING_FORCECONTROL::_isMoving);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_DATA_EX  (union members exposed via named properties)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_DATA_EX>(m, "MONITORING_DATA_EX")
        .def(py::init<>())
        .def_readwrite("_tCtrl", &MONITORING_DATA_EX::_tCtrl)
        .def_readwrite("_tMisc", &MONITORING_DATA_EX::_tMisc)
        // union _tMiscEx – expose named struct member
        .def_property("_tCtrlEx",
            [](const MONITORING_DATA_EX &s) -> const MONITORING_FORCECONTROL & { return s._tMiscEx._tCtrlEx; },
            [](MONITORING_DATA_EX &s, const MONITORING_FORCECONTROL &v) { s._tMiscEx._tCtrlEx = v; })
        // union _tModel
        .def_property("_tAModel",
            [](const MONITORING_DATA_EX &s) -> const MONITORING_AMODEL & { return s._tModel._tAModel; },
            [](MONITORING_DATA_EX &s, const MONITORING_AMODEL &v) { s._tModel._tAModel = v; })
        // union _tFlangeIo
        .def_property("_tConfig",
            [](const MONITORING_DATA_EX &s) -> const MONITORING_FLANGE_IO_CONFIG & { return s._tFlangeIo._tConfig; },
            [](MONITORING_DATA_EX &s, const MONITORING_FLANGE_IO_CONFIG &v) { s._tFlangeIo._tConfig = v; });

    // ─────────────────────────────────────────────────────────────────────────
    // READ_CTRLIO_INPUT
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_CTRLIO_INPUT>(m, "READ_CTRLIO_INPUT")
        .def(py::init<>())
        ARRAY_PROP(READ_CTRLIO_INPUT, _iActualDI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT, _fActualAI, float)
        ARRAY_PROP(READ_CTRLIO_INPUT, _iActualSW, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT, _iActualSI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT, _iActualEI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT, _iAcutualED, unsigned int);

    // ─────────────────────────────────────────────────────────────────────────
    // READ_CTRLIO_OUTPUT
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_CTRLIO_OUTPUT>(m, "READ_CTRLIO_OUTPUT")
        .def(py::init<>())
        ARRAY_PROP(READ_CTRLIO_OUTPUT, _iTargetDO, unsigned char)
        ARRAY_PROP(READ_CTRLIO_OUTPUT, _fTargetAO, float);

    // ─────────────────────────────────────────────────────────────────────────
    // READ_ENCODER_INPUT
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_ENCODER_INPUT>(m, "READ_ENCODER_INPUT")
        .def(py::init<>())
        ARRAY_PROP(READ_ENCODER_INPUT, _iActualES, unsigned char)
        ARRAY_PROP(READ_ENCODER_INPUT, _iActualED, unsigned int)
        ARRAY_PROP(READ_ENCODER_INPUT, _iActualER, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // READ_PROCESS_INPUT
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_PROCESS_INPUT>(m, "READ_PROCESS_INPUT")
        .def(py::init<>())
        ARRAY_PROP(READ_PROCESS_INPUT, _iActualDI, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_CTRLIO
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_CTRLIO>(m, "MONITORING_CTRLIO")
        .def(py::init<>())
        .def_readwrite("_tInput",  &MONITORING_CTRLIO::_tInput)
        .def_readwrite("_tOutput", &MONITORING_CTRLIO::_tOutput);

    // ─────────────────────────────────────────────────────────────────────────
    // READ_CTRLIO_INPUT_EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_CTRLIO_INPUT_EX>(m, "READ_CTRLIO_INPUT_EX")
        .def(py::init<>())
        ARRAY_PROP(READ_CTRLIO_INPUT_EX, _iActualDI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX, _fActualAI, float)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX, _iActualSW, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX, _iActualSI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX, _iActualAT, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // READ_CTRLIO_OUTPUT_EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_CTRLIO_OUTPUT_EX>(m, "READ_CTRLIO_OUTPUT_EX")
        .def(py::init<>())
        ARRAY_PROP(READ_CTRLIO_OUTPUT_EX, _iTargetDO, unsigned char)
        ARRAY_PROP(READ_CTRLIO_OUTPUT_EX, _fTargetAO, float)
        ARRAY_PROP(READ_CTRLIO_OUTPUT_EX, _iTargetAT, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_CTRLIO_EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_CTRLIO_EX>(m, "MONITORING_CTRLIO_EX")
        .def(py::init<>())
        .def_readwrite("_tInput",   &MONITORING_CTRLIO_EX::_tInput)
        .def_readwrite("_tOutput",  &MONITORING_CTRLIO_EX::_tOutput)
        .def_readwrite("_tEncoder", &MONITORING_CTRLIO_EX::_tEncoder);

    // ─────────────────────────────────────────────────────────────────────────
    // READ_CTRLIO_INPUT_EX2 / OUTPUT_EX2 / MONITORING_CTRLIO_EX2
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_CTRLIO_INPUT_EX2>(m, "READ_CTRLIO_INPUT_EX2")
        .def(py::init<>())
        ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualDI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _fActualAI, float)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualSW, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualSI, unsigned char)
        ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualAT, unsigned char);

    py::class_<READ_CTRLIO_OUTPUT_EX2>(m, "READ_CTRLIO_OUTPUT_EX2")
        .def(py::init<>())
        ARRAY_PROP(READ_CTRLIO_OUTPUT_EX2, _iTargetDO, unsigned char)
        ARRAY_PROP(READ_CTRLIO_OUTPUT_EX2, _fTargetAO, float)
        ARRAY_PROP(READ_CTRLIO_OUTPUT_EX2, _iTargetAT, unsigned char);

    py::class_<MONITORING_CTRLIO_EX2>(m, "MONITORING_CTRLIO_EX2")
        .def(py::init<>())
        .def_readwrite("_tInput",   &MONITORING_CTRLIO_EX2::_tInput)
        .def_readwrite("_tOutput",  &MONITORING_CTRLIO_EX2::_tOutput)
        .def_readwrite("_tEncoder", &MONITORING_CTRLIO_EX2::_tEncoder);

    // ─────────────────────────────────────────────────────────────────────────
    // MODBUS_REGISTER
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MODBUS_REGISTER>(m, "MODBUS_REGISTER")
        .def(py::init<>())
        STR_PROP(MODBUS_REGISTER, _szSymbol)
        .def_readwrite("_iValue", &MODBUS_REGISTER::_iValue);

    // ─────────────────────────────────────────────────────────────────────────
    // MONITORING_MODBUS
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MONITORING_MODBUS>(m, "MONITORING_MODBUS")
        .def(py::init<>())
        .def_readwrite("_iRegCount", &MONITORING_MODBUS::_iRegCount)
        .def_property("_tRegister",
            [](const MONITORING_MODBUS &s) {
                py::list lst;
                for (int i = 0; i < MAX_MODBUS_TOTAL_REGISTERS; ++i)
                    lst.append(s._tRegister[i]);
                return lst;
            },
            [](MONITORING_MODBUS &s, py::list lst) {
                if ((int)lst.size() != MAX_MODBUS_TOTAL_REGISTERS)
                    throw std::out_of_range("Expected " + std::to_string(MAX_MODBUS_TOTAL_REGISTERS) + " elements");
                for (int i = 0; i < MAX_MODBUS_TOTAL_REGISTERS; ++i)
                    s._tRegister[i] = lst[i].cast<MODBUS_REGISTER>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // LOG_ALARM / MONITORING_ALARM
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<LOG_ALARM>(m, "LOG_ALARM")
        .def(py::init<>())
        .def_readwrite("_iLevel", &LOG_ALARM::_iLevel)
        .def_readwrite("_iGroup", &LOG_ALARM::_iGroup)
        .def_readwrite("_iIndex", &LOG_ALARM::_iIndex)
        .def_property("_szParam",
            [](const LOG_ALARM &s) {
                py::list lst;
                for (int i = 0; i < 3; ++i)
                    lst.append(std::string(s._szParam[i]));
                return lst;
            },
            [](LOG_ALARM &s, py::list lst) {
                if (lst.size() != 3) throw std::out_of_range("Expected 3 strings");
                for (int i = 0; i < 3; ++i) {
                    std::string v = lst[i].cast<std::string>();
                    std::strncpy(s._szParam[i], v.c_str(), MAX_STRING_SIZE - 1);
                    s._szParam[i][MAX_STRING_SIZE - 1] = '\0';
                }
            });
    m.attr("MONITORING_ALARM") = m.attr("LOG_ALARM");

    // ─────────────────────────────────────────────────────────────────────────
    // MESSAGE_PROGRESS
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MESSAGE_PROGRESS>(m, "MESSAGE_PROGRESS")
        .def(py::init<>())
        .def_readwrite("_iCurrentCount", &MESSAGE_PROGRESS::_iCurrentCount)
        .def_readwrite("_iTotalCount",   &MESSAGE_PROGRESS::_iTotalCount);

    // ─────────────────────────────────────────────────────────────────────────
    // MESSAGE_POPUP / MONITORING_POPUP
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MESSAGE_POPUP>(m, "MESSAGE_POPUP")
        .def(py::init<>())
        STR_PROP(MESSAGE_POPUP, _szText)
        .def_readwrite("_iLevel",   &MESSAGE_POPUP::_iLevel)
        .def_readwrite("_iBtnType", &MESSAGE_POPUP::_iBtnType);
    m.attr("MONITORING_POPUP") = m.attr("MESSAGE_POPUP");

    // ─────────────────────────────────────────────────────────────────────────
    // MESSAGE_INPUT / MONITORING_INPUT
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MESSAGE_INPUT>(m, "MESSAGE_INPUT")
        .def(py::init<>())
        STR_PROP(MESSAGE_INPUT, _szText)
        .def_readwrite("_iType", &MESSAGE_INPUT::_iType);
    m.attr("MONITORING_INPUT") = m.attr("MESSAGE_INPUT");

    // ─────────────────────────────────────────────────────────────────────────
    // CONTROL_BRAKE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONTROL_BRAKE>(m, "CONTROL_BRAKE")
        .def(py::init<>())
        .def_readwrite("_iTargetAxs", &CONTROL_BRAKE::_iTargetAxs)
        .def_readwrite("_bValue",     &CONTROL_BRAKE::_bValue);

    // ─────────────────────────────────────────────────────────────────────────
    // MOVE_POSB
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MOVE_POSB>(m, "MOVE_POSB")
        .def(py::init<>())
        .def_property("_fTargetPos",
            [](const MOVE_POSB &s) { return array2d_copy(s._fTargetPos); },
            [](MOVE_POSB &s, py::array_t<float> v) { array2d_set(s._fTargetPos, v); })
        .def_readwrite("_iBlendType", &MOVE_POSB::_iBlendType)
        .def_readwrite("_fBlendRad",  &MOVE_POSB::_fBlendRad);

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_POSE / ROBOT_VEL / ROBOT_FORCE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_POSE>(m, "ROBOT_POSE")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_POSE, _fPosition, float);

    py::class_<ROBOT_VEL>(m, "ROBOT_VEL")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_VEL, _fVelocity, float);

    py::class_<ROBOT_FORCE>(m, "ROBOT_FORCE")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_FORCE, _fForce, float);

    // ─────────────────────────────────────────────────────────────────────────
    // ROBOT_TASK_POSE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ROBOT_TASK_POSE>(m, "ROBOT_TASK_POSE")
        .def(py::init<>())
        ARRAY_PROP(ROBOT_TASK_POSE, _fTargetPos, float)
        .def_readwrite("_iTargetSol", &ROBOT_TASK_POSE::_iTargetSol);

    // ─────────────────────────────────────────────────────────────────────────
    // USER_COORDINATE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<USER_COORDINATE>(m, "USER_COORDINATE")
        .def(py::init<>())
        .def_readwrite("_iReqId",     &USER_COORDINATE::_iReqId)
        .def_readwrite("_iTargetRef", &USER_COORDINATE::_iTargetRef)
        ARRAY_PROP(USER_COORDINATE, _fTargetPos, float);

    // ─────────────────────────────────────────────────────────────────────────
    // MEASURE_TOOL_RESPONSE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MEASURE_TOOL_RESPONSE>(m, "MEASURE_TOOL_RESPONSE")
        .def(py::init<>())
        .def_readwrite("_fWeight", &MEASURE_TOOL_RESPONSE::_fWeight)
        ARRAY_PROP(MEASURE_TOOL_RESPONSE, _fXYZ, float);

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_TCP
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_TCP>(m, "CONFIG_TCP")
        .def(py::init<>())
        ARRAY_PROP(CONFIG_TCP, _fTargetPos, float);

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_TOOL
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_TOOL>(m, "CONFIG_TOOL")
        .def(py::init<>())
        .def_readwrite("_fWeight",  &CONFIG_TOOL::_fWeight)
        ARRAY_PROP(CONFIG_TOOL, _fXYZ, float)
        ARRAY_PROP(CONFIG_TOOL, _fInertia, float);

    // ─────────────────────────────────────────────────────────────────────────
    // MEASURE_TCP_RESPONSE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MEASURE_TCP_RESPONSE>(m, "MEASURE_TCP_RESPONSE")
        .def(py::init<>())
        .def_readwrite("_tTCP",   &MEASURE_TCP_RESPONSE::_tTCP)
        .def_readwrite("_fError", &MEASURE_TCP_RESPONSE::_fError);

    // ─────────────────────────────────────────────────────────────────────────
    // FLANGE_SERIAL_DATA (union exposed through named sub-struct properties)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<FLANGE_SERIAL_DATA>(m, "FLANGE_SERIAL_DATA")
        .def(py::init<>())
        .def_readwrite("_iCommand", &FLANGE_SERIAL_DATA::_iCommand)
        .def_property("_szBaudrate",
            [](const FLANGE_SERIAL_DATA &s) { return array_copy(s._tData._tConfig._szBaudrate); },
            [](FLANGE_SERIAL_DATA &s, py::array_t<unsigned char> v) { array_set(s._tData._tConfig._szBaudrate, v); })
        .def_property("_szDataLength",
            [](const FLANGE_SERIAL_DATA &s) { return s._tData._tConfig._szDataLength; },
            [](FLANGE_SERIAL_DATA &s, unsigned char v) { s._tData._tConfig._szDataLength = v; })
        .def_property("_szParity",
            [](const FLANGE_SERIAL_DATA &s) { return s._tData._tConfig._szParity; },
            [](FLANGE_SERIAL_DATA &s, unsigned char v) { s._tData._tConfig._szParity = v; })
        .def_property("_szStopBit",
            [](const FLANGE_SERIAL_DATA &s) { return s._tData._tConfig._szStopBit; },
            [](FLANGE_SERIAL_DATA &s, unsigned char v) { s._tData._tConfig._szStopBit = v; })
        .def_property("_iLength",
            [](const FLANGE_SERIAL_DATA &s) { return s._tData._tValue._iLength; },
            [](FLANGE_SERIAL_DATA &s, unsigned short v) { s._tData._tValue._iLength = v; })
        .def_property("_szValue",
            [](const FLANGE_SERIAL_DATA &s) { return std::string(reinterpret_cast<const char*>(s._tData._tValue._szValue)); },
            [](FLANGE_SERIAL_DATA &s, const std::string &v) {
                std::strncpy(reinterpret_cast<char*>(s._tData._tValue._szValue), v.c_str(), MAX_SYMBOL_SIZE - 1);
            });

    // ─────────────────────────────────────────────────────────────────────────
    // FLANGE_SER_RXD_INFO / _EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<FLANGE_SER_RXD_INFO>(m, "FLANGE_SER_RXD_INFO")
        .def(py::init<>())
        .def_readwrite("_iSize", &FLANGE_SER_RXD_INFO::_iSize)
        ARRAY_PROP(FLANGE_SER_RXD_INFO, _cRxd, unsigned char);

    py::class_<FLANGE_SER_RXD_INFO_EX>(m, "FLANGE_SER_RXD_INFO_EX")
        .def(py::init<>())
        .def_readwrite("_iSize",    &FLANGE_SER_RXD_INFO_EX::_iSize)
        ARRAY_PROP(FLANGE_SER_RXD_INFO_EX, _cRxd, unsigned char)
        .def_readwrite("_portNum",  &FLANGE_SER_RXD_INFO_EX::_portNum);
    // ─────────────────────────────────────────────────────────────────────────
    // READ_FLANGE_SERIAL / _EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<READ_FLANGE_SERIAL>(m, "READ_FLANGE_SERIAL")
        .def(py::init<>())
        .def_readwrite("_bRecvFlag", &READ_FLANGE_SERIAL::_bRecvFlag);

    py::class_<READ_FLANGE_SERIAL_EX>(m, "READ_FLANGE_SERIAL_EX")
        .def(py::init<>())
        ARRAY_PROP(READ_FLANGE_SERIAL_EX, _bRecvFlag, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // INVERSE_KINEMATIC_RESPONSE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<INVERSE_KINEMATIC_RESPONSE>(m, "INVERSE_KINEMATIC_RESPONSE")
        .def(py::init<>())
        ARRAY_PROP(INVERSE_KINEMATIC_RESPONSE, _fTargetPos, float)
        .def_readwrite("_iStatus", &INVERSE_KINEMATIC_RESPONSE::_iStatus);

    // ─────────────────────────────────────────────────────────────────────────
    // RT_INPUT_DATA_LIST
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<RT_INPUT_DATA_LIST>(m, "RT_INPUT_DATA_LIST")
        .def(py::init<>())
        ARRAY_PROP(RT_INPUT_DATA_LIST, _fExternalForceTorque, float)
        .def_readwrite("_iExternalDI",      &RT_INPUT_DATA_LIST::_iExternalDI)
        .def_readwrite("_iExternalDO",      &RT_INPUT_DATA_LIST::_iExternalDO)
        ARRAY_PROP(RT_INPUT_DATA_LIST, _fExternalAnalogInput, float)
        ARRAY_PROP(RT_INPUT_DATA_LIST, _fExternalAnalogOutput, float);

    // ─────────────────────────────────────────────────────────────────────────
    // RT_OUTPUT_DATA_LIST  (the big real-time data struct)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<RT_OUTPUT_DATA_LIST>(m, "RT_OUTPUT_DATA_LIST")
        .def(py::init<>())
        .def_readwrite("time_stamp",                     &RT_OUTPUT_DATA_LIST::time_stamp)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_position, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_position_abs, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_velocity, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_velocity_abs, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_tcp_position, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_tcp_velocity, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_flange_position, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_flange_velocity, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_motor_torque, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_torque, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, raw_joint_torque, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, raw_force_torque, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, external_joint_torque, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, external_tcp_force, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, target_joint_position, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, target_joint_velocity, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, target_joint_acceleration, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, target_motor_torque, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, target_tcp_position, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, target_tcp_velocity, float)
        .def_property("jacobian_matrix",
            [](const RT_OUTPUT_DATA_LIST &s) { return array2d_copy(s.jacobian_matrix); },
            [](RT_OUTPUT_DATA_LIST &s, py::array_t<float> v) { array2d_set(s.jacobian_matrix, v); })
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, gravity_torque, float)
        .def_property("coriolis_matrix",
            [](const RT_OUTPUT_DATA_LIST &s) { return array2d_copy(s.coriolis_matrix); },
            [](RT_OUTPUT_DATA_LIST &s, py::array_t<float> v) { array2d_set(s.coriolis_matrix, v); })
        .def_property("mass_matrix",
            [](const RT_OUTPUT_DATA_LIST &s) { return array2d_copy(s.mass_matrix); },
            [](RT_OUTPUT_DATA_LIST &s, py::array_t<float> v) { array2d_set(s.mass_matrix, v); })
        .def_readwrite("solution_space",       &RT_OUTPUT_DATA_LIST::solution_space)
        .def_readwrite("singularity",          &RT_OUTPUT_DATA_LIST::singularity)
        .def_readwrite("operation_speed_rate", &RT_OUTPUT_DATA_LIST::operation_speed_rate)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, joint_temperature, float)
        .def_readwrite("controller_digital_input",  &RT_OUTPUT_DATA_LIST::controller_digital_input)
        .def_readwrite("controller_digital_output", &RT_OUTPUT_DATA_LIST::controller_digital_output)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_input_type, unsigned char)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_input, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_output_type, unsigned char)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_output, float)
        .def_readwrite("flange_digital_input",  &RT_OUTPUT_DATA_LIST::flange_digital_input)
        .def_readwrite("flange_digital_output", &RT_OUTPUT_DATA_LIST::flange_digital_output)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, flange_analog_input, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, external_encoder_strobe_count, unsigned char)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, external_encoder_count, unsigned int)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, goal_joint_position, float)
        ARRAY_PROP(RT_OUTPUT_DATA_LIST, goal_tcp_position, float)
        .def_readwrite("robot_mode",   &RT_OUTPUT_DATA_LIST::robot_mode)
        .def_readwrite("robot_state",  &RT_OUTPUT_DATA_LIST::robot_state)
        .def_readwrite("control_mode", &RT_OUTPUT_DATA_LIST::control_mode);
    // ─────────────────────────────────────────────────────────────────────────
    // JOINT_RANGE / CONFIG_JOINT_RANGE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<JOINT_RANGE>(m, "JOINT_RANGE")
        .def(py::init<>())
        ARRAY_PROP(JOINT_RANGE, _fMaxVelocity, float)
        ARRAY_PROP(JOINT_RANGE, _fMaxRange, float)
        ARRAY_PROP(JOINT_RANGE, _fMinRange, float);

    py::class_<CONFIG_JOINT_RANGE>(m, "CONFIG_JOINT_RANGE")
        .def(py::init<>())
        .def_readwrite("_Normal",  &CONFIG_JOINT_RANGE::_Normal)
        .def_readwrite("_Reduced", &CONFIG_JOINT_RANGE::_Reduced);
    // ─────────────────────────────────────────────────────────────────────────
    // GENERAL_RANGE / CONFIG_GENERAL_RANGE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<GENERAL_RANGE>(m, "GENERAL_RANGE")
        .def(py::init<>())
        .def_readwrite("_fMaxForce",    &GENERAL_RANGE::_fMaxForce)
        .def_readwrite("_fMaxPower",    &GENERAL_RANGE::_fMaxPower)
        .def_readwrite("_fMaxSpeed",    &GENERAL_RANGE::_fMaxSpeed)
        .def_readwrite("_fMaxMomentum", &GENERAL_RANGE::_fMaxMomentum);

    py::class_<CONFIG_GENERAL_RANGE>(m, "CONFIG_GENERAL_RANGE")
        .def(py::init<>())
        .def_readwrite("_Normal",  &CONFIG_GENERAL_RANGE::_Normal)
        .def_readwrite("_Reduced", &CONFIG_GENERAL_RANGE::_Reduced);

    // ─────────────────────────────────────────────────────────────────────────
    // POINT_2D / POINT_3D
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<POINT_2D>(m, "POINT_2D")
        .def(py::init<>())
        .def_readwrite("_fXPos", &POINT_2D::_fXPos)
        .def_readwrite("_fYPos", &POINT_2D::_fYPos);

    py::class_<POINT_3D>(m, "POINT_3D")
        .def(py::init<>())
        .def_readwrite("_fXPos", &POINT_3D::_fXPos)
        .def_readwrite("_fYPos", &POINT_3D::_fYPos)
        .def_readwrite("_fZPos", &POINT_3D::_fZPos);

    // ─────────────────────────────────────────────────────────────────────────
    // LINE (= LINE_2D alias)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<LINE>(m, "LINE")
        .def(py::init<>())
        .def_readwrite("_tFromPoint", &LINE::_tFromPoint)
        .def_readwrite("_tToPoint",   &LINE::_tToPoint);
    m.attr("LINE_2D") = m.attr("LINE");

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_SAFETY_FUNCTION (union – expose raw byte array + helper)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_SAFETY_FUNCTION>(m, "CONFIG_SAFETY_FUNCTION")
        .def(py::init<>())
        .def_property("_iStopCode",
            [](const CONFIG_SAFETY_FUNCTION &s) { return array_copy(s._iStopCode); },
            [](CONFIG_SAFETY_FUNCTION &s, py::array_t<unsigned char> v) { array_set(s._iStopCode, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_INSTALL_POSE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_INSTALL_POSE>(m, "CONFIG_INSTALL_POSE")
        .def(py::init<>())
        .def_readwrite("_fGradient", &CONFIG_INSTALL_POSE::_fGradient)
        .def_readwrite("_fRotation", &CONFIG_INSTALL_POSE::_fRotation);

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_SAFETY_IO / _EX / _OP
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_SAFETY_IO>(m, "CONFIG_SAFETY_IO")
        .def(py::init<>())
        .def_property("_iIO",
            [](const CONFIG_SAFETY_IO &s) { return array2d_copy(s._iIO); },
            [](CONFIG_SAFETY_IO &s, py::array_t<unsigned char> v) { array2d_set(s._iIO, v); });

    py::class_<CONFIG_SAFETY_IO_EX>(m, "CONFIG_SAFETY_IO_EX")
        .def(py::init<>())
        .def_property("_iIO",
            [](const CONFIG_SAFETY_IO_EX &s) { return array2d_copy(s._iIO); },
            [](CONFIG_SAFETY_IO_EX &s, py::array_t<unsigned char> v) { array2d_set(s._iIO, v); });

    py::class_<CONFIG_SAFETY_IO_OP>(m, "CONFIG_SAFETY_IO_OP")
        .def(py::init<>())
        .def_property("_iIO",
            [](const CONFIG_SAFETY_IO_OP &s) { return array2d_copy(s._iIO); },
            [](CONFIG_SAFETY_IO_OP &s, py::array_t<unsigned char> v) { array2d_set(s._iIO, v); })
        .def_readwrite("_iTBI_Op",    &CONFIG_SAFETY_IO_OP::_iTBI_Op)
        .def_readwrite("_iReserved",  &CONFIG_SAFETY_IO_OP::_iReserved)
        .def_property("_iIO_Op",
            [](const CONFIG_SAFETY_IO_OP &s) { return array2d_copy(s._iIO_Op); },
            [](CONFIG_SAFETY_IO_OP &s, py::array_t<unsigned char> v) { array2d_set(s._iIO_Op, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_VIRTUAL_FENCE  (union buffer exposed as bytes)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_VIRTUAL_FENCE>(m, "CONFIG_VIRTUAL_FENCE")
        .def(py::init<>())
        .def_readwrite("_iTargetRef", &CONFIG_VIRTUAL_FENCE::_iTargetRef)
        .def_readwrite("_iFenceType", &CONFIG_VIRTUAL_FENCE::_iFenceType)
        .def_property("_iBuffer",
            [](const CONFIG_VIRTUAL_FENCE &s) { return array_copy(s._tFenceObject._iBuffer); },
            [](CONFIG_VIRTUAL_FENCE &s, py::array_t<unsigned char> v) { array_set(s._tFenceObject._iBuffer, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_SAFE_ZONE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_SAFE_ZONE>(m, "CONFIG_SAFE_ZONE")
        .def(py::init<>())
        .def_readwrite("_iTargetRef", &CONFIG_SAFE_ZONE::_iTargetRef)
        .def_property("_tLine",
            [](const CONFIG_SAFE_ZONE &s) {
                py::list lst;
                for (int i = 0; i < 2; ++i) lst.append(s._tLine[i]);
                return lst;
            },
            [](CONFIG_SAFE_ZONE &s, py::list v) {
                if (v.size() != 2) throw std::out_of_range("Expected 2 LINE elements");
                for (int i = 0; i < 2; ++i) s._tLine[i] = v[i].cast<LINE>();
            })
        .def_property("_tPoint",
            [](const CONFIG_SAFE_ZONE &s) {
                py::list lst;
                for (int i = 0; i < 3; ++i) lst.append(s._tPoint[i]);
                return lst;
            },
            [](CONFIG_SAFE_ZONE &s, py::list v) {
                if (v.size() != 3) throw std::out_of_range("Expected 3 POINT_2D elements");
                for (int i = 0; i < 3; ++i) s._tPoint[i] = v[i].cast<POINT_2D>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // ENABLE_SAFE_ZONE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<ENABLE_SAFE_ZONE>(m, "ENABLE_SAFE_ZONE")
        .def(py::init<>())
        ARRAY_PROP(ENABLE_SAFE_ZONE, _iRegion, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // SAFETY_OBJECT_SPHERE / CAPSULE / CUBE / OBB / POLYPRISM
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<SAFETY_OBJECT_SPHERE>(m, "SAFETY_OBJECT_SPHERE")
        .def(py::init<>())
        .def_readwrite("_fRadius",    &SAFETY_OBJECT_SPHERE::_fRadius)
        .def_readwrite("_tTargetPos", &SAFETY_OBJECT_SPHERE::_tTargetPos);

    py::class_<SAFETY_OBJECT_CAPSULE>(m, "SAFETY_OBJECT_CAPSULE")
        .def(py::init<>())
        .def_readwrite("_fRadius", &SAFETY_OBJECT_CAPSULE::_fRadius)
        .def_property("_tTargetPos",
            [](const SAFETY_OBJECT_CAPSULE &s) {
                py::list lst;
                for (int i = 0; i < 2; ++i) lst.append(s._tTargetPos[i]);
                return lst;
            },
            [](SAFETY_OBJECT_CAPSULE &s, py::list v) {
                if (v.size() != 2) throw std::out_of_range("Expected 2 POINT_3D elements");
                for (int i = 0; i < 2; ++i) s._tTargetPos[i] = v[i].cast<POINT_3D>();
            });

    py::class_<SAFETY_OBJECT_CUBE>(m, "SAFETY_OBJECT_CUBE")
        .def(py::init<>())
        .def_property("_tTargetPos",
            [](const SAFETY_OBJECT_CUBE &s) {
                py::list lst;
                for (int i = 0; i < 2; ++i) lst.append(s._tTargetPos[i]);
                return lst;
            },
            [](SAFETY_OBJECT_CUBE &s, py::list v) {
                if (v.size() != 2) throw std::out_of_range("Expected 2 POINT_3D elements");
                for (int i = 0; i < 2; ++i) s._tTargetPos[i] = v[i].cast<POINT_3D>();
            });

    py::class_<SAFETY_OBJECT_OBB>(m, "SAFETY_OBJECT_OBB")
        .def(py::init<>())
        .def_property("_tTargetPos",
            [](const SAFETY_OBJECT_OBB &s) {
                py::list lst;
                for (int i = 0; i < 4; ++i) lst.append(s._tTargetPos[i]);
                return lst;
            },
            [](SAFETY_OBJECT_OBB &s, py::list v) {
                if (v.size() != 4) throw std::out_of_range("Expected 4 POINT_3D elements");
                for (int i = 0; i < 4; ++i) s._tTargetPos[i] = v[i].cast<POINT_3D>();
            });

    py::class_<SAFETY_OBJECT_POLYPRISM>(m, "SAFETY_OBJECT_POLYPRISM")
        .def(py::init<>())
        .def_readwrite("_iPointCount", &SAFETY_OBJECT_POLYPRISM::_iPointCount)
        .def_property("_tPoint",
            [](const SAFETY_OBJECT_POLYPRISM &s) {
                py::list lst;
                for (int i = 0; i < 10; ++i) lst.append(s._tPoint[i]);
                return lst;
            },
            [](SAFETY_OBJECT_POLYPRISM &s, py::list v) {
                if (v.size() != 10) throw std::out_of_range("Expected 10 POINT_2D elements");
                for (int i = 0; i < 10; ++i) s._tPoint[i] = v[i].cast<POINT_2D>();
            })
        .def_readwrite("_fZLoLimit", &SAFETY_OBJECT_POLYPRISM::_fZLoLimit)
        .def_readwrite("_fZUpLimit", &SAFETY_OBJECT_POLYPRISM::_fZUpLimit);

    // SAFETY_OBJECT_DATA union – expose raw buffer
    py::class_<SAFETY_OBJECT_DATA>(m, "SAFETY_OBJECT_DATA")
        .def(py::init<>())
        .def_property("_iBuffer",
            [](const SAFETY_OBJECT_DATA &s) { return array_copy(s._iBuffer); },
            [](SAFETY_OBJECT_DATA &s, py::array_t<unsigned char> v) { array_set(s._iBuffer, v); });

    // SAFETY_OBJECT
    py::class_<SAFETY_OBJECT>(m, "SAFETY_OBJECT")
        .def(py::init<>())
        .def_readwrite("_iTargetRef",  &SAFETY_OBJECT::_iTargetRef)
        .def_readwrite("_iObjectType", &SAFETY_OBJECT::_iObjectType)
        .def_readwrite("_tObject",     &SAFETY_OBJECT::_tObject);

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_PROTECTED_ZONE / CONFIG_COLLISION_MUTE_ZONE_PROPERTY / _ZONE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_PROTECTED_ZONE>(m, "CONFIG_PROTECTED_ZONE")
        .def(py::init<>())
        ARRAY_PROP(CONFIG_PROTECTED_ZONE, _iValidity, unsigned char)
        .def_property("_tZone",
            [](const CONFIG_PROTECTED_ZONE &s) {
                py::list lst;
                for (int i = 0; i < 10; ++i) lst.append(s._tZone[i]);
                return lst;
            },
            [](CONFIG_PROTECTED_ZONE &s, py::list v) {
                if (v.size() != 10) throw std::out_of_range("Expected 10 SAFETY_OBJECT elements");
                for (int i = 0; i < 10; ++i) s._tZone[i] = v[i].cast<SAFETY_OBJECT>();
            });

    py::class_<CONFIG_COLLISION_MUTE_ZONE_PROPERTY>(m, "CONFIG_COLLISION_MUTE_ZONE_PROPERTY")
        .def(py::init<>())
        STR_PROP(CONFIG_COLLISION_MUTE_ZONE_PROPERTY, _szIdentifier)
        .def_readwrite("_iOnOff",       &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_iOnOff)
        .def_readwrite("_iSafetyIO",    &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_iSafetyIO)
        .def_readwrite("_fSensitivity", &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_fSensitivity)
        .def_readwrite("_tZone",        &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_tZone);

    py::class_<CONFIG_COLLISION_MUTE_ZONE>(m, "CONFIG_COLLISION_MUTE_ZONE")
        .def(py::init<>())
        ARRAY_PROP(CONFIG_COLLISION_MUTE_ZONE, _iValidity, unsigned char)
        .def_property("_tProperty",
            [](const CONFIG_COLLISION_MUTE_ZONE &s) {
                py::list lst;
                for (int i = 0; i < 10; ++i) lst.append(s._tProperty[i]);
                return lst;
            },
            [](CONFIG_COLLISION_MUTE_ZONE &s, py::list v) {
                if (v.size() != 10) throw std::out_of_range("Expected 10 elements");
                for (int i = 0; i < 10; ++i) s._tProperty[i] = v[i].cast<CONFIG_COLLISION_MUTE_ZONE_PROPERTY>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // SAFETY_TOOL_ORIENTATION_LIMIT / CONFIG_TOOL_ORIENTATION_LIMIT_ZONE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<SAFETY_TOOL_ORIENTATION_LIMIT>(m, "SAFETY_TOOL_ORIENTATION_LIMIT")
        .def(py::init<>())
        .def_readwrite("_tTargetDir", &SAFETY_TOOL_ORIENTATION_LIMIT::_tTargetDir)
        .def_readwrite("_fTargetAng", &SAFETY_TOOL_ORIENTATION_LIMIT::_fTargetAng);

    py::class_<CONFIG_TOOL_ORIENTATION_LIMIT_ZONE>(m, "CONFIG_TOOL_ORIENTATION_LIMIT_ZONE")
        .def(py::init<>())
        ARRAY_PROP(CONFIG_TOOL_ORIENTATION_LIMIT_ZONE, _iValidity, unsigned char)
        .def_property("_tZone",
            [](const CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s) {
                py::list lst;
                for (int i = 0; i < 10; ++i) lst.append(s._tZone[i]);
                return lst;
            },
            [](CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s, py::list v) {
                if (v.size() != 10) throw std::out_of_range("Expected 10 SAFETY_OBJECT elements");
                for (int i = 0; i < 10; ++i) s._tZone[i] = v[i].cast<SAFETY_OBJECT>();
            })
        .def_property("_tLimit",
            [](const CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s) {
                py::list lst;
                for (int i = 0; i < 10; ++i) lst.append(s._tLimit[i]);
                return lst;
            },
            [](CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s, py::list v) {
                if (v.size() != 10) throw std::out_of_range("Expected 10 limit elements");
                for (int i = 0; i < 10; ++i) s._tLimit[i] = v[i].cast<SAFETY_TOOL_ORIENTATION_LIMIT>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_NUDGE / CONFIG_COCKPIT_EX / CONFIG_IDLE_OFF
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_NUDGE>(m, "CONFIG_NUDGE")
        .def(py::init<>())
        .def_readwrite("_bEnable",     &CONFIG_NUDGE::_bEnable)
        .def_readwrite("_fInputForce", &CONFIG_NUDGE::_fInputForce)
        .def_readwrite("_fDelayTime",  &CONFIG_NUDGE::_fDelayTime);

    py::class_<CONFIG_COCKPIT_EX>(m, "CONFIG_COCKPIT_EX")
        .def(py::init<>())
        .def_readwrite("_bEnable",        &CONFIG_COCKPIT_EX::_bEnable)
        ARRAY_PROP(CONFIG_COCKPIT_EX, _iButton, unsigned char)
        .def_readwrite("_bRecoveryTeach", &CONFIG_COCKPIT_EX::_bRecoveryTeach);

    py::class_<CONFIG_IDLE_OFF>(m, "CONFIG_IDLE_OFF")
        .def(py::init<>())
        .def_readwrite("_bFuncEnable",  &CONFIG_IDLE_OFF::_bFuncEnable)
        .def_readwrite("_fElapseTime",  &CONFIG_IDLE_OFF::_fElapseTime);

    // ─────────────────────────────────────────────────────────────────────────
    // WRITE_MODBUS_DATA / WRITE_MODBUS_RTU_DATA / MODBUS_DATA / MODBUS_DATA_LIST
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<WRITE_MODBUS_DATA>(m, "WRITE_MODBUS_DATA")
        .def(py::init<>())
        STR_PROP(WRITE_MODBUS_DATA, _szSymbol)
        STR_PROP(WRITE_MODBUS_DATA, _szIpAddr)
        .def_readwrite("_iPort",     &WRITE_MODBUS_DATA::_iPort)
        .def_readwrite("_iSlaveID",  &WRITE_MODBUS_DATA::_iSlaveID)
        .def_readwrite("_iRegType",  &WRITE_MODBUS_DATA::_iRegType)
        .def_readwrite("_iRegIndex", &WRITE_MODBUS_DATA::_iRegIndex)
        .def_readwrite("_iRegValue", &WRITE_MODBUS_DATA::_iRegValue);
    m.attr("WRITE_MODBUS_TCP_DATA") = m.attr("WRITE_MODBUS_DATA");

    py::class_<WRITE_MODBUS_RTU_DATA>(m, "WRITE_MODBUS_RTU_DATA")
        .def(py::init<>())
        STR_PROP(WRITE_MODBUS_RTU_DATA, _szSymbol)
        STR_PROP(WRITE_MODBUS_RTU_DATA, _szttyPort)
        .def_readwrite("_iSlaveID",  &WRITE_MODBUS_RTU_DATA::_iSlaveID)
        .def_readwrite("_iBaudRate", &WRITE_MODBUS_RTU_DATA::_iBaudRate)
        .def_readwrite("_iByteSize", &WRITE_MODBUS_RTU_DATA::_iByteSize)
        .def_readwrite("_szParity",  &WRITE_MODBUS_RTU_DATA::_szParity)
        .def_readwrite("_iStopBit",  &WRITE_MODBUS_RTU_DATA::_iStopBit)
        .def_readwrite("_iRegType",  &WRITE_MODBUS_RTU_DATA::_iRegType)
        .def_readwrite("_iRegIndex", &WRITE_MODBUS_RTU_DATA::_iRegIndex)
        .def_readwrite("_iRegValue", &WRITE_MODBUS_RTU_DATA::_iRegValue);

    py::class_<MODBUS_DATA>(m, "MODBUS_DATA")
        .def(py::init<>())
        .def_readwrite("_iType", &MODBUS_DATA::_iType)
        .def_property("_tcp",
            [](const MODBUS_DATA &s) -> const WRITE_MODBUS_TCP_DATA & { return s._tData._tcp; },
            [](MODBUS_DATA &s, const WRITE_MODBUS_TCP_DATA &v) { s._tData._tcp = v; })
        .def_property("_rtu",
            [](const MODBUS_DATA &s) -> const WRITE_MODBUS_RTU_DATA & { return s._tData._rtu; },
            [](MODBUS_DATA &s, const WRITE_MODBUS_RTU_DATA &v) { s._tData._rtu = v; });

    py::class_<MODBUS_DATA_LIST>(m, "MODBUS_DATA_LIST")
        .def(py::init<>())
        .def_readwrite("_nCount", &MODBUS_DATA_LIST::_nCount)
        .def_property("_tRegister",
            [](const MODBUS_DATA_LIST &s) {
                py::list lst;
                for (int i = 0; i < MAX_MODBUS_TOTAL_REGISTERS; ++i)
                    lst.append(s._tRegister[i]);
                return lst;
            },
            [](MODBUS_DATA_LIST &s, py::list v) {
                if ((int)v.size() != MAX_MODBUS_TOTAL_REGISTERS)
                    throw std::out_of_range("Expected " + std::to_string(MAX_MODBUS_TOTAL_REGISTERS) + " elements");
                for (int i = 0; i < MAX_MODBUS_TOTAL_REGISTERS; ++i)
                    s._tRegister[i] = v[i].cast<MODBUS_DATA>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_WORLD_COORDINATE / CONFIG_CONFIGURABLE_IO / _EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_WORLD_COORDINATE>(m, "CONFIG_WORLD_COORDINATE")
        .def(py::init<>())
        .def_readwrite("_iType", &CONFIG_WORLD_COORDINATE::_iType)
        ARRAY_PROP(CONFIG_WORLD_COORDINATE, _fPosition, float);

    py::class_<CONFIG_CONFIGURABLE_IO>(m, "CONFIG_CONFIGURABLE_IO")
        .def(py::init<>())
        .def_property("_iIO",
            [](const CONFIG_CONFIGURABLE_IO &s) { return array2d_copy(s._iIO); },
            [](CONFIG_CONFIGURABLE_IO &s, py::array_t<unsigned char> v) { array2d_set(s._iIO, v); });

    py::class_<CONFIG_CONFIGURABLE_IO_EX>(m, "CONFIG_CONFIGURABLE_IO_EX")
        .def(py::init<>())
        .def_property("_iIO",
            [](const CONFIG_CONFIGURABLE_IO_EX &s) { return array2d_copy(s._iIO); },
            [](CONFIG_CONFIGURABLE_IO_EX &s, py::array_t<unsigned char> v) { array2d_set(s._iIO, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_TOOL_SHAPE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_TOOL_SHAPE>(m, "CONFIG_TOOL_SHAPE")
        .def(py::init<>())
        ARRAY_PROP(CONFIG_TOOL_SHAPE, _iValidity, unsigned char)
        .def_property("_tShape",
            [](const CONFIG_TOOL_SHAPE &s) {
                py::list lst;
                for (int i = 0; i < 5; ++i) lst.append(s._tShape[i]);
                return lst;
            },
            [](CONFIG_TOOL_SHAPE &s, py::list v) {
                if (v.size() != 5) throw std::out_of_range("Expected 5 SAFETY_OBJECT elements");
                for (int i = 0; i < 5; ++i) s._tShape[i] = v[i].cast<SAFETY_OBJECT>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_TOOL_SYMBOL / CONFIG_TCP_SYMBOL / CONFIG_TOOL_LIST / etc.
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_TOOL_SYMBOL>(m, "CONFIG_TOOL_SYMBOL")
        .def(py::init<>())
        STR_PROP(CONFIG_TOOL_SYMBOL, _szSymbol)
        .def_readwrite("_tTool", &CONFIG_TOOL_SYMBOL::_tTool);

    py::class_<CONFIG_TCP_SYMBOL>(m, "CONFIG_TCP_SYMBOL")
        .def(py::init<>())
        STR_PROP(CONFIG_TCP_SYMBOL, _szSymbol)
        .def_readwrite("_tTCP", &CONFIG_TCP_SYMBOL::_tTCP);

    py::class_<CONFIG_TOOL_SHAPE_SYMBOL>(m, "CONFIG_TOOL_SHAPE_SYMBOL")
        .def(py::init<>())
        STR_PROP(CONFIG_TOOL_SHAPE_SYMBOL, _szSymbol)
        .def_readwrite("_tToolShape", &CONFIG_TOOL_SHAPE_SYMBOL::_tToolShape);
    // Helper lambda for fixed-size arrays of compound structs (list-of-structs)
    auto make_struct_list_prop = [](auto &cls, const char *name, auto getter, auto setter) {
        cls.def_property(name, getter, setter);
    };
    (void)make_struct_list_prop;

    py::class_<CONFIG_TOOL_LIST>(m, "CONFIG_TOOL_LIST")
        .def(py::init<>())
        .def_readwrite("_iToolCount", &CONFIG_TOOL_LIST::_iToolCount)
        .def_property("_tTooList",
            [](const CONFIG_TOOL_LIST &s) {
                py::list lst;
                for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i) lst.append(s._tTooList[i]);
                return lst;
            },
            [](CONFIG_TOOL_LIST &s, py::list v) {
                if ((int)v.size() != MAX_CONFIG_TOOL_SIZE) throw std::out_of_range("Size mismatch");
                for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i) s._tTooList[i] = v[i].cast<CONFIG_TOOL_SYMBOL>();
            });

    py::class_<CONFIG_TCP_LIST>(m, "CONFIG_TCP_LIST")
        .def(py::init<>())
        .def_readwrite("_iToolCount", &CONFIG_TCP_LIST::_iToolCount)
        .def_property("_tTooList",
            [](const CONFIG_TCP_LIST &s) {
                py::list lst;
                for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i) lst.append(s._tTooList[i]);
                return lst;
            },
            [](CONFIG_TCP_LIST &s, py::list v) {
                if ((int)v.size() != MAX_CONFIG_TCP_SIZE) throw std::out_of_range("Size mismatch");
                for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i) s._tTooList[i] = v[i].cast<CONFIG_TCP_SYMBOL>();
            });

    py::class_<CONFIG_TOOL_SHAPE_LIST>(m, "CONFIG_TOOL_SHAPE_LIST")
        .def(py::init<>())
        .def_readwrite("_iToolCount", &CONFIG_TOOL_SHAPE_LIST::_iToolCount)
        .def_property("_tTooList",
            [](const CONFIG_TOOL_SHAPE_LIST &s) {
                py::list lst;
                for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i) lst.append(s._tTooList[i]);
                return lst;
            },
            [](CONFIG_TOOL_SHAPE_LIST &s, py::list v) {
                if ((int)v.size() != MAX_CONFIG_TOOL_SIZE) throw std::out_of_range("Size mismatch");
                for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i) s._tTooList[i] = v[i].cast<CONFIG_TOOL_SHAPE_SYMBOL>();
            });

    // ─────────────────────────────────────────────────────────────────────────
    // CONFIG_USER_COORDINATE_EX / CONFIG_PAYLOAD_EX
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<CONFIG_USER_COORDINATE_EX>(m, "CONFIG_USER_COORDINATE_EX")
        .def(py::init<>())
        .def_readwrite("_iTargetRef", &CONFIG_USER_COORDINATE_EX::_iTargetRef)
        ARRAY_PROP(CONFIG_USER_COORDINATE_EX, _fTargetPos, float)
        .def_readwrite("_iUserID",    &CONFIG_USER_COORDINATE_EX::_iUserID);

    py::class_<CONFIG_PAYLOAD_EX>(m, "CONFIG_PAYLOAD_EX")
        .def(py::init<>())
        .def_readwrite("_fWeight",         &CONFIG_PAYLOAD_EX::_fWeight)
        ARRAY_PROP(CONFIG_PAYLOAD_EX, _fXYZ, float)
        .def_readwrite("_iCogReference",   &CONFIG_PAYLOAD_EX::_iCogReference)
        .def_readwrite("_iAddUp",          &CONFIG_PAYLOAD_EX::_iAddUp)
        .def_readwrite("_fStartTime",      &CONFIG_PAYLOAD_EX::_fStartTime)
        .def_readwrite("_fTransitionTime", &CONFIG_PAYLOAD_EX::_fTransitionTime);

    // ─────────────────────────────────────────────────────────────────────────
    // SYSTEM_TIME / SYSTEM_IPADDRESS / SYSTEM_POWER / SYSTEM_CPUUSAGE / SYSTEM_DISKSIZE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<SYSTEM_TIME>(m, "SYSTEM_TIME")
        .def(py::init<>())
        STR_PROP(SYSTEM_TIME, _szDate)
        STR_PROP(SYSTEM_TIME, _szTime);

    py::class_<SYSTEM_IPADDRESS>(m, "SYSTEM_IPADDRESS")
        .def(py::init<>())
        .def_readwrite("_iUsage",   &SYSTEM_IPADDRESS::_iUsage)
        .def_readwrite("_iIpType",  &SYSTEM_IPADDRESS::_iIpType)
        STR_PROP(SYSTEM_IPADDRESS, _szHotsIp)
        STR_PROP(SYSTEM_IPADDRESS, _szSubnet)
        STR_PROP(SYSTEM_IPADDRESS, _szGateway)
        .def_property("_szDNS",
            [](const SYSTEM_IPADDRESS &s) {
                py::list lst;
                for (int i = 0; i < 2; ++i) lst.append(std::string(s._szDNS[i]));
                return lst;
            },
            [](SYSTEM_IPADDRESS &s, py::list v) {
                if (v.size() != 2) throw std::out_of_range("Expected 2 DNS strings");
                for (int i = 0; i < 2; ++i) {
                    std::string sv = v[i].cast<std::string>();
                    std::strncpy(s._szDNS[i], sv.c_str(), 15); s._szDNS[i][15] = '\0';
                }
            });

    py::class_<SYSTEM_POWER>(m, "SYSTEM_POWER")
        .def(py::init<>())
        .def_readwrite("_iTarget", &SYSTEM_POWER::_iTarget)
        .def_readwrite("_iPower",  &SYSTEM_POWER::_iPower);

    py::class_<SYSTEM_CPUUSAGE>(m, "SYSTEM_CPUUSAGE")
        .def(py::init<>())
        .def_readwrite("_iTotalUsage",   &SYSTEM_CPUUSAGE::_iTotalUsage)
        .def_readwrite("_iProcessUsage", &SYSTEM_CPUUSAGE::_iProcessUsage);

    py::class_<SYSTEM_DISKSIZE>(m, "SYSTEM_DISKSIZE")
        .def(py::init<>())
        .def_readwrite("_iTotalDiskSize", &SYSTEM_DISKSIZE::_iTotalDiskSize)
        .def_readwrite("_iUsedDiskSize",  &SYSTEM_DISKSIZE::_iUsedDiskSize);

    // ─────────────────────────────────────────────────────────────────────────
    // JTS_PARAM_DATA / FTS_PARAM_DATA  (aliased to CALIBRATE_*)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<JTS_PARAM_DATA>(m, "JTS_PARAM_DATA")
        .def(py::init<>())
        ARRAY_PROP(JTS_PARAM_DATA, _fOffset, float)
        ARRAY_PROP(JTS_PARAM_DATA, _fScale, float);
    m.attr("CALIBRATE_JTS_RESPONSE") = m.attr("JTS_PARAM_DATA");
    py::class_<FTS_PARAM_DATA>(m, "FTS_PARAM_DATA")
        .def(py::init<>())
        ARRAY_PROP(FTS_PARAM_DATA, _fOffset, float);
    m.attr("CALIBRATE_FTS_RESPONSE") = m.attr("FTS_PARAM_DATA");

    // ─────────────────────────────────────────────────────────────────────────
    // INSTALL_SUB_SYSTEM
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<INSTALL_SUB_SYSTEM>(m, "INSTALL_SUB_SYSTEM")
        .def(py::init<>())
        .def_readwrite("_bProcessButton", &INSTALL_SUB_SYSTEM::_bProcessButton)
        .def_readwrite("_bFTS",           &INSTALL_SUB_SYSTEM::_bFTS)
        .def_readwrite("_bCockpit",       &INSTALL_SUB_SYSTEM::_bCockpit);

    // ─────────────────────────────────────────────────────────────────────────
    // SYSTEM_UPDATE_RESPONSE
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<SYSTEM_UPDATE_RESPONSE>(m, "SYSTEM_UPDATE_RESPONSE")
        .def(py::init<>())
        .def_readwrite("_iProcess",  &SYSTEM_UPDATE_RESPONSE::_iProcess)
        ARRAY_PROP(SYSTEM_UPDATE_RESPONSE, _iInverter, unsigned char);

    // ─────────────────────────────────────────────────────────────────────────
    // KT_5G_CONFIG_PARAM
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<KT_5G_CONFIG_PARAM>(m, "KT_5G_CONFIG_PARAM")
        .def(py::init<>())
        .def_readwrite("_bEnable",   &KT_5G_CONFIG_PARAM::_bEnable)
        STR_PROP(KT_5G_CONFIG_PARAM, _szIpAddress)
        .def_readwrite("_nPort",     &KT_5G_CONFIG_PARAM::_nPort)
        STR_PROP(KT_5G_CONFIG_PARAM, _szDeviceId)
        STR_PROP(KT_5G_CONFIG_PARAM, _szDevicePw)
        STR_PROP(KT_5G_CONFIG_PARAM, _szGatewayId)
        .def_readwrite("_fPeriod",   &KT_5G_CONFIG_PARAM::_fPeriod);
    // ─────────────────────────────────────────────────────────────────────────
    // VECTOR3D / POSITION / NORMAL_VECTOR
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<VECTOR3D>(m, "VECTOR3D")
        .def(py::init<>())
        ARRAY_PROP(VECTOR3D, _fTargetPos, float);

    py::class_<POSITION>(m, "POSITION")
        .def(py::init<>())
        ARRAY_PROP(POSITION, _fTargetPos, float);
    m.attr("VECTOR6D") = m.attr("POSITION");
    m.attr("NORMAL_VECTOR_RESPONSE") = m.attr("VECTOR3D");
    m.attr("POSITION_ADDTO_RESPONSE") = m.attr("POSITION");

    py::class_<NORMAL_VECTOR>(m, "NORMAL_VECTOR")
        .def(py::init<>())
        .def_property("_fTargetPos",
            [](const NORMAL_VECTOR &s) { return array2d_copy(s._fTargetPos); },
            [](NORMAL_VECTOR &s, py::array_t<float> v) { array2d_set(s._fTargetPos, v); });

    // ─────────────────────────────────────────────────────────────────────────
    // GPIO_PORT / WRITE_SERIAL_BURST (GPIO_SETOUTPUT_BURST)
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<GPIO_PORT>(m, "GPIO_PORT")
        .def(py::init<>())
        .def_readwrite("_iIndex", &GPIO_PORT::_iIndex)
        .def_readwrite("_fValue", &GPIO_PORT::_fValue);

    py::class_<WRITE_SERIAL_BURST>(m, "WRITE_SERIAL_BURST")
        .def(py::init<>())
        .def_readwrite("_iLocation", &WRITE_SERIAL_BURST::_iLocation)
        .def_readwrite("_iCount",    &WRITE_SERIAL_BURST::_iCount)
        .def_property("_tPort",
            [](const WRITE_SERIAL_BURST &s) {
                py::list lst;
                for (int i = 0; i < MAX_DIGITAL_BURST_SIZE; ++i) lst.append(s._tPort[i]);
                return lst;
            },
            [](WRITE_SERIAL_BURST &s, py::list v) {
                if ((int)v.size() != MAX_DIGITAL_BURST_SIZE) throw std::out_of_range("Size mismatch");
                for (int i = 0; i < MAX_DIGITAL_BURST_SIZE; ++i) s._tPort[i] = v[i].cast<GPIO_PORT>();
            });
    m.attr("GPIO_SETOUTPUT_BURST") = m.attr("WRITE_SERIAL_BURST");

    // ─────────────────────────────────────────────────────────────────────────
    // MODBUS_REGISTER_MONITORING / WRITE_MODBUS_BURST / MODBUS_MULTI_REGISTER
    // ─────────────────────────────────────────────────────────────────────────
    py::class_<MODBUS_REGISTER_MONITORING>(m, "MODBUS_REGISTER_MONITORING")
        .def(py::init<>())
        STR_PROP(MODBUS_REGISTER_MONITORING, _szSymbol)
        .def_readwrite("_iRegValue", &MODBUS_REGISTER_MONITORING::_iRegValue);

    py::class_<WRITE_MODBUS_BURST>(m, "WRITE_MODBUS_BURST")
        .def(py::init<>())
        .def_readwrite("_iCount", &WRITE_MODBUS_BURST::_iCount)
        .def_property("_tRegister",
            [](const WRITE_MODBUS_BURST &s) {
                py::list lst;
                for (int i = 0; i < MAX_MODBUS_BURST_SIZE; ++i) lst.append(s._tRegister[i]);
                return lst;
            },
            [](WRITE_MODBUS_BURST &s, py::list v) {
                if ((int)v.size() != MAX_MODBUS_BURST_SIZE) throw std::out_of_range("Size mismatch");
                for (int i = 0; i < MAX_MODBUS_BURST_SIZE; ++i) s._tRegister[i] = v[i].cast<MODBUS_REGISTER_MONITORING>();
            });
    m.attr("MODBUS_REGISTER_BURST") = m.attr("WRITE_MODBUS_BURST");

    py::class_<MODBUS_MULTI_REGISTER>(m, "MODBUS_MULTI_REGISTER")
        .def(py::init<>())
        STR_PROP(MODBUS_MULTI_REGISTER, _szSymbol)
        .def_readwrite("_iRegCount", &MODBUS_MULTI_REGISTER::_iRegCount)
        ARRAY_PROP(MODBUS_MULTI_REGISTER, _iRegValue, unsigned short)
        .def_readwrite("_iRegIndex", &MODBUS_MULTI_REGISTER::_iRegIndex)
        .def_readwrite("_iSlaveID",  &MODBUS_MULTI_REGISTER::_iSlaveID);

    py::class_<UPDATE_MODBUS_MULTI_REGISTER>(m, "UPDATE_MODBUS_MULTI_REGISTER")
        .def(py::init<>())
        STR_PROP(UPDATE_MODBUS_MULTI_REGISTER, _szSymbol)
        .def_readwrite("_iRegCount", &UPDATE_MODBUS_MULTI_REGISTER::_iRegCount)
        ARRAY_PROP(UPDATE_MODBUS_MULTI_REGISTER, _iRegValue, unsigned short);
    // ─────────────────────────────────────────────────────────────────────────
    // LOCAL / SAFETY_ZONE_PROPERTY structs (compact form)
    // ─────────────────────────────────────────────────────────────────────────

    py::class_<LOCAL_ZONE_PROPERTY_JOINT_RANGE>(m, "LOCAL_ZONE_PROPERTY_JOINT_RANGE")
        .def(py::init<>())
        ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_RANGE, _iOverride, unsigned char)
        ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_RANGE, _fMinRange, float)
        ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_RANGE, _fMaxRange, float);

    py::class_<LOCAL_ZONE_PROPERTY_JOINT_SPEED>(m, "LOCAL_ZONE_PROPERTY_JOINT_SPEED")
        .def(py::init<>())
        ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_SPEED, _iOverride, unsigned char)
        ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_SPEED, _fSpeed, float);

    //bind the CDRFLEx class
    py::class_<DRAFramework::CDRFLEx>(m, "CDRFLEx")
        .def(py::init<>()) // Binds the default constructor
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

    py::class_<LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE>(m, "LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE")
        .def(py::init<>())
        .def_readwrite("_iOverride",  &LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE::_iOverride)
        .def_readwrite("_iStopMode",  &LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE::_iStopMode);

    py::class_<LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE>(m, "LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE")
        .def(py::init<>())
        .def_readwrite("_iOverride",  &LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE::_iOverride)
        .def_readwrite("_iStopMode",  &LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE::_iStopMode);

    py::class_<LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION>(m, "LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION")
        .def(py::init<>())
        .def_readwrite("_iOverride", &LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION::_iOverride)
        ARRAY_PROP(LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION, _fDirection, float)
        .def_readwrite("_fAngle", &LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION::_fAngle);

    py::class_<SAFETY_ZONE_PROPERTY_SPACE_LIMIT>(m, "SAFETY_ZONE_PROPERTY_SPACE_LIMIT")
        .def(py::init<>())
        .def_readwrite("_iInspectionType",      &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_iInspectionType)
        .def_readwrite("_tJointRangeOverride",  &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_tJointRangeOverride)
        .def_readwrite("_iDynamicZoneEnable",   &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_iDynamicZoneEnable)
        .def_readwrite("_iInsideZoneDectection",&SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_iInsideZoneDectection);

    py::class_<SAFETY_ZONE_PROPERTY_LOCAL_ZONE>(m, "SAFETY_ZONE_PROPERTY_LOCAL_ZONE")
        .def(py::init<>())
        .def_readwrite("_tJointRangeOverride",               &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tJointRangeOverride)
        .def_readwrite("_tJointSpeedOverride",               &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tJointSpeedOverride)
        .def_readwrite("_tTcpForceOverride",                 &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpForceOverride)
        .def_readwrite("_tTcpPowerOverride",                 &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpPowerOverride)
        .def_readwrite("_tTcpSpeedOverride",                 &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpSpeedOverride)
        .def_readwrite("_tTcpMomentumOverride",              &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpMomentumOverride)
        .def_readwrite("_tCollisionOverride",                &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tCollisionOverride)
        .def_readwrite("_tSpeedRate",                        &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tSpeedRate)
        .def_readwrite("_tCollisionViolationStopmodeOverride",&SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tCollisionViolationStopmodeOverride)
        .def_readwrite("_tForceViolationStopmodeOverride",   &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tForceViolationStopmodeOverride)
        .def_readwrite("_tToolOrientationLimitOverride",     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tToolOrientationLimitOverride)
        .def_readwrite("_iDynamicZoneEnable",                &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iDynamicZoneEnable)
        .def_readwrite("_iLedOverride",                      &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iLedOverride)
        .def_readwrite("_iNundgeEanble",                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iNundgeEanble)
        .def_readwrite("_iAllowLessSafeWork",                &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iAllowLessSafeWork)
            .def_readwrite("_iOverrideReduce",                   &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iOverrideReduce)
            .def_readwrite("_iInsideZoneDectection",             &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iInsideZoneDectection)
            .def_readwrite("_bCollaborativeZone",                &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_bCollaborativeZone);

        py::class_<SAFETY_ZONE_PROPERTY_DATA>(m, "SAFETY_ZONE_PROPERTY_DATA")
            .def(py::init<>())
            .def_property("_tSpaceLimitZone",
                [](const SAFETY_ZONE_PROPERTY_DATA &s) -> const SAFETY_ZONE_PROPERTY_SPACE_LIMIT & { return s._tSpaceLimitZone; },
                [](SAFETY_ZONE_PROPERTY_DATA &s, const SAFETY_ZONE_PROPERTY_SPACE_LIMIT &v) { s._tSpaceLimitZone = v; })
            .def_property("_tLocalZone",
                [](const SAFETY_ZONE_PROPERTY_DATA &s) -> const SAFETY_ZONE_PROPERTY_LOCAL_ZONE & { return s._tLocalZone; },
                [](SAFETY_ZONE_PROPERTY_DATA &s, const SAFETY_ZONE_PROPERTY_LOCAL_ZONE &v) { s._tLocalZone = v; });

        // ─────────────────────────────────────────────────────────────────────────
        // CONFIG_USER_COORDINATE / SAFETY zone shapes
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<CONFIG_USER_COORDINATE>(m, "CONFIG_USER_COORDINATE")
            .def(py::init<>())
            ARRAY_PROP(CONFIG_USER_COORDINATE, _fTargetPos, float)
            .def_readwrite("_iReqId", &CONFIG_USER_COORDINATE::_iReqId);

        py::class_<SAFETY_ZONE_SHAPE_SPHERE>(m, "SAFETY_ZONE_SHAPE_SPHERE")
            .def(py::init<>())
            .def_readwrite("_tCenter", &SAFETY_ZONE_SHAPE_SPHERE::_tCenter)
            .def_readwrite("_fRadius", &SAFETY_ZONE_SHAPE_SPHERE::_fRadius);

        py::class_<SAFETY_ZONE_SHAPE_CYLINDER>(m, "SAFETY_ZONE_SHAPE_CYLINDER")
            .def(py::init<>())
            .def_readwrite("_tCenter",   &SAFETY_ZONE_SHAPE_CYLINDER::_tCenter)
            .def_readwrite("_fRadius",   &SAFETY_ZONE_SHAPE_CYLINDER::_fRadius)
            .def_readwrite("_fZLoLimit", &SAFETY_ZONE_SHAPE_CYLINDER::_fZLoLimit)
            .def_readwrite("_fZUpLimit", &SAFETY_ZONE_SHAPE_CYLINDER::_fZUpLimit);

        py::class_<SAFETY_ZONE_SHAPE_CUBOID>(m, "SAFETY_ZONE_SHAPE_CUBOID")
            .def(py::init<>())
            .def_readwrite("_fXLoLimit", &SAFETY_ZONE_SHAPE_CUBOID::_fXLoLimit)
            .def_readwrite("_fYLoLimit", &SAFETY_ZONE_SHAPE_CUBOID::_fYLoLimit)
            .def_readwrite("_fZLoLimit", &SAFETY_ZONE_SHAPE_CUBOID::_fZLoLimit)
            .def_readwrite("_fXUpLimit", &SAFETY_ZONE_SHAPE_CUBOID::_fXUpLimit)
            .def_readwrite("_fYUpLimit", &SAFETY_ZONE_SHAPE_CUBOID::_fYUpLimit)
            .def_readwrite("_fZUpLimit", &SAFETY_ZONE_SHAPE_CUBOID::_fZUpLimit);

        py::class_<SAFETY_ZONE_SHAPE_TILTED_CUBOID>(m, "SAFETY_ZONE_SHAPE_TILTED_CUBOID")
            .def(py::init<>())
            .def_readwrite("_tOrigin",   &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tOrigin)
            .def_readwrite("_tUAxisEnd", &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tUAxisEnd)
            .def_readwrite("_tVAxisEnd", &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tVAxisEnd)
            .def_readwrite("_tWAxisEnd", &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tWAxisEnd);

        py::class_<SAFETY_ZONE_SHAPE_CAPSULE>(m, "SAFETY_ZONE_SHAPE_CAPSULE")
            .def(py::init<>())
            .def_readwrite("_tCenter1", &SAFETY_ZONE_SHAPE_CAPSULE::_tCenter1)
            .def_readwrite("_tCenter2", &SAFETY_ZONE_SHAPE_CAPSULE::_tCenter2)
            .def_readwrite("_fRadius",  &SAFETY_ZONE_SHAPE_CAPSULE::_fRadius);

        py::class_<SAFETY_ZONE_SHAPE_DATA>(m, "SAFETY_ZONE_SHAPE_DATA")
            .def(py::init<>())
            .def_property("_iBuffer",
                [](const SAFETY_ZONE_SHAPE_DATA &s) { return array_copy(s._iBuffer); },
                [](SAFETY_ZONE_SHAPE_DATA &s, py::array_t<unsigned char> v) { array_set(s._iBuffer, v); });

        py::class_<SAFETY_ZONE_SHAPE>(m, "SAFETY_ZONE_SHAPE")
            .def(py::init<>())
            .def_readwrite("_iCoordinate",  &SAFETY_ZONE_SHAPE::_iCoordinate)
            .def_readwrite("_iShapeType",   &SAFETY_ZONE_SHAPE::_iShapeType)
            .def_readwrite("_tShapeData",   &SAFETY_ZONE_SHAPE::_tShapeData)
            .def_readwrite("_fMargin",      &SAFETY_ZONE_SHAPE::_fMargin)
            .def_readwrite("_iValidSpace",  &SAFETY_ZONE_SHAPE::_iValidSpace);

        py::class_<CONFIG_ADD_SAFETY_ZONE>(m, "CONFIG_ADD_SAFETY_ZONE")
            .def(py::init<>())
            STR_PROP(CONFIG_ADD_SAFETY_ZONE, _szIdentifier)
            STR_PROP(CONFIG_ADD_SAFETY_ZONE, _szAlias)
            .def_readwrite("_iZoneType",    &CONFIG_ADD_SAFETY_ZONE::_iZoneType)
            .def_readwrite("_tZoneProperty",&CONFIG_ADD_SAFETY_ZONE::_tZoneProperty)
            .def_readwrite("_tShape",       &CONFIG_ADD_SAFETY_ZONE::_tShape);

        m.attr("CONFIG_SAFETY_ZONE") = m.attr("CONFIG_ADD_SAFETY_ZONE");

        py::class_<CONFIG_DELETE_SAFETY_ZONE>(m, "CONFIG_DELETE_SAFETY_ZONE")
            .def(py::init<>())
            STR_PROP(CONFIG_DELETE_SAFETY_ZONE, _szIdentifier);

        // ─────────────────────────────────────────────────────────────────────────
        // CONFIG_ENCODER_POLARITY / CONFIG_ENCODER_MODE
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<CONFIG_ENCODER_POLARITY>(m, "CONFIG_ENCODER_POLARITY")
            .def(py::init<>())
            .def_readwrite("_iChannel",  &CONFIG_ENCODER_POLARITY::_iChannel)
            ARRAY_PROP(CONFIG_ENCODER_POLARITY, _iPolarity, unsigned char);

        py::class_<CONFIG_ENCODER_MODE>(m, "CONFIG_ENCODER_MODE")
            .def(py::init<>())
            .def_readwrite("_iChannel",  &CONFIG_ENCODER_MODE::_iChannel)
            .def_readwrite("_iABMode",   &CONFIG_ENCODER_MODE::_iABMode)
            .def_readwrite("_iZMode",    &CONFIG_ENCODER_MODE::_iZMode)
            .def_readwrite("_iSMode",    &CONFIG_ENCODER_MODE::_iSMode)
            .def_readwrite("_iInvMode",  &CONFIG_ENCODER_MODE::_iInvMode)
            .def_readwrite("_nPulseAZ",  &CONFIG_ENCODER_MODE::_nPulseAZ);

        // ─────────────────────────────────────────────────────────────────────────
        // CONFIG_IO_FUNC / CONFIG_REMOTE_CONTROL
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<CONFIG_IO_FUNC>(m, "CONFIG_IO_FUNC")
            .def(py::init<>())
            .def_readwrite("_iPort",  &CONFIG_IO_FUNC::_iPort)
            .def_readwrite("_bLevel", &CONFIG_IO_FUNC::_bLevel);

        py::class_<CONFIG_REMOTE_CONTROL>(m, "CONFIG_REMOTE_CONTROL")
            .def(py::init<>())
            .def_readwrite("_bEnable", &CONFIG_REMOTE_CONTROL::_bEnable)
            .def_property("_tFunc",
                [](const CONFIG_REMOTE_CONTROL &s) { return array2d_copy(s._tFunc); },
                [](CONFIG_REMOTE_CONTROL &s, py::array_t<CONFIG_IO_FUNC> /*v*/) {
                    // Compound type 2D array – use raw buffer copy
                    throw std::runtime_error("Use individual element assignment for CONFIG_IO_FUNC arrays");
                });

        // ─────────────────────────────────────────────────────────────────────────
        // PROGRAM_SYNTAX_CHECK / PROGRAM_EXECUTION_EX / PROGRAM_WATCH_VARIABLE / PROGRAM_ERROR
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<PROGRAM_SYNTAX_CHECK>(m, "PROGRAM_SYNTAX_CHECK")
            .def(py::init<>())
            .def_readwrite("_iTextLength", &PROGRAM_SYNTAX_CHECK::_iTextLength);

        py::class_<PROGRAM_EXECUTION_EX>(m, "PROGRAM_EXECUTION_EX")
            .def(py::init<>())
            .def_readwrite("_iLineNumber",  &PROGRAM_EXECUTION_EX::_iLineNumber)
            .def_readwrite("_fElapseTime",  &PROGRAM_EXECUTION_EX::_fElapseTime)
            STR_PROP(PROGRAM_EXECUTION_EX, _szFile);

        py::class_<PROGRAM_WATCH_VARIABLE>(m, "PROGRAM_WATCH_VARIABLE")
            .def(py::init<>())
            .def_readwrite("_iDivision", &PROGRAM_WATCH_VARIABLE::_iDivision)
            .def_readwrite("_iType",     &PROGRAM_WATCH_VARIABLE::_iType)
            STR_PROP(PROGRAM_WATCH_VARIABLE, _szName)
            STR_PROP(PROGRAM_WATCH_VARIABLE, _szData);

        py::class_<PROGRAM_ERROR>(m, "PROGRAM_ERROR")
            .def(py::init<>())
            .def_readwrite("_iError", &PROGRAM_ERROR::_iError)
            .def_readwrite("_nLine",  &PROGRAM_ERROR::_nLine)
            STR_PROP(PROGRAM_ERROR, _szFile);

        // ─────────────────────────────────────────────────────────────────────────
        // Welding structs
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<WELDING_CHANNEL>(m, "WELDING_CHANNEL")
            .def(py::init<>())
            .def_readwrite("_bTargetCh", &WELDING_CHANNEL::_bTargetCh)
            .def_readwrite("_bTargetAT", &WELDING_CHANNEL::_bTargetAT)
            ARRAY_PROP(WELDING_CHANNEL, _ConstValue, float)
            .def_readwrite("_fMinValue", &WELDING_CHANNEL::_fMinValue)
            .def_readwrite("_fMaxValue", &WELDING_CHANNEL::_fMaxValue);

        py::class_<CONFIG_WELDING_INTERFACE>(m, "CONFIG_WELDING_INTERFACE")
            .def(py::init<>())
            .def_readwrite("_bEnable",  &CONFIG_WELDING_INTERFACE::_bEnable)
            .def_property("_tChOut",
                [](const CONFIG_WELDING_INTERFACE &s) {
                    py::list lst;
                    for (int i = 0; i < 2; ++i) lst.append(s._tChOut[i]);
                    return lst;
                },
                [](CONFIG_WELDING_INTERFACE &s, py::list v) {
                    if (v.size() != 2) throw std::out_of_range("Expected 2 elements");
                    for (int i = 0; i < 2; ++i) s._tChOut[i] = v[i].cast<WELDING_CHANNEL>();
                })
            .def_property("_tChIn",
                [](const CONFIG_WELDING_INTERFACE &s) {
                    py::list lst;
                    for (int i = 0; i < 2; ++i) lst.append(s._tChIn[i]);
                    return lst;
                },
                [](CONFIG_WELDING_INTERFACE &s, py::list v) {
                    if (v.size() != 2) throw std::out_of_range("Expected 2 elements");
                    for (int i = 0; i < 2; ++i) s._tChIn[i] = v[i].cast<WELDING_CHANNEL>();
                })
            .def_readwrite("_iArcOnDO", &CONFIG_WELDING_INTERFACE::_iArcOnDO)
            .def_readwrite("_iGasOnDO", &CONFIG_WELDING_INTERFACE::_iGasOnDO)
            .def_readwrite("_iInchPDO", &CONFIG_WELDING_INTERFACE::_iInchPDO)
            .def_readwrite("_iInchNDO", &CONFIG_WELDING_INTERFACE::_iInchNDO);

        py::class_<ROBOT_WELDING_DATA>(m, "ROBOT_WELDING_DATA")
            .def(py::init<>())
            .def_readwrite("_iAdjAvail",  &ROBOT_WELDING_DATA::_iAdjAvail)
            .def_readwrite("_fTargetVol", &ROBOT_WELDING_DATA::_fTargetVol)
            .def_readwrite("_fTargetCur", &ROBOT_WELDING_DATA::_fTargetCur)
            .def_readwrite("_fTargetVel", &ROBOT_WELDING_DATA::_fTargetVel)
            .def_readwrite("_fActualVol", &ROBOT_WELDING_DATA::_fActualVol)
            .def_readwrite("_fActualCur", &ROBOT_WELDING_DATA::_fActualCur)
            .def_readwrite("_fOffsetY",   &ROBOT_WELDING_DATA::_fOffsetY)
            .def_readwrite("_fOffsetZ",   &ROBOT_WELDING_DATA::_fOffsetZ)
            .def_readwrite("_iArcOnDO",   &ROBOT_WELDING_DATA::_iArcOnDO)
            .def_readwrite("_iGasOnDO",   &ROBOT_WELDING_DATA::_iGasOnDO)
            .def_readwrite("_iInchPDO",   &ROBOT_WELDING_DATA::_iInchPDO)
            .def_readwrite("_iInchNPO",   &ROBOT_WELDING_DATA::_iInchNPO)
            .def_readwrite("_iStatus",    &ROBOT_WELDING_DATA::_iStatus);

        m.attr("MONITORING_WELDING") = m.attr("ROBOT_WELDING_DATA");

        // ─────────────────────────────────────────────────────────────────────────
        // POSITION_EX (complex union)
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<POSITION_EX>(m, "POSITION_EX")
            .def(py::init<>())
            .def_readwrite("_pos_type", &POSITION_EX::_pos_type)
            // posj union arm
            .def_property("posj_pos",
                [](const POSITION_EX &s) { return array_copy(s._posj._pos); },
                [](POSITION_EX &s, py::array_t<float> v) { array_set(s._posj._pos, v); })
            // posx union arm
            .def_property("posx_pos",
                [](const POSITION_EX &s) { return array_copy(s._posx._pos); },
                [](POSITION_EX &s, py::array_t<float> v) { array_set(s._posx._pos, v); })
            .def_property("_ori_type",
                [](const POSITION_EX &s) { return s._posx._ori_type; },
                [](POSITION_EX &s, unsigned char v) { s._posx._ori_type = v; })
            .def_property("_sol_space",
                [](const POSITION_EX &s) { return s._posx._sol_space; },
                [](POSITION_EX &s, unsigned char v) { s._posx._sol_space = v; })
            .def_property("_multi_turn",
                [](const POSITION_EX &s) { return s._posx._multi_turn; },
                [](POSITION_EX &s, unsigned char v) { s._posx._multi_turn = v; });

        m.attr("CONFIG_TCP_EX") = m.attr("POSITION_EX");

        // ─────────────────────────────────────────────────────────────────────────
        // CONFIG_TCP_SYMBOL_EX / CONFIG_TCP_LIST_EX
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<CONFIG_TCP_SYMBOL_EX>(m, "CONFIG_TCP_SYMBOL_EX")
            .def(py::init<>())
            STR_PROP(CONFIG_TCP_SYMBOL_EX, _szSymbol)
            .def_readwrite("_tTCP", &CONFIG_TCP_SYMBOL_EX::_tTCP);

        py::class_<CONFIG_TCP_LIST_EX>(m, "CONFIG_TCP_LIST_EX")
            .def(py::init<>())
            .def_readwrite("_iToolCount", &CONFIG_TCP_LIST_EX::_iToolCount)
            .def_property("_tTooList",
                [](const CONFIG_TCP_LIST_EX &s) {
                    py::list lst;
                    for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i) lst.append(s._tTooList[i]);
                    return lst;
                },
                [](CONFIG_TCP_LIST_EX &s, py::list v) {
                    if ((int)v.size() != MAX_CONFIG_TCP_SIZE) throw std::out_of_range("Size mismatch");
                    for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i) s._tTooList[i] = v[i].cast<CONFIG_TCP_SYMBOL_EX>();
                });

        // ─────────────────────────────────────────────────────────────────────────
        // CONFIG_WORLD_COORDINATE_EX / CONFIG_USER_COORDINATE_EX2
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<CONFIG_WORLD_COORDINATE_EX>(m, "CONFIG_WORLD_COORDINATE_EX")
            .def(py::init<>())
            .def_readwrite("_iType",     &CONFIG_WORLD_COORDINATE_EX::_iType)
            .def_readwrite("_tPosition", &CONFIG_WORLD_COORDINATE_EX::_tPosition);

        py::class_<CONFIG_USER_COORDINATE_EX2>(m, "CONFIG_USER_COORDINATE_EX2")
            .def(py::init<>())
            .def_readwrite("_iTargetRef", &CONFIG_USER_COORDINATE_EX2::_iTargetRef)
            .def_readwrite("_tTargetPos", &CONFIG_USER_COORDINATE_EX2::_tTargetPos)
            .def_readwrite("_iUserID",    &CONFIG_USER_COORDINATE_EX2::_iUserID);

        // ─────────────────────────────────────────────────────────────────────────
        // ROBOT_LINK_INFO
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<ROBOT_LINK_INFO>(m, "ROBOT_LINK_INFO")
            .def(py::init<>())
            ARRAY_PROP(ROBOT_LINK_INFO, d, float)
            ARRAY_PROP(ROBOT_LINK_INFO, a, float)
            ARRAY_PROP(ROBOT_LINK_INFO, alpha, float)
            ARRAY_PROP(ROBOT_LINK_INFO, theta, float)
            ARRAY_PROP(ROBOT_LINK_INFO, offset, float)
            .def_readwrite("gradient", &ROBOT_LINK_INFO::gradient)
            .def_readwrite("rotation", &ROBOT_LINK_INFO::rotation);

        // ─────────────────────────────────────────────────────────────────────────
        // Misc utility structs
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<MEASURE_FRICTION_RESPONSE>(m, "MEASURE_FRICTION_RESPONSE")
            .def(py::init<>())
            ARRAY_PROP(MEASURE_FRICTION_RESPONSE, _iResult, unsigned char)
            ARRAY_PROP(MEASURE_FRICTION_RESPONSE, _fError, float)
            .def_property("_fPositive",
                [](const MEASURE_FRICTION_RESPONSE &s) { return array2d_copy(s._fPositive); },
                [](MEASURE_FRICTION_RESPONSE &s, py::array_t<float> v) { array2d_set(s._fPositive, v); })
            .def_property("_fNegative",
                [](const MEASURE_FRICTION_RESPONSE &s) { return array2d_copy(s._fNegative); },
                [](MEASURE_FRICTION_RESPONSE &s, py::array_t<float> v) { array2d_set(s._fNegative, v); })
            ARRAY_PROP(MEASURE_FRICTION_RESPONSE, _fTemperature, float);

        py::class_<USER_COORDINATE_MATRIX_RESPONSE>(m, "USER_COORDINATE_MATRIX_RESPONSE")
            .def(py::init<>())
            .def_property("_fOrientXYZ",
                [](const USER_COORDINATE_MATRIX_RESPONSE &s) { return array2d_copy(s._fOrientXYZ); },
                [](USER_COORDINATE_MATRIX_RESPONSE &s, py::array_t<float> v) { array2d_set(s._fOrientXYZ, v); })
            ARRAY_PROP(USER_COORDINATE_MATRIX_RESPONSE, _fTranslXYZ, float);

        py::class_<POSITION_ADDTO>(m, "POSITION_ADDTO")
            .def(py::init<>())
            ARRAY_PROP(POSITION_ADDTO, _fTargetPos, float)
            ARRAY_PROP(POSITION_ADDTO, _fTargetVal, float);

        py::class_<VIRTUAL_FENCE_RESPONSE>(m, "VIRTUAL_FENCE_RESPONSE")
            .def(py::init<>())
            ARRAY_PROP(VIRTUAL_FENCE_RESPONSE, _iCubeResult, unsigned char)
            ARRAY_PROP(VIRTUAL_FENCE_RESPONSE, _iPolyResult, unsigned char)
            .def_readwrite("_iCylinderResult", &VIRTUAL_FENCE_RESPONSE::_iCylinderResult);

        py::class_<REPORT_TCP_CLIENT>(m, "REPORT_TCP_CLIENT")
            .def(py::init<>())
            .def_readwrite("_iId",    &REPORT_TCP_CLIENT::_iId)
            .def_readwrite("_iCount", &REPORT_TCP_CLIENT::_iCount);

        py::class_<SERIAL_PORT_NAME>(m, "SERIAL_PORT_NAME")
            .def(py::init<>())
            STR_PROP(SERIAL_PORT_NAME, _szPort)
            STR_PROP(SERIAL_PORT_NAME, _szName);

        py::class_<SERIAL_SEARCH>(m, "SERIAL_SEARCH")
            .def(py::init<>())
            .def_readwrite("_nCount", &SERIAL_SEARCH::_nCount)
            .def_property("_tSerial",
                [](const SERIAL_SEARCH &s) {
                    py::list lst;
                    for (int i = 0; i < 10; ++i) lst.append(s._tSerial[i]);
                    return lst;
                },
                [](SERIAL_SEARCH &s, py::list v) {
                    if (v.size() != 10) throw std::out_of_range("Expected 10 elements");
                    for (int i = 0; i < 10; ++i) s._tSerial[i] = v[i].cast<SERIAL_PORT_NAME>();
                });

        py::class_<MONITORING_MBUS_SLAVE_COIL>(m, "MONITORING_MBUS_SLAVE_COIL")
            .def(py::init<>())
            .def_readwrite("_nCtrlDigitalInput",                 &MONITORING_MBUS_SLAVE_COIL::_nCtrlDigitalInput)
            .def_readwrite("_nCtrlDigitalOutput",                &MONITORING_MBUS_SLAVE_COIL::_nCtrlDigitalOutput)
            .def_readwrite("_nToolDigitalInput",                 &MONITORING_MBUS_SLAVE_COIL::_nToolDigitalInput)
            .def_readwrite("_nToolDigitalOutput",                &MONITORING_MBUS_SLAVE_COIL::_nToolDigitalOutput)
            .def_readwrite("_nServoOnRobot",                     &MONITORING_MBUS_SLAVE_COIL::_nServoOnRobot)
            .def_readwrite("_nEmergencyStopped",                 &MONITORING_MBUS_SLAVE_COIL::_nEmergencyStopped)
            .def_readwrite("_nSafetyStopped",                    &MONITORING_MBUS_SLAVE_COIL::_nSafetyStopped)
            .def_readwrite("_nDirectTeachButtonPress",           &MONITORING_MBUS_SLAVE_COIL::_nDirectTeachButtonPress)
            .def_readwrite("_nPowerButtonPress",                 &MONITORING_MBUS_SLAVE_COIL::_nPowerButtonPress)
            .def_readwrite("_nSafetyStoppedRequiredRecoveryMode",&MONITORING_MBUS_SLAVE_COIL::_nSafetyStoppedRequiredRecoveryMode);

        py::class_<MONITORING_MBUS_SLAVE_HOILDING_REGISTER>(m, "MONITORING_MBUS_SLAVE_HOILDING_REGISTER")
            .def(py::init<>())
            .def_readwrite("_nCtrlDigitalInput",      &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlDigitalInput)
            .def_readwrite("_nCtrlDigitalOutput",     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlDigitalOutput)
            .def_readwrite("_nCtrlAnalogInput1",      &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput1)
            .def_readwrite("_nCtrlAnalogInput1Type",  &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput1Type)
            .def_readwrite("_nCtrlAnalogInput2",      &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput2)
            .def_readwrite("_nCtrlAnalogInput2Type",  &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput2Type)
            .def_readwrite("_nCtrlAnalogOutput1",     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput1)
            .def_readwrite("_nCtrlAnalogOutput1Type", &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput1Type)
            .def_readwrite("_nCtrlAnalogOutput2",     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput2)
            .def_readwrite("_nCtrlAnalogOutput2Type", &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput2Type)
            .def_readwrite("_nCtrlToolDigitalInput",  &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlToolDigitalInput)
            .def_readwrite("_nCtrlToolDigitalOutput", &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlToolDigitalOutput)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nGPR, unsigned short)
            .def_readwrite("_nCtrlMajorVer",          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlMajorVer)
            .def_readwrite("_nCtrlMinorVer",          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlMinorVer)
            .def_readwrite("_nCtrlPatchVer",          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlPatchVer)
            .def_readwrite("_nRobotState",            &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nRobotState)
            .def_readwrite("_nServoOnRobot",          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nServoOnRobot)
            .def_readwrite("_nEmergencyStopped",      &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nEmergencyStopped)
            .def_readwrite("_nSafetyStopped",         &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nSafetyStopped)
            .def_readwrite("_nDirectTeachButtonPressed",&MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nDirectTeachButtonPressed)
            .def_readwrite("_nPowerButtonPressed",    &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nPowerButtonPressed)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nJointPosition, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nJointVelocity, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nJointMotorCurrent, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nJointMotorTemp, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nJointTorque, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nTaskPosition, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nTaskVelocity, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nToolOffsetLength, unsigned short)
            ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nTaskExternalForce, unsigned short);

        py::class_<MONITORING_IE_GPR>(m, "MONITORING_IE_GPR")
            .def(py::init<>())
            ARRAY_PROP(MONITORING_IE_GPR, _nGpr, unsigned char);

        py::class_<MONITORING_IE_SLAVE>(m, "MONITORING_IE_SLAVE")
            .def(py::init<>())
            .def_readwrite("_tMbusCoil",              &MONITORING_IE_SLAVE::_tMbusCoil)
            .def_readwrite("_tMbusHoldingRegister",   &MONITORING_IE_SLAVE::_tMbusHoldingRegister)
            .def_readwrite("_tIndustrialEthernetGPR", &MONITORING_IE_SLAVE::_tIndustrialEthernetGPR);

        py::class_<CONFIG_SAFETY_PARAM_ENABLE>(m, "CONFIG_SAFETY_PARAM_ENABLE")
            .def(py::init<>())
            .def_readwrite("_wPreviousCmdid", &CONFIG_SAFETY_PARAM_ENABLE::_wPreviousCmdid)
            .def_readwrite("_iRefCrc32",      &CONFIG_SAFETY_PARAM_ENABLE::_iRefCrc32);

        // ─────────────────────────────────────────────────────────────────────────
        // USER_COORD_EXTERNAL_FORCE structs
        // ─────────────────────────────────────────────────────────────────────────
        py::class_<USER_COORD_EXTERNAL_FORCE>(m, "USER_COORD_EXTERNAL_FORCE")
            .def(py::init<>())
            .def_readwrite("_iUserId", &USER_COORD_EXTERNAL_FORCE::_iUserId)
            ARRAY_PROP(USER_COORD_EXTERNAL_FORCE, _fExternalForce, float);

        py::class_<USER_COORD_EXTERNAL_FORCE_INFO>(m, "USER_COORD_EXTERNAL_FORCE_INFO")
            .def(py::init<>())
            .def_readwrite("bIsMonitoring", &USER_COORD_EXTERNAL_FORCE_INFO::bIsMonitoring)
            ARRAY_PROP(USER_COORD_EXTERNAL_FORCE_INFO, iUserID, unsigned char);

        py::class_<DIGITAL_WELDING_COMM_STATE>(m, "DIGITAL_WELDING_COMM_STATE")
            .def(py::init<>())
            .def_readwrite("_cWeldingMachineOnline",    &DIGITAL_WELDING_COMM_STATE::_cWeldingMachineOnline)
            .def_readwrite("_cWeldingEipSlaveState",    &DIGITAL_WELDING_COMM_STATE::_cWeldingEipSlaveState);

        py::class_<ROBOT_LED_CONFIG>(m, "ROBOT_LED_CONFIG")
            .def(py::init<>())
            .def_readwrite("_szLedRule",     &ROBOT_LED_CONFIG::_szLedRule)
            .def_readwrite("_szCommandColor",&ROBOT_LED_CONFIG::_szCommandColor)
            .def_property("_szStateColor",
                [](const ROBOT_LED_CONFIG &s) { return array2d_copy(s._szStateColor); },
                [](ROBOT_LED_CONFIG &s, py::array_t<unsigned char> v) { array2d_set(s._szStateColor, v); });

        py::class_<COUNTER_BALANCE_PARAM_DATA>(m, "COUNTER_BALANCE_PARAM_DATA")
            .def(py::init<>())
            .def_readwrite("_fK",    &COUNTER_BALANCE_PARAM_DATA::_fK)
            .def_readwrite("_fIrod", &COUNTER_BALANCE_PARAM_DATA::_fIrod)
            .def_readwrite("_fR",    &COUNTER_BALANCE_PARAM_DATA::_fR)
            .def_readwrite("_fSi",   &COUNTER_BALANCE_PARAM_DATA::_fSi);

        py::class_<CALIBRATION_PARAM_DATA>(m, "CALIBRATION_PARAM_DATA")
            .def(py::init<>())
            .def_readwrite("_Ax", &CALIBRATION_PARAM_DATA::_Ax)
            .def_readwrite("_Bx", &CALIBRATION_PARAM_DATA::_Bx)
            .def_readwrite("_Cx", &CALIBRATION_PARAM_DATA::_Cx)
            .def_readwrite("_Dx", &CALIBRATION_PARAM_DATA::_Dx);

        py::class_<LICENSE_TEXT_PARAM>(m, "LICENSE_TEXT_PARAM")
            .def(py::init<>())
            .def_readwrite("_bLicenseInController", &LICENSE_TEXT_PARAM::_bLicenseInController)
            ARRAY_PROP(LICENSE_TEXT_PARAM, _szLicenseKey, unsigned char);

        py::class_<WEAVING_OFFSET>(m, "WEAVING_OFFSET")
            .def(py::init<>())
            .def_readwrite("_fOffsetY", &WEAVING_OFFSET::_fOffsetY)
            .def_readwrite("_fOffsetZ", &WEAVING_OFFSET::_fOffsetZ);

        py::class_<IETHERNET_SLAVE_DATA_EX>(m, "IETHERNET_SLAVE_DATA_EX")
            .def(py::init<>())
            .def_readwrite("_iGprType", &IETHERNET_SLAVE_DATA_EX::_iGprType)
            .def_readwrite("_iGprAddr", &IETHERNET_SLAVE_DATA_EX::_iGprAddr)
            .def_readwrite("_iInOut",   &IETHERNET_SLAVE_DATA_EX::_iInOut);

        py::class_<IETHERNET_SLAVE_RESPONSE_DATA_EX>(m, "IETHERNET_SLAVE_RESPONSE_DATA_EX")
            .def(py::init<>())
            .def_readwrite("_iGprType", &IETHERNET_SLAVE_RESPONSE_DATA_EX::_iGprType)
            .def_readwrite("_iGprAddr", &IETHERNET_SLAVE_RESPONSE_DATA_EX::_iGprAddr)
            .def_readwrite("_iInOut",   &IETHERNET_SLAVE_RESPONSE_DATA_EX::_iInOut)
            STR_PROP(IETHERNET_SLAVE_RESPONSE_DATA_EX, _szData);
}
