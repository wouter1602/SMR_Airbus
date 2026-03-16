/*
 *
 */
#include "./cdrflex_bindings.hpp"
#include <pybind11/cast.h>
#include <pybind11/functional.h>
#include <sys/types.h>
#include <stdexcept>
#include <optional>
#include <string>

using Class = py::class_<DRAFramework::CDRFLEx>;

// Helper functions
// Validate and return pointer to a 1D float array of expected size
static float* checkArray1D(py::array_t<float, py::array::c_style | py::array::forcecast>& arr,
                            int expected, const char* name)
{
    if (arr.size() != expected)
        throw std::runtime_error(std::string(name) + " must have exactly "
                                 + std::to_string(expected) + " elements");
    return const_cast<float*>(arr.data());
}

// Validate and return pointer to a 2D float array of expected shape [rows][cols]
static float* checkArray2D(py::array_t<float, py::array::c_style | py::array::forcecast>& arr,
                            int rows, int cols, const char* name)
{
    if (arr.ndim() != 2 || arr.shape(0) != rows || arr.shape(1) != cols)
        throw std::runtime_error(std::string(name) + " must have shape ("
                                 + std::to_string(rows) + ", " + std::to_string(cols) + ")");
    return const_cast<float*>(arr.data());
}



// Connection functions
void bOpenConnection(Class& c) {
    c.def("open_connection",
        [](DRAFramework::CDRFLEx& self, string ipAddr, unsigned int port){
            return self.open_connection(ipAddr, port);
        },
        py::arg("ip") = "192.168.137.100",
        py::arg("port") = 12345,
        "Connect to the Doosan robot controller");
}

void bCloseConnection(Class& c) {
    c.def("close_connection",
    [](DRAFramework::CDRFLEx& self) {
        return self.close_connection();
    },
    "Close connection to the Doosan robot controller");
}

// Robot property functions
void bGetSystemVersion(Class& c) {
    c.def("get_system_version",
        [](DRAFramework::CDRFLEx& self, LPSYSTEM_VERSION version) {
            return self.get_system_version(version);
        },
        py::arg("version"),
        "Get the system version");
}

void bGetLibraryVersion(Class& c) {
    c.def("get_library_version",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_library_version();
        },
        "Get the library version");
}

void bGetRobotMode(Class& c) {
    c.def("get_robot_mode",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_robot_mode();
        },
        "Get the robot mode");
}

void bSetRobotMode(Class& c) {
    c.def("set_robot_mode",
        [](DRAFramework::CDRFLEx& self, ROBOT_MODE mode) {
            return self.set_robot_mode(mode);
        },
        py::arg("mode"),
        "Set the robot mode");
}

void bGetRobotState(Class& c){
    c.def("get_robot_state",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_robot_state();
        },
        "Get the robot state");
}

void bSetRobotControl(Class& c){
    c.def("set_robot_control",
        [](DRAFramework::CDRFLEx& self, ROBOT_CONTROL control) {
            return self.set_robot_control(control);
        },
        py::arg("control"),
        "Set the robot control");
}

void bSetRobotSystem(Class& c){
    c.def("set_robot_system",
        [](DRAFramework::CDRFLEx& self, ROBOT_SYSTEM system) {
            return self.set_robot_system(system);
        },
        py::arg("system"),
        "Set the robot system");
}

void bGetRobotSpeedMode(Class& c){
    c.def("get_robot_speed_mode",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_robot_speed_mode();
        },
        "Get the robot speed mode");
}

void bSetRobotSpeedMode(Class& c){
    c.def("set_robot_speed_mode",
        [](DRAFramework::CDRFLEx& self, SPEED_MODE mode) {
            return self.set_robot_speed_mode(mode);
        },
        py::arg("mode"),
        "Set the robot speed mode");
}

void bGetProgramState(Class& c){
    c.def("get_program_state",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_program_state();
        },
        "Get the program state");
}

void bGetRobotSystem(Class& c){
    c.def("get_robot_system",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_robot_system();
        },
        "Get the robot system");
}

void bSetSafeStopResetType(Class& c){
    c.def("set_safe_stop_reset_type",
        [](DRAFramework::CDRFLEx& self, SAFE_STOP_RESET_TYPE type) {
            return self.set_safe_stop_reset_type(type);
        },
        py::arg("type"),
        "Set the safe stop reset type");
}

void bGetCurrentPose(Class& c){
    c.def("get_current_pose",
        [](DRAFramework::CDRFLEx& self, ROBOT_SPACE spaceType) {
            return self.get_current_pose(spaceType);
        },
        py::arg("spaceType") = ROBOT_SPACE::ROBOT_SPACE_JOINT,
        "Get the current pose");
}

void bGetcurrentPosj(Class& c){
    c.def("get_current_posj",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_current_posj();
        },
        "Get the current position in joint space");
}

void bGetDesiredPosj(Class& c){
    c.def("get_desired_posj",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_desired_posj();
        },
        "Get the desired position in joint space");
}

void bGetCurrentVelj(Class& c){
    c.def("get_current_velj",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_current_velj();
        },
        "Get the current velocity in joint space");
}

void bGetCurrentPosx(Class& c){
    c.def("get_current_posx",
        [](DRAFramework::CDRFLEx& self, COORDINATE_SYSTEM coodType) {
            return self.get_current_posx(coodType);
        },
        py::arg("coodType") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
        "Get the current position in Cartesian space");
}

void bGetDesiredPosx(Class& c){
    c.def("get_desired_posx",
        [](DRAFramework::CDRFLEx& self, COORDINATE_SYSTEM coodType) {
            return self.get_desired_posx(coodType);
        },
        py::arg("coodType") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
        "Get the desired position in Cartesian space");
}

void bGetCurrentToolFlangePosx(Class& c){
    c.def("get_current_tool_flange_posx",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_current_tool_flange_posx();
        },
        "Get the current tool flange position in Cartesian space");
}

void bGetCurrentVelx(Class& c){
    c.def("get_current_velx",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_current_velx();
        },
        "Get the current velocity in Cartesian space");
}

void bGetDesiredVelx(Class& c){
    c.def("get_desired_velx",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_desired_velx();
        },
        "Get the desired velocity in Cartesian space");
}

void bGetJointTorque(Class& c){
    c.def("get_joint_torque",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_joint_torque();
        },
        "Get the joint torque");
}

void bGetControlSpace(Class& c){
    c.def("get_control_space",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_control_space();
        },
        "Get the control space");
}

void bGetExternalTorque(Class& c){
    c.def("get_external_torque",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_external_torque();
        },
        "Get the external torque");
}

void bGetToolForce(Class& c){
    c.def("get_tool_force",
        [](DRAFramework::CDRFLEx& self, COORDINATE_SYSTEM targetRef) {
            return self.get_tool_force(targetRef);
        },
        py::arg("targetRef") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
        "Get the tool force");
}

void bGetCurrentSolutionSpace(Class& c){
    c.def("get_current_solution_space",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_current_solution_space();
        },
        "Get the current solution space");
}

void bGetLastalarm(Class& c){
    c.def("get_last_alarm",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_last_alarm();
        },
        "Get the last alarm");
}

void bGetSolutionSpace(Class& c){
    c.def("get_solution_space",
        [](DRAFramework::CDRFLEx& self, py::array_t<float> targetPos) {
            if (targetPos.size() != NUM_JOINT)
                throw std::runtime_error("Pos must have exactly 6 elements.");
            return self.get_solution_space(targetPos.mutable_data());
        },
        py::arg("targetPos"),
        "Get the solution space");
}

void bGetOrientationError(Class& c){
    c.def("get_orientation_error",
        [](DRAFramework::CDRFLEx& self, py::array_t<float> position1, py::array_t<float> position2, TASK_AXIS taskAxis) {
            if (position1.size() != NUM_TASK || position2.size() != NUM_TASK)
                throw std::runtime_error("Position arrays must have exactly 6 elements.");
            return self.get_orientation_error(position1.mutable_data(), position2.mutable_data(), taskAxis);
        },
        py::arg("position1"),
        py::arg("position2"),
        py::arg("taskAxis"),
        "Get the orientation error");
}

void bGetControlMode(Class& c){
    c.def("get_control_mode",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_control_mode();
        },
        "Get the control mode");
}

void bGetCurrentRotm(Class& c){
    c.def("get_current_rotm",
        [](DRAFramework::CDRFLEx& self, COORDINATE_SYSTEM targetRef) {
            float (*ptr)[3] = self.get_current_rotm(targetRef);

            // Copy the data into a numpy array
            // IMPORTANT: We copy because the original pointer may become
            // invalid
            py::array_t<float> result(3);
            auto buf = result.request();
            float *result_ptr = static_cast<float *>(buf.ptr);

            // Copy 3 elements
            for (int i = 0; i < 3; i++) {
              result_ptr[i] = (*ptr)[i];
            }

            return result;
        },
        py::arg("targetRef") = COORDINATE_SYSTEM::COORDINATE_SYSTEM_BASE,
        "Get the current rotation matrix");
}

void bGetSafetyConfiguration(Class& c){
    c.def("get_safety_configuration",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_safety_configuration();
        },
        "Get the safety configuration");
}

// Callback functions
static py::function s_cb_homming_completed;
static void trampoline_homming_completed() {
    py::gil_scoped_acquire gil;
    if (s_cb_homming_completed) s_cb_homming_completed();
}
void bSetOnHommingCompleted(Class& c) {
    c.def("set_on_homming_completed",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_homming_completed = std::move(fn);
            self.set_on_homming_completed(
                s_cb_homming_completed.is_none() ? nullptr : trampoline_homming_completed);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_tp_initializing_completed;
static void trampoline_tp_initializing_completed() {
    py::gil_scoped_acquire gil;
    if (s_cb_tp_initializing_completed) s_cb_tp_initializing_completed();
}
void bSetOnTpInitializingCompleted(Class& c) {
    c.def("set_on_tp_initializing_completed",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_tp_initializing_completed = std::move(fn);
            self.set_on_tp_initializing_completed(
                s_cb_tp_initializing_completed.is_none() ? nullptr : trampoline_tp_initializing_completed);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_mastering_need;
static void trampoline_mastering_need() {
    py::gil_scoped_acquire gil;
    if (s_cb_mastering_need) s_cb_mastering_need();
}
void bSetOnMasteringNeed(Class& c) {
    c.def("set_on_mastering_need",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_mastering_need = std::move(fn);
            self.set_on_mastering_need(
                s_cb_mastering_need.is_none() ? nullptr : trampoline_mastering_need);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_disconnected;
static void trampoline_disconnected() {
    py::gil_scoped_acquire gil;
    if (s_cb_disconnected) s_cb_disconnected();
}
void bSetOnDisconnected(Class& c) {
    c.def("set_on_disconnected",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_disconnected = std::move(fn);
            self.set_on_disconnected(
                s_cb_disconnected.is_none() ? nullptr : trampoline_disconnected);
        }, py::arg("pCallbackFunc"));
}

// ── Enum callbacks ────────────────────────────────────────────────────────────
// Passed to Python as int. Cast back with the enum type on the C++ side if needed.

static py::function s_cb_monitoring_state;
static void trampoline_monitoring_state(const ROBOT_STATE eState) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_state) s_cb_monitoring_state(static_cast<int>(eState));
}
void bSetOnMonitoringState(Class& c) {
    c.def("set_on_monitoring_state",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_state = std::move(fn);
            self.set_on_monitoring_state(
                s_cb_monitoring_state.is_none() ? nullptr : trampoline_monitoring_state);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_safety_state;
static void trampoline_monitoring_safety_state(const SAFETY_STATE eState) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_safety_state) s_cb_monitoring_safety_state(static_cast<int>(eState));
}
void bSetOnMonitoringSafetyState(Class& c) {
    c.def("set_on_monitoring_safety_state",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_safety_state = std::move(fn);
            self.set_on_monitoring_safety_state(
                s_cb_monitoring_safety_state.is_none() ? nullptr : trampoline_monitoring_safety_state);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_robot_system;
static void trampoline_monitoring_robot_system(const ROBOT_SYSTEM eSystem) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_robot_system) s_cb_monitoring_robot_system(static_cast<int>(eSystem));
}
void bSetOnMonitoringRobotSystem(Class& c) {
    c.def("set_on_monitoring_robot_system",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_robot_system = std::move(fn);
            self.set_on_monitoring_robot_system(
                s_cb_monitoring_robot_system.is_none() ? nullptr : trampoline_monitoring_robot_system);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_access_control;
static void trampoline_monitoring_access_control(const MONITORING_ACCESS_CONTROL eCtrl) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_access_control) s_cb_monitoring_access_control(static_cast<int>(eCtrl));
}
void bSetOnMonitoringAccessControl(Class& c) {
    c.def("set_on_monitoring_access_control",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_access_control = std::move(fn);
            self.set_on_monitoring_access_control(
                s_cb_monitoring_access_control.is_none() ? nullptr : trampoline_monitoring_access_control);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_program_stopped;
static void trampoline_program_stopped(const PROGRAM_STOP_CAUSE eCause) {
    py::gil_scoped_acquire gil;
    if (s_cb_program_stopped) s_cb_program_stopped(static_cast<int>(eCause));
}
void bSetOnProgramStopped(Class& c) {
    c.def("set_on_program_stopped",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_program_stopped = std::move(fn);
            self.set_on_program_stopped(
                s_cb_program_stopped.is_none() ? nullptr : trampoline_program_stopped);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_speed_mode;
static void trampoline_monitoring_speed_mode(const MONITORING_SPEED eSpeed) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_speed_mode) s_cb_monitoring_speed_mode(static_cast<int>(eSpeed));
}
void bSetOnMonitoringSpeedMode(Class& c) {
    c.def("set_on_monitoring_speed_mode",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_speed_mode = std::move(fn);
            self.set_on_monitoring_speed_mode(
                s_cb_monitoring_speed_mode.is_none() ? nullptr : trampoline_monitoring_speed_mode);
        }, py::arg("pCallbackFunc"));
}

// ── String callback ───────────────────────────────────────────────────────────
// TOnTpLogCB: const char[256] → converted to Python str

static py::function s_cb_tp_log;
static void trampoline_tp_log(const char szLog[256]) {
    py::gil_scoped_acquire gil;
    if (s_cb_tp_log) s_cb_tp_log(std::string(szLog, strnlen(szLog, 256)));
}
void bSetOnTpLog(Class& c) {
    c.def("set_on_tp_log",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_tp_log = std::move(fn);
            self.set_on_tp_log(
                s_cb_tp_log.is_none() ? nullptr : trampoline_tp_log);
        }, py::arg("pCallbackFunc"));
}

// ── Struct pointer callbacks ──────────────────────────────────────────────────
// LP* pointers are passed to Python as py::capsule with the struct type name
// as the capsule name. Access fields by exposing the struct with py::class_,
// or read them on the C++ side before forwarding to Python.

static py::function s_cb_monitoring_data;
static void trampoline_monitoring_data(const LPMONITORING_DATA pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_data)
        s_cb_monitoring_data(py::capsule(pData, "LPMONITORING_DATA"));
}
void bSetOnMonitoringData(Class& c) {
    c.def("set_on_monitoring_data",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_data = std::move(fn);
            self.set_on_monitoring_data(
                s_cb_monitoring_data.is_none() ? nullptr : trampoline_monitoring_data);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_data_ex;
static void trampoline_monitoring_data_ex(const LPMONITORING_DATA_EX pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_data_ex)
        s_cb_monitoring_data_ex(py::capsule(pData, "LPMONITORING_DATA_EX"));
}
void bSetOnMonitoringDataEx(Class& c) {
    c.def("set_on_monitoring_data_ex",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_data_ex = std::move(fn);
            self.set_on_monitoring_data_ex(
                s_cb_monitoring_data_ex.is_none() ? nullptr : trampoline_monitoring_data_ex);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_ctrl_io;
static void trampoline_monitoring_ctrl_io(const LPMONITORING_CTRLIO pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_ctrl_io)
        s_cb_monitoring_ctrl_io(py::capsule(pData, "LPMONITORING_CTRLIO"));
}
void bSetOnMonitoringCtrlIo(Class& c) {
    c.def("set_on_monitoring_ctrl_io",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_ctrl_io = std::move(fn);
            self.set_on_monitoring_ctrl_io(
                s_cb_monitoring_ctrl_io.is_none() ? nullptr : trampoline_monitoring_ctrl_io);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_ctrl_io_ex;
static void trampoline_monitoring_ctrl_io_ex(const LPMONITORING_CTRLIO_EX pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_ctrl_io_ex)
        s_cb_monitoring_ctrl_io_ex(py::capsule(pData, "LPMONITORING_CTRLIO_EX"));
}
void bSetOnMonitoringCtrlIoEx(Class& c) {
    c.def("set_on_monitoring_ctrl_io_ex",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_ctrl_io_ex = std::move(fn);
            self.set_on_monitoring_ctrl_io_ex(
                s_cb_monitoring_ctrl_io_ex.is_none() ? nullptr : trampoline_monitoring_ctrl_io_ex);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_monitoring_modbus;
static void trampoline_monitoring_modbus(const LPMONITORING_MODBUS pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_monitoring_modbus)
        s_cb_monitoring_modbus(py::capsule(pData, "LPMONITORING_MODBUS"));
}
void bSetOnMonitoringModbus(Class& c) {
    c.def("set_on_monitoring_modbus",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_monitoring_modbus = std::move(fn);
            self.set_on_monitoring_modbus(
                s_cb_monitoring_modbus.is_none() ? nullptr : trampoline_monitoring_modbus);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_log_alarm;
static void trampoline_log_alarm(LPLOG_ALARM pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_log_alarm)
        s_cb_log_alarm(py::capsule(pData, "LPLOG_ALARM"));
}
void bSetOnLogAlarm(Class& c) {
    c.def("set_on_log_alarm",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_log_alarm = std::move(fn);
            self.set_on_log_alarm(
                s_cb_log_alarm.is_none() ? nullptr : trampoline_log_alarm);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_tp_popup;
static void trampoline_tp_popup(LPMESSAGE_POPUP pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_tp_popup)
        s_cb_tp_popup(py::capsule(pData, "LPMESSAGE_POPUP"));
}
void bSetOnTpPopup(Class& c) {
    c.def("set_on_tp_popup",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_tp_popup = std::move(fn);
            self.set_on_tp_popup(
                s_cb_tp_popup.is_none() ? nullptr : trampoline_tp_popup);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_tp_progress;
static void trampoline_tp_progress(LPMESSAGE_PROGRESS pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_tp_progress)
        s_cb_tp_progress(py::capsule(pData, "LPMESSAGE_PROGRESS"));
}
void bSetOnTpProgress(Class& c) {
    c.def("set_on_tp_progress",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_tp_progress = std::move(fn);
            self.set_on_tp_progress(
                s_cb_tp_progress.is_none() ? nullptr : trampoline_tp_progress);
        }, py::arg("pCallbackFunc"));
}

static py::function s_cb_tp_get_user_input;
static void trampoline_tp_get_user_input(LPMESSAGE_INPUT pData) {
    py::gil_scoped_acquire gil;
    if (s_cb_tp_get_user_input)
        s_cb_tp_get_user_input(py::capsule(pData, "LPMESSAGE_INPUT"));
}
void bSetOnTpGetUserInput(Class& c) {
    c.def("set_on_tp_get_user_input",
        [](DRAFramework::CDRFLEx& self, py::function fn) {
            s_cb_tp_get_user_input = std::move(fn);
            self.set_on_tp_get_user_input(
                s_cb_tp_get_user_input.is_none() ? nullptr : trampoline_tp_get_user_input);
        }, py::arg("pCallbackFunc"));
}

// Acces control functions
void bManageAccessControl(Class& c) {
    c.def("manage_access_control",
        [](DRAFramework::CDRFLEx& self, MANAGE_ACCESS_CONTROL accessControl) {
            return self.manage_access_control(accessControl);
        },
        py::arg("accessControl") = MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_REQUEST,
        "Manage access control");
}

// Basic control functions
void bJog(Class& c){
    c.def("jog",
        [](DRAFramework::CDRFLEx& self, JOG_AXIS jogAxis, MOVE_REFERENCE moveReference, float velocity) {
            return self.jog(jogAxis, moveReference, velocity);
        },
        py::arg("axis"),
        py::arg("reference"),
        py::arg("velocity"),
        "Jog the robot");
}

void bMoveHome(Class& c){
    c.def("move_home",
        [](DRAFramework::CDRFLEx& self, MOVE_HOME mode, unsigned char run) {
            return self.move_home();
        },
        py::arg("mode") = MOVE_HOME::MOVE_HOME_MECHANIC,
        py::arg("run") = (unsigned char)1,
        "Move the robot to home position");
}

// Robot motion functions

// movej
void bMovej(Class& c) {
    c.def(
        "movej",
        [](DRAFramework::CDRFLEx &self, py::array_t<float> targetPos,
           float targetVal, float targetAcc, float targetTime,
           MOVE_MODE moveMode, float bendingRadius,
           BLENDING_SPEED_TYPE blendingType) {
          if (targetPos.size() != NUM_JOINT)
            throw std::runtime_error("Pos must have exactly 6 elements");
          return self.movej(targetPos.mutable_data(), targetVal, targetAcc,
                            targetTime, moveMode, bendingRadius,
                            blendingType);
        },
        py::arg("pos"), py::arg("vel"), py::arg("acc"), py::arg("time") = 0.f,
        py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE,
        py::arg("blending_radius") = 0.f,
        py::arg("blending_type") =
            BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
        "Movej array command");

    c.def(
        "movej",
        [](DRAFramework::CDRFLEx &self, py::array_t<float> targetPos,
           py::array_t<float> targetVel, py::array_t<float> targetAcc,
           float targetTime, MOVE_MODE moveMode, float blendingRadius,
           BLENDING_SPEED_TYPE blendingType) {
          if (targetPos.size() != NUM_JOINT)
            throw std::runtime_error("Pos must have exactly 6 elements.");
          if (targetVel.size() != NUM_JOINT)
            throw std::runtime_error("Vel must have exactly 6 elements.");
          if (targetAcc.size() != NUM_JOINT)
            throw std::runtime_error("Acc must have exactly 6 elements.");
          return self.movej(targetPos.mutable_data(),
                            targetVel.mutable_data(),
                            targetAcc.mutable_data(), targetTime, moveMode,
                            blendingRadius, blendingType);
        },
        py::arg("pos"), py::arg("vel"), py::arg("acc"), py::arg("time") = 0.f,
        py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE,
        py::arg("blending_radius") = 0.f,
        py::arg("blending_type") =
            BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
        "Movej array command");
}

// Movel funciton
void bMovel(Class& c) {
    c.def(
        "movel",
        [](DRAFramework::CDRFLEx &self, py::array_t<float> targetPos,
           py::array_t<float> targetVel, py::array_t<float> targetAcc,
           float targetTime, MOVE_MODE moveMode, MOVE_REFERENCE moveReference,
           float blendingRadius, BLENDING_SPEED_TYPE blendingType,
           DR_MV_APP appType) {
          if (targetPos.size() != NUM_TASK)
            throw std::runtime_error("pos must have exactly 6 elements.");
          if (targetVel.size() != 2)
            throw std::runtime_error("Vel must have exactly 2 elements.");
          if (targetAcc.size() != 2)
            throw std::runtime_error("Acc must have exactly 2 elements.");
          return self.movel(
              targetPos.mutable_data(), targetVel.mutable_data(),
              targetAcc.mutable_data(), targetTime, moveMode, moveReference,
              blendingRadius, blendingType, appType);
        },
        py::arg("pos"), py::arg("vel"), py::arg("acc"), py::arg("time") = 0.f,
        py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE,
        py::arg("move_reference") = MOVE_REFERENCE::MOVE_REFERENCE_BASE,
        py::arg("blending_radius") = 0.f,
        py::arg("blending_type") =
            BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
        py::arg("app_type") = DR_MV_APP::DR_MV_APP_NONE, "Movel command");
}

// movejx function
void bMovejx(Class& c) {
    c.def(
        "movejx",
        [](DRAFramework::CDRFLEx &self, py::array_t<float> targetPos,
           unsigned char solutionSpace, float targetVel, float targetAcc,
           float targetTime, MOVE_MODE moveMode, MOVE_REFERENCE moveReference,
           float blendingRadius, BLENDING_SPEED_TYPE blendingType) {
          if (targetPos.size() != NUM_TASK)
            throw std::runtime_error("Pos must have exactly 6 elements.");
          return self.movejx(targetPos.mutable_data(), solutionSpace,
                             targetVel, targetAcc, targetTime, moveMode,
                             moveReference, blendingRadius, blendingType);
        },
        py::arg("pos"), py::arg("solution_space"), py::arg("vel"),
        py::arg("acc"), py::arg("time") = 0.f,
        py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE,
        py::arg("move_reference") = MOVE_REFERENCE::MOVE_REFERENCE_BASE,
        py::arg("blending_radius") = 0.1f,
        py::arg("blending_type") =
            BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
        "movejx command");

    c.def(
        "movejx",
        [](DRAFramework::CDRFLEx &self, py::array_t<float> targetPos,
           unsigned char solutionSpace, py::array_t<float> targetVel,
           py::array_t<float> targetAcc, float targetTime, MOVE_MODE moveMode,
           MOVE_REFERENCE moveReverence, float blendingRadius,
           BLENDING_SPEED_TYPE blendingType) {
          if (targetPos.size() != NUM_JOINT)
            throw std::runtime_error("Pos must have exactly 6 elements.");
          if (targetVel.size() != NUM_JOINT)
            throw std::runtime_error("vel must have exactly 6 elements.");
          if (targetAcc.size() != NUM_JOINT)
            throw std::runtime_error("Acc must have exactly 6 elements.");
          return self.movejx(targetPos.mutable_data(), solutionSpace,
                             targetVel.mutable_data(),
                             targetAcc.mutable_data(), targetTime, moveMode,
                             moveReverence, blendingRadius, blendingType);
        },
        py::arg("pos"), py::arg("solution_space"), py::arg("vel"),
        py::arg("acc"), py::arg("time") = 0.f,
        py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE,
        py::arg("move_reference") = MOVE_REFERENCE::MOVE_REFERENCE_BASE,
        py::arg("blending_radius") = 0.1f,
        py::arg("blending_type") =
            BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
        "movejx command");
}

// movec function
void bMovec(Class& c){
    c.def(
        "movec",
        [](DRAFramework::CDRFLEx &self,
           py::array_t<float, py::array::c_style | py::array::forcecast>
               targetPos,
           py::array_t<float> targetVel, py::array_t<float> targetAcc,
           float targetTime, MOVE_MODE moveMode, MOVE_REFERENCE moveReference,
           float targetAngle1, float targetAngle2, float blendingRadius,
           BLENDING_SPEED_TYPE blendingType, MOVE_ORIENTATION orientation,
           DR_MV_APP appType) {
          if (targetPos.ndim() != 2 || targetPos.shape(0) != 2 ||
              targetPos.shape(1) != NUM_TASK)
            throw std::runtime_error("targetPos must have shape (2, 6)");
          if (targetVel.size() != 2)
            throw std::runtime_error(
                "targetVel must have exactly 2 elements");
          if (targetAcc.size() != 2)
            throw std::runtime_error(
                "targetAcc must have exactly 2 elements");

          // Cast to 2D C array pointer for the C++ call
          float (*pTargetPos)[NUM_TASK] =
              reinterpret_cast<float (*)[NUM_TASK]>(targetPos.mutable_data());

          return self.movec(
              pTargetPos, targetVel.mutable_data(), targetAcc.mutable_data(),
              targetTime, moveMode, moveReference, targetAngle1, targetAngle2,
              blendingRadius, blendingType, orientation, appType);
        },
        py::arg("pos"), py::arg("vel"), py::arg("acc"), py::arg("time") = 0.f,
        py::arg("move_mode") = MOVE_MODE::MOVE_MODE_ABSOLUTE,
        py::arg("move_reference") = MOVE_REFERENCE::MOVE_REFERENCE_BASE,
        py::arg("target_angle1") = 0.f, py::arg("target_angle2") = 0.f,
        py::arg("blending_radius") = 0.f,
        py::arg("blending_type") =
            BLENDING_SPEED_TYPE::BLENDING_SPEED_TYPE_DUPLICATE,
        py::arg("orientation") = MOVE_ORIENTATION::DR_MV_ORI_TEACH,
        py::arg("app_type") = DR_MV_APP::DR_MV_APP_NONE);
}

// movesj functions
void bMovesj(Class& c){
    // -------------------------------------------------------------------------
    // movesj - scalar vel/acc overload
    // -------------------------------------------------------------------------
    c.def(
        "movesj",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, unsigned char nPosCount,
           float fTargetVel, float fTargetAcc, float fTargetTime,
           MOVE_MODE eMoveMode) {
          if (fTargetPos.ndim() != 2 ||
              fTargetPos.shape(0) != MAX_SPLINE_POINT ||
              fTargetPos.shape(1) != NUM_JOINT)
            throw std::runtime_error(
                "fTargetPos must have shape (MAX_SPLINE_POINT, NUM_JOINT)");
          float (*pTargetPos)[NUM_JOINT] =
              reinterpret_cast<float (*)[NUM_JOINT]>(
                  fTargetPos.mutable_data());
          return self.movesj(pTargetPos, nPosCount, fTargetVel, fTargetAcc,
                             fTargetTime, eMoveMode);
        },
        py::arg("fTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE);

    // -------------------------------------------------------------------------
    // movesj - array vel/acc overload
    // -------------------------------------------------------------------------
    c.def(
        "movesj",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, unsigned char nPosCount,
           arr_f fTargetVel, arr_f fTargetAcc, float fTargetTime,
           MOVE_MODE eMoveMode) {
          if (fTargetPos.ndim() != 2 ||
              fTargetPos.shape(0) != MAX_SPLINE_POINT ||
              fTargetPos.shape(1) != NUM_JOINT)
            throw std::runtime_error(
                "fTargetPos must have shape (MAX_SPLINE_POINT, NUM_JOINT)");
          if (fTargetVel.size() != NUMBER_OF_JOINT)
            throw std::runtime_error(
                "fTargetVel must have exactly NUMBER_OF_JOINT elements");
          if (fTargetAcc.size() != NUMBER_OF_JOINT)
            throw std::runtime_error(
                "fTargetAcc must have exactly NUMBER_OF_JOINT elements");
          float (*pTargetPos)[NUM_JOINT] =
              reinterpret_cast<float (*)[NUM_JOINT]>(
                  fTargetPos.mutable_data());
          return self.movesj(pTargetPos, nPosCount, fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime,
                             eMoveMode);
        },
        py::arg("fTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE);
}

void bMovesx(Class& c){
    // -------------------------------------------------------------------------
    // movesx
    // -------------------------------------------------------------------------
    c.def(
        "movesx",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, unsigned char nPosCount,
           arr_f fTargetVel, arr_f fTargetAcc, float fTargetTime,
           MOVE_MODE eMoveMode, MOVE_REFERENCE eMoveReference,
           SPLINE_VELOCITY_OPTION eVelOpt) {
          if (fTargetPos.ndim() != 2 ||
              fTargetPos.shape(0) != MAX_SPLINE_POINT ||
              fTargetPos.shape(1) != NUM_TASK)
            throw std::runtime_error(
                "fTargetPos must have shape (MAX_SPLINE_POINT, NUM_TASK)");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          float (*pTargetPos)[NUM_TASK] =
              reinterpret_cast<float (*)[NUM_TASK]>(
                  fTargetPos.mutable_data());
          return self.movesx(pTargetPos, nPosCount, fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime,
                             eMoveMode, eMoveReference, eVelOpt);
        },
        py::arg("fTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eMoveReference") = MOVE_REFERENCE_BASE,
        py::arg("eVelOpt") = SPLINE_VELOCITY_OPTION_DEFAULT);
}

void bMoveb(Class &c) {
    // -------------------------------------------------------------------------
    // moveb
    // -------------------------------------------------------------------------
    c.def(
        "moveb",
        [](DRAFramework::CDRFLEx &self, py::array_t<MOVE_POSB> tTargetPos,
           unsigned char nPosCount, arr_f fTargetVel, arr_f fTargetAcc,
           float fTargetTime, MOVE_MODE eMoveMode,
           MOVE_REFERENCE eMoveReference, DR_MV_APP eAppType) {
          if (tTargetPos.size() != MAX_MOVEB_POINT)
            throw std::runtime_error(
                "tTargetPos must have exactly MAX_MOVEB_POINT elements");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.moveb(tTargetPos.mutable_data(), nPosCount,
                            fTargetVel.mutable_data(),
                            fTargetAcc.mutable_data(), fTargetTime, eMoveMode,
                            eMoveReference, eAppType);
        },
        py::arg("tTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eMoveReference") = MOVE_REFERENCE_BASE,
        py::arg("eAppType") = DR_MV_APP_NONE);
}

void bMoveSpiral(Class& c){
    // -------------------------------------------------------------------------
    // move_spiral - radius/length overload
    // -------------------------------------------------------------------------
    c.def(
        "move_spiral",
        [](DRAFramework::CDRFLEx &self, TASK_AXIS eTaskAxis, float fRevolution,
           float fMaximuRadius, float fMaximumLength, arr_f fTargetVel,
           arr_f fTargetAcc, float fTargetTime,
           MOVE_REFERENCE eMoveReference) {
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.move_spiral(eTaskAxis, fRevolution, fMaximuRadius,
                                  fMaximumLength, fTargetVel.mutable_data(),
                                  fTargetAcc.mutable_data(), fTargetTime,
                                  eMoveReference);
        },
        py::arg("eTaskAxis"), py::arg("fRevolution"),
        py::arg("fMaximuRadius"), py::arg("fMaximumLength"),
        py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveReference") = MOVE_REFERENCE_TOOL);

    // -------------------------------------------------------------------------
    // move_spiral - target position overload
    // -------------------------------------------------------------------------
    c.def(
        "move_spiral",
        [](DRAFramework::CDRFLEx &self, TASK_AXIS eTaskAxis, float fRevolution,
           arr_f fTargetPos, arr_f fTargetVel, arr_f fTargetAcc,
           float fTargetTime, MOVE_REFERENCE eMoveReference,
           MOVE_MODE eMoveMode, SPIRAL_DIR eSpiralDir, ROT_DIR eRotDir) {
          if (fTargetPos.size() != 3)
            throw std::runtime_error(
                "fTargetPos must have exactly 3 elements");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.move_spiral(
              eTaskAxis, fRevolution, fTargetPos.mutable_data(),
              fTargetVel.mutable_data(), fTargetAcc.mutable_data(),
              fTargetTime, eMoveReference, eMoveMode, eSpiralDir, eRotDir);
        },
        py::arg("eTaskAxis"), py::arg("fRevolution"), py::arg("fTargetPos"),
        py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveReference") = MOVE_REFERENCE_TOOL,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eSpiralDir") = DR_SPIRAL_OUTWARD,
        py::arg("eRotDir") = DR_ROT_FORWARD);
}

void bMovePeriodic(Class& c){
    // -------------------------------------------------------------------------
    // move_periodic
    // -------------------------------------------------------------------------
    c.def(
        "move_periodic",
        [](DRAFramework::CDRFLEx &self, arr_f fAmplitude, arr_f fPeriodic, float fAccelTime,
           unsigned int nRepeat, MOVE_REFERENCE eMoveReference) {
          if (fAmplitude.size() != NUM_TASK)
            throw std::runtime_error(
                "fAmplitude must have exactly NUM_TASK elements");
          if (fPeriodic.size() != NUM_TASK)
            throw std::runtime_error(
                "fPeriodic must have exactly NUM_TASK elements");
          return self.move_periodic(fAmplitude.mutable_data(),
                                    fPeriodic.mutable_data(), fAccelTime,
                                    nRepeat, eMoveReference);
        },
        py::arg("fAmplitude"), py::arg("fPeriodic"), py::arg("fAccelTime"),
        py::arg("nRepeat"), py::arg("eMoveReference") = MOVE_REFERENCE_TOOL);
}

void bAmovej(Class& c) {
    // -------------------------------------------------------------------------
    // amovej
    // -------------------------------------------------------------------------
    c.def(
        "amovej",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, arr_f fTargetVel,
           arr_f fTargetAcc, float fTargetTime, MOVE_MODE eMoveMode,
           BLENDING_SPEED_TYPE eBlendingType) {
          if (fTargetPos.size() != NUM_JOINT)
            throw std::runtime_error(
                "fTargetPos must have exactly NUM_JOINT elements");
          if (fTargetVel.size() != NUM_JOINT)
            throw std::runtime_error(
                "fTargetVel must have exactly NUM_JOINT elements");
          if (fTargetAcc.size() != NUM_JOINT)
            throw std::runtime_error(
                "fTargetAcc must have exactly NUM_JOINT elements");
          return self.amovej(fTargetPos.mutable_data(),
                             fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime,
                             eMoveMode, eBlendingType);
        },
        py::arg("fTargetPos"), py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eBlendingType") = BLENDING_SPEED_TYPE_DUPLICATE);
}

void bAmovel(Class& c) {
    // -------------------------------------------------------------------------
    // amovel
    // -------------------------------------------------------------------------
    c.def(
        "amovel",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, arr_f fTargetVel,
           arr_f fTargetAcc, float fTargetTime, MOVE_MODE eMoveMode,
           MOVE_REFERENCE eMoveReference, BLENDING_SPEED_TYPE eBlendingType,
           DR_MV_APP eAppType) {
          if (fTargetPos.size() != NUM_TASK)
            throw std::runtime_error(
                "fTargetPos must have exactly NUM_TASK elements");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.amovel(
              fTargetPos.mutable_data(), fTargetVel.mutable_data(),
              fTargetAcc.mutable_data(), fTargetTime, eMoveMode,
              eMoveReference, eBlendingType, eAppType);
        },
        py::arg("fTargetPos"), py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eMoveReference") = MOVE_REFERENCE_BASE,
        py::arg("eBlendingType") = BLENDING_SPEED_TYPE_DUPLICATE,
        py::arg("eAppType") = DR_MV_APP_NONE);
}

void bAmovec(Class& c) {
    // -------------------------------------------------------------------------
    // amovec
    // -------------------------------------------------------------------------
    c.def(
        "amovec",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, arr_f fTargetVel,
           arr_f fTargetAcc, float fTargetTime, MOVE_MODE eMoveMode,
           MOVE_REFERENCE eMoveReference, float fTargetAngle1,
           float fTargetAngle2, BLENDING_SPEED_TYPE eBlendingType,
           MOVE_ORIENTATION eOrientation, DR_MV_APP eAppType) {
          if (fTargetPos.ndim() != 2 || fTargetPos.shape(0) != 2 ||
              fTargetPos.shape(1) != NUM_TASK)
            throw std::runtime_error(
                "fTargetPos must have shape (2, NUM_TASK)");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          float (*pTargetPos)[NUM_TASK] =
              reinterpret_cast<float (*)[NUM_TASK]>(
                  fTargetPos.mutable_data());
          return self.amovec(pTargetPos, fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime,
                             eMoveMode, eMoveReference, fTargetAngle1,
                             fTargetAngle2, eBlendingType, eOrientation,
                             eAppType);
        },
        py::arg("fTargetPos"), py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eMoveReference") = MOVE_REFERENCE_BASE,
        py::arg("fTargetAngle1") = 0.f, py::arg("fTargetAngle2") = 0.f,
        py::arg("eBlendingType") = BLENDING_SPEED_TYPE_DUPLICATE,
        py::arg("eOrientation") = DR_MV_ORI_TEACH,
        py::arg("eAppType") = DR_MV_APP_NONE);
}

void bAmovesj(Class& c) {
    // -------------------------------------------------------------------------
    // amovesj - scalar vel/acc overload
    // -------------------------------------------------------------------------
    c.def(
        "amovesj",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, unsigned char nPosCount,
           float fTargetVel, float fTargetAcc, float fTargetTime,
           MOVE_MODE eMoveMode) {
          if (fTargetPos.ndim() != 2 ||
              fTargetPos.shape(0) != MAX_SPLINE_POINT ||
              fTargetPos.shape(1) != NUM_JOINT)
            throw std::runtime_error(
                "fTargetPos must have shape (MAX_SPLINE_POINT, NUM_JOINT)");
          float (*pTargetPos)[NUM_JOINT] =
              reinterpret_cast<float (*)[NUM_JOINT]>(
                  fTargetPos.mutable_data());
          return self.amovesj(pTargetPos, nPosCount, fTargetVel, fTargetAcc,
                              fTargetTime, eMoveMode);
        },
        py::arg("fTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE);

    // -------------------------------------------------------------------------
    // amovesj - array vel/acc overload
    // -------------------------------------------------------------------------
    c.def(
        "amovesj",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, unsigned char nPosCount,
           arr_f fTargetVel, arr_f fTargetAcc, float fTargetTime,
           MOVE_MODE eMoveMode) {
          if (fTargetPos.ndim() != 2 ||
              fTargetPos.shape(0) != MAX_SPLINE_POINT ||
              fTargetPos.shape(1) != NUM_JOINT)
            throw std::runtime_error(
                "fTargetPos must have shape (MAX_SPLINE_POINT, NUM_JOINT)");
          if (fTargetVel.size() != NUMBER_OF_JOINT)
            throw std::runtime_error(
                "fTargetVel must have exactly NUMBER_OF_JOINT elements");
          if (fTargetAcc.size() != NUMBER_OF_JOINT)
            throw std::runtime_error(
                "fTargetAcc must have exactly NUMBER_OF_JOINT elements");
          float (*pTargetPos)[NUM_JOINT] =
              reinterpret_cast<float (*)[NUM_JOINT]>(
                  fTargetPos.mutable_data());
          return self.amovesj(
              pTargetPos, nPosCount, fTargetVel.mutable_data(),
              fTargetAcc.mutable_data(), fTargetTime, eMoveMode);
        },
        py::arg("fTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE);
}

void bAmovesx(Class& c) {
    // -------------------------------------------------------------------------
    // amovesx
    // -------------------------------------------------------------------------
    c.def(
        "amovesx",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, unsigned char nPosCount,
           arr_f fTargetVel, arr_f fTargetAcc, float fTargetTime,
           MOVE_MODE eMoveMode, MOVE_REFERENCE eMoveReference,
           SPLINE_VELOCITY_OPTION eVelOpt) {
          if (fTargetPos.ndim() != 2 ||
              fTargetPos.shape(0) != MAX_SPLINE_POINT ||
              fTargetPos.shape(1) != NUM_TASK)
            throw std::runtime_error(
                "fTargetPos must have shape (MAX_SPLINE_POINT, NUM_TASK)");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          float (*pTargetPos)[NUM_TASK] =
              reinterpret_cast<float (*)[NUM_TASK]>(
                  fTargetPos.mutable_data());
          return self.amovesx(pTargetPos, nPosCount,
                              fTargetVel.mutable_data(),
                              fTargetAcc.mutable_data(), fTargetTime,
                              eMoveMode, eMoveReference, eVelOpt);
        },
        py::arg("fTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eMoveReference") = MOVE_REFERENCE_BASE,
        py::arg("eVelOpt") = SPLINE_VELOCITY_OPTION_DEFAULT);
}

void bAmoveb(Class& c){
    // -------------------------------------------------------------------------
    // amoveb
    // -------------------------------------------------------------------------
    c.def(
        "amoveb",
        [](DRAFramework::CDRFLEx &self, py::array_t<MOVE_POSB> tTargetPos,
           unsigned char nPosCount, arr_f fTargetVel, arr_f fTargetAcc,
           float fTargetTime, MOVE_MODE eMoveMode,
           MOVE_REFERENCE eMoveReference, DR_MV_APP eAppType) {
          if (tTargetPos.size() != MAX_MOVEB_POINT)
            throw std::runtime_error(
                "tTargetPos must have exactly MAX_MOVEB_POINT elements");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.amoveb(tTargetPos.mutable_data(), nPosCount,
                             fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime,
                             eMoveMode, eMoveReference, eAppType);
        },
        py::arg("tTargetPos"), py::arg("nPosCount"), py::arg("fTargetVel"),
        py::arg("fTargetAcc"), py::arg("fTargetTime") = 0.f,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eMoveReference") = MOVE_REFERENCE_BASE,
        py::arg("eAppType") = DR_MV_APP_NONE);
}

void bAmoveSpiral(Class& c) {
    // -------------------------------------------------------------------------
    // amove_spiral - radius/length overload
    // -------------------------------------------------------------------------
    c.def(
        "amove_spiral",
        [](DRAFramework::CDRFLEx &self, TASK_AXIS eTaskAxis, float fRevolution,
           float fMaximuRadius, float fMaximumLength, arr_f fTargetVel,
           arr_f fTargetAcc, float fTargetTime,
           MOVE_REFERENCE eMoveReference) {
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.amove_spiral(eTaskAxis, fRevolution, fMaximuRadius,
                                   fMaximumLength, fTargetVel.mutable_data(),
                                   fTargetAcc.mutable_data(), fTargetTime,
                                   eMoveReference);
        },
        py::arg("eTaskAxis"), py::arg("fRevolution"),
        py::arg("fMaximuRadius"), py::arg("fMaximumLength"),
        py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveReference") = MOVE_REFERENCE_TOOL);

    // -------------------------------------------------------------------------
    // amove_spiral - target position overload
    // -------------------------------------------------------------------------
    c.def(
        "amove_spiral",
        [](DRAFramework::CDRFLEx &self, TASK_AXIS eTaskAxis, float fRevolution,
           arr_f fTargetPos, arr_f fTargetVel, arr_f fTargetAcc,
           float fTargetTime, MOVE_REFERENCE eMoveReference,
           MOVE_MODE eMoveMode, SPIRAL_DIR eSpiralDir, ROT_DIR eRotDir) {
          if (fTargetPos.size() != 3)
            throw std::runtime_error(
                "fTargetPos must have exactly 3 elements");
          if (fTargetVel.size() != 2)
            throw std::runtime_error(
                "fTargetVel must have exactly 2 elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.amove_spiral(
              eTaskAxis, fRevolution, fTargetPos.mutable_data(),
              fTargetVel.mutable_data(), fTargetAcc.mutable_data(),
              fTargetTime, eMoveReference, eMoveMode, eSpiralDir, eRotDir);
        },
        py::arg("eTaskAxis"), py::arg("fRevolution"), py::arg("fTargetPos"),
        py::arg("fTargetVel"), py::arg("fTargetAcc"),
        py::arg("fTargetTime") = 0.f,
        py::arg("eMoveReference") = MOVE_REFERENCE_TOOL,
        py::arg("eMoveMode") = MOVE_MODE_ABSOLUTE,
        py::arg("eSpiralDir") = DR_SPIRAL_OUTWARD,
        py::arg("eRotDir") = DR_ROT_FORWARD);
}

void bAmovePeriodic(Class& c){
    // -------------------------------------------------------------------------
    // amove_periodic
    // -------------------------------------------------------------------------
    c.def(
        "amove_periodic",
        [](DRAFramework::CDRFLEx &self, arr_f fAmplitude, arr_f fPeriodic, float fAccelTime,
           unsigned int nRepeat, MOVE_REFERENCE eMoveReference) {
          if (fAmplitude.size() != NUM_TASK)
            throw std::runtime_error(
                "fAmplitude must have exactly NUM_TASK elements");
          if (fPeriodic.size() != NUM_TASK)
            throw std::runtime_error(
                "fPeriodic must have exactly NUM_TASK elements");
          return self.amove_periodic(fAmplitude.mutable_data(),
                                     fPeriodic.mutable_data(), fAccelTime,
                                     nRepeat, eMoveReference);
        },
        py::arg("fAmplitude"), py::arg("fPeriodic"), py::arg("fAccelTime"),
        py::arg("nRepeat"), py::arg("eMoveReference") = MOVE_REFERENCE_TOOL);
}

void bStop(Class& c) {
    // -------------------------------------------------------------------------
    // stop / pause / resume / mwait  (no arrays)
    // -------------------------------------------------------------------------
    c.def(
        "stop",
        [](DRAFramework::CDRFLEx &self, STOP_TYPE eStopType) {
          return self.stop(eStopType);
        },
        py::arg("eStopType") = STOP_TYPE_QUICK)
    .def("move_pause", [](DRAFramework::CDRFLEx &self) { return self.move_pause(); })
    .def("move_resume", [](DRAFramework::CDRFLEx &self) { return self.move_resume(); })
    .def("mwait", [](DRAFramework::CDRFLEx &self) { return self.mwait(); });
}

void bTrans(Class& c) {
    // -------------------------------------------------------------------------
    // trans
    // -------------------------------------------------------------------------
    c.def(
        "trans",
        [](DRAFramework::CDRFLEx &self, arr_f fSourcePos, arr_f fOffset,
           COORDINATE_SYSTEM eSourceRef, COORDINATE_SYSTEM eTargetRef) {
          if (fSourcePos.size() != NUM_TASK)
            throw std::runtime_error(
                "fSourcePos must have exactly NUM_TASK elements");
          if (fOffset.size() != NUM_TASK)
            throw std::runtime_error(
                "fOffset must have exactly NUM_TASK elements");
          return self.trans(fSourcePos.mutable_data(), fOffset.mutable_data(),
                            eSourceRef, eTargetRef);
        },
        py::arg("fSourcePos"), py::arg("fOffset"),
        py::arg("eSourceRef") = COORDINATE_SYSTEM_BASE,
        py::arg("eTargetRef") = COORDINATE_SYSTEM_BASE);
}

void bFkin(Class& c) {
    // -------------------------------------------------------------------------
    // fkin
    // -------------------------------------------------------------------------
    c.def(
        "fkin",
        [](DRAFramework::CDRFLEx &self, arr_f fSourcePos, COORDINATE_SYSTEM eTargetRef) {
          if (fSourcePos.size() != NUM_JOINT)
            throw std::runtime_error(
                "fSourcePos must have exactly NUM_JOINT elements");
          return self.fkin(fSourcePos.mutable_data(), eTargetRef);
        },
        py::arg("fSourcePos"), py::arg("eTargetRef") = COORDINATE_SYSTEM_BASE);
}

void bIkine(Class& c) {
    // -------------------------------------------------------------------------
    // ikin - returns LPROBOT_POSE
    // -------------------------------------------------------------------------
    c.def(
        "ikin",
        [](DRAFramework::CDRFLEx &self, arr_f fSourcePos, unsigned char iSolutionSpace,
           COORDINATE_SYSTEM eTargetRef) {
          if (fSourcePos.size() != NUM_TASK)
            throw std::runtime_error(
                "fSourcePos must have exactly NUM_TASK elements");
          return self.ikin(fSourcePos.mutable_data(), iSolutionSpace,
                           eTargetRef);
        },
        py::arg("fSourcePos"), py::arg("iSolutionSpace"),
        py::arg("eTargetRef") = COORDINATE_SYSTEM_BASE);

    // -------------------------------------------------------------------------
    // ikin - returns LPINVERSE_KINEMATIC_RESPONSE (iRefPosOpt overload)
    // -------------------------------------------------------------------------
    c.def(
        "ikin",
        [](DRAFramework::CDRFLEx &self, arr_f fSourcePos, unsigned char iSolutionSpace,
           COORDINATE_SYSTEM eTargetRef, unsigned char iRefPosOpt) {
          if (fSourcePos.size() != NUM_TASK)
            throw std::runtime_error(
                "fSourcePos must have exactly NUM_TASK elements");
          return self.ikin(fSourcePos.mutable_data(), iSolutionSpace,
                           eTargetRef, iRefPosOpt);
        },
        py::arg("fSourcePos"), py::arg("iSolutionSpace"),
        py::arg("eTargetRef"), py::arg("iRefPosOpt"));

    // -------------------------------------------------------------------------
    // ikin - returns LPINVERSE_KINEMATIC_RESPONSE (fIterThreshold overload)
    // -------------------------------------------------------------------------
    c.def(
        "ikin",
        [](DRAFramework::CDRFLEx &self, arr_f fSourcePos, unsigned char iSolutionSpace,
           COORDINATE_SYSTEM eTargetRef, arr_f fIterThreshold) {
          if (fSourcePos.size() != NUM_TASK)
            throw std::runtime_error(
                "fSourcePos must have exactly NUM_TASK elements");
          if (fIterThreshold.size() != NUMBER_OF_ITER_THRESHOULD)
            throw std::runtime_error("fIterThreshold must have exactly "
                                     "NUMBER_OF_ITER_THRESHOULD elements");
          return self.ikin(fSourcePos.mutable_data(), iSolutionSpace,
                           eTargetRef, fIterThreshold.mutable_data());
        },
        py::arg("fSourcePos"), py::arg("iSolutionSpace"),
        py::arg("eTargetRef"), py::arg("fIterThreshold"));
}

void bSetRefCoord(Class& c){
    // -------------------------------------------------------------------------
    // set_ref_coord
    // -------------------------------------------------------------------------
    c.def(
        "set_ref_coord",
        [](DRAFramework::CDRFLEx &self, COORDINATE_SYSTEM eTargetCoordSystem) {
          return self.set_ref_coord(eTargetCoordSystem);
        },
        py::arg("eTargetCoordSystem"));
}

void bCheckMotion(Class& c){
    // -------------------------------------------------------------------------
    // check_motion
    // -------------------------------------------------------------------------
    c.def("check_motion", [](DRAFramework::CDRFLEx &self) { return self.check_motion(); });
}

void bEnableAlterMotion(Class& c){
    // -------------------------------------------------------------------------
    // enable_alter_motion
    // -------------------------------------------------------------------------
    c.def(
        "enable_alter_motion",
        [](DRAFramework::CDRFLEx &self, int iCycleTime, PATH_MODE ePathMode,
           COORDINATE_SYSTEM eTargetRef, arr_f fLimitDpos,
           arr_f fLimitDposPer) {
          if (fLimitDpos.size() != 2)
            throw std::runtime_error(
                "fLimitDpos must have exactly 2 elements");
          if (fLimitDposPer.size() != 2)
            throw std::runtime_error(
                "fLimitDposPer must have exactly 2 elements");
          return self.enable_alter_motion(iCycleTime, ePathMode, eTargetRef,
                                          fLimitDpos.mutable_data(),
                                          fLimitDposPer.mutable_data());
        },
        py::arg("iCycleTime"), py::arg("ePathMode"), py::arg("eTargetRef"),
        py::arg("fLimitDpos"), py::arg("fLimitDposPer"));
}

void bAlterMotion(Class& c) {
    // -------------------------------------------------------------------------
    // alter_motion
    // -------------------------------------------------------------------------
    c.def(
        "alter_motion",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos) {
          if (fTargetPos.size() != NUM_TASK)
            throw std::runtime_error(
                "fTargetPos must have exactly NUM_TASK elements");
          return self.alter_motion(fTargetPos.mutable_data());
        },
        py::arg("fTargetPos"));
}

void bDisableAlterMotion(Class& c) {
    // -------------------------------------------------------------------------
    // disable_alter_motion
    // -------------------------------------------------------------------------
    c.def("disable_alter_motion",
         [](DRAFramework::CDRFLEx &self) { return self.disable_alter_motion(); });
}

void bServoj(Class& c) {
    // -------------------------------------------------------------------------
    // servoj
    // -------------------------------------------------------------------------
    c.def(
        "servoj",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, arr_f fLimitVel, arr_f fLimitAcc,
           float fTargetTime, DR_SERVOJ_TYPE eTargetMod) {
          if (fTargetPos.size() != NUM_JOINT)
            throw std::runtime_error(
                "fTargetPos must have exactly NUM_JOINT elements");
          if (fLimitVel.size() != NUM_JOINT)
            throw std::runtime_error(
                "fLimitVel must have exactly NUM_JOINT elements");
          if (fLimitAcc.size() != NUM_JOINT)
            throw std::runtime_error(
                "fLimitAcc must have exactly NUM_JOINT elements");
          return self.servoj(
              fTargetPos.mutable_data(), fLimitVel.mutable_data(),
              fLimitAcc.mutable_data(), fTargetTime, eTargetMod);
        },
        py::arg("fTargetPos"), py::arg("fLimitVel"), py::arg("fLimitAcc"),
        py::arg("fTargetTime"), py::arg("eTargetMod") = DR_SERVO_OVERRIDE);
}

void bServol(Class& c) {
    // -------------------------------------------------------------------------
    // servol
    // -------------------------------------------------------------------------
    c.def(
        "servol",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetPos, arr_f fLimitVel, arr_f fLimitAcc,
           float fTargetTime) {
          if (fTargetPos.size() != NUM_TASK)
            throw std::runtime_error(
                "fTargetPos must have exactly NUM_TASK elements");
          if (fLimitVel.size() != 2)
            throw std::runtime_error(
                "fLimitVel must have exactly 2 elements");
          if (fLimitAcc.size() != 2)
            throw std::runtime_error(
                "fLimitAcc must have exactly 2 elements");
          return self.servol(fTargetPos.mutable_data(),
                             fLimitVel.mutable_data(),
                             fLimitAcc.mutable_data(), fTargetTime);
        },
        py::arg("fTargetPos"), py::arg("fLimitVel"), py::arg("fLimitAcc"),
        py::arg("fTargetTime"));
}

void bSpeedj(Class& c){
    // -------------------------------------------------------------------------
    // speedj
    // -------------------------------------------------------------------------
    c.def(
        "speedj",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetVel, arr_f fTargetAcc,
           float fTargetTime) {
          if (fTargetVel.size() != NUM_JOINT)
            throw std::runtime_error(
                "fTargetVel must have exactly NUM_JOINT elements");
          if (fTargetAcc.size() != NUM_JOINT)
            throw std::runtime_error(
                "fTargetAcc must have exactly NUM_JOINT elements");
          return self.speedj(fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime);
        },
        py::arg("fTargetVel"), py::arg("fTargetAcc"), py::arg("fTargetTime"));
}

void bSpeedl(Class& c) {
    // -------------------------------------------------------------------------
    // speedl
    // -------------------------------------------------------------------------
    c.def(
        "speedl",
        [](DRAFramework::CDRFLEx &self, arr_f fTargetVel, arr_f fTargetAcc,
           float fTargetTime) {
          if (fTargetVel.size() != NUM_TASK)
            throw std::runtime_error(
                "fTargetVel must have exactly NUM_TASK elements");
          if (fTargetAcc.size() != 2)
            throw std::runtime_error(
                "fTargetAcc must have exactly 2 elements");
          return self.speedl(fTargetVel.mutable_data(),
                             fTargetAcc.mutable_data(), fTargetTime);
        },
        py::arg("fTargetVel"), py::arg("fTargetAcc"), py::arg("fTargetTime"));
}

// Robot settings fuctions
// bool add_tool(string strSymbol, float fWeight, float fCog[3], float fInertia[NUM_TASK])
void bAddTool(Class& c) {
    c.def("add_tool",
        [](DRAFramework::CDRFLEx& self,
           const std::string& strSymbol,
           float fWeight,
           py::array_t<float, py::array::c_style | py::array::forcecast> fCog,
           py::array_t<float, py::array::c_style | py::array::forcecast> fInertia)
        {
            if (fCog.size() != 3)
                throw std::runtime_error("fCog must have exactly 3 elements");
            if (fInertia.size() != NUM_TASK)
                throw std::runtime_error("fInertia must have exactly NUM_TASK elements");
            return self.add_tool(strSymbol, fWeight,
                const_cast<float*>(fCog.data()),
                const_cast<float*>(fInertia.data()));
        },
        py::arg("strSymbol"), py::arg("fWeight"), py::arg("fCog"), py::arg("fInertia"));
}

// bool del_tool(string strSymbol)
void bDelTool(Class& c) {
    c.def("del_tool",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol) {
            return self.del_tool(strSymbol);
        },
        py::arg("strSymbol"));
}

// bool set_tool(string strSymbol)
void bSetTool(Class& c) {
    // -------------------------------------------------------------------------
    // check_motion
    // -------------------------------------------------------------------------
    c.def("check_motion", [](DRAFramework::CDRFLEx &self) { return self.check_motion(); });
}

// string get_tool()
void bGetTool(Class& c) {
    c.def("get_tool",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_tool();
        });
}

// bool add_tcp(string strSymbol, float fPosition[NUM_TASK])
void bAddTcp(Class& c) {
    c.def("add_tcp",
        [](DRAFramework::CDRFLEx& self,
           const std::string& strSymbol,
           py::array_t<float, py::array::c_style | py::array::forcecast> fPosition)
        {
            if (fPosition.size() != NUM_TASK)
                throw std::runtime_error("fPosition must have exactly NUM_TASK elements");
            return self.add_tcp(strSymbol, const_cast<float*>(fPosition.data()));
        },
        py::arg("strSymbol"), py::arg("fPosition"));
}

// bool del_tcp(string strSymbol)
void bDelTcp(Class& c) {
    c.def("del_tcp",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol) {
            return self.del_tcp(strSymbol);
        },
        py::arg("strSymbol"));
}

// bool set_tcp(string strSymbol)
void bSetTcp(Class& c) {
    c.def("set_tcp",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol) {
            return self.set_tcp(strSymbol);
        },
        py::arg("strSymbol"));
}

// string get_tcp()
void bGetTcp(Class& c) {
    c.def("get_tcp",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_tcp();
        });
}

// bool set_tool_shape(string strSymbol)
void bSetToolShape(Class& c) {
    c.def("set_tool_shape",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol) {
            return self.set_tool_shape(strSymbol);
        },
        py::arg("strSymbol"));
}

// float get_workpiece_weight()
void bGetWorkpieceWeight(Class& c) {
    c.def("get_workpiece_weight",
        [](DRAFramework::CDRFLEx& self) {
            return self.get_workpiece_weight();
        });
}

// bool reset_workpiece_weight()
void bResetWorkpieceWeight(Class& c) {
    c.def("reset_workpiece_weight",
        [](DRAFramework::CDRFLEx& self) {
            return self.reset_workpiece_weight();
        });
}

// bool set_singularity_handling(SINGULARITY_AVOIDANCE eMode)
void bSetSingularityHandling(Class& c) {
    c.def("set_singularity_handling",
        [](DRAFramework::CDRFLEx& self, SINGULARITY_AVOIDANCE eMode) {
            return self.set_singularity_handling(eMode);
        },
        py::arg("eMode"));
}

// bool setup_monitoring_version(int iVersion)
void bSetupMonitoringVersion(Class& c) {
    c.def("setup_monitoring_version",
        [](DRAFramework::CDRFLEx& self, int iVersion) {
            return self.setup_monitoring_version(iVersion);
        },
        py::arg("iVersion"));
}

// bool config_program_watch_variable(VARIABLE_TYPE eDivision, DATA_TYPE eType, string strName, string strData)
void bConfigProgramWatchVariable(Class& c) {
    c.def("config_program_watch_variable",
        [](DRAFramework::CDRFLEx& self,
           VARIABLE_TYPE eDivision,
           DATA_TYPE eType,
           const std::string& strName,
           const std::string& strData)
        {
            return self.config_program_watch_variable(eDivision, eType, strName, strData);
        },
        py::arg("eDivision"), py::arg("eType"), py::arg("strName"), py::arg("strData"));
}

// bool set_user_home()
void bSetUserHome(Class& c) {
    c.def("set_user_home",
        [](DRAFramework::CDRFLEx& self) {
            return self.set_user_home();
        });
}

// int servo_off(STOP_TYPE eStopType)
void bServoOff(Class& c) {
    c.def("servo_off",
        [](DRAFramework::CDRFLEx& self, STOP_TYPE eStopType) {
            return self.servo_off(eStopType);
        },
        py::arg("eStopType"));
}

// bool release_protective_stop(RELEASE_MODE eReleaseMode)
void bReleaseProtectiveStop(Class& c) {
    c.def("release_protective_stop",
        [](DRAFramework::CDRFLEx& self, RELEASE_MODE eReleaseMode) {
            return self.release_protective_stop(eReleaseMode);
        },
        py::arg("eReleaseMode"));
}

// bool change_collision_sensitivity(float fSensitivity)
void bChangeCollisionSensitivity(Class& c) {
    c.def("change_collision_sensitivity",
        [](DRAFramework::CDRFLEx& self, float fSensitivity) {
            return self.change_collision_sensitivity(fSensitivity);
        },
        py::arg("fSensitivity"));
}

// bool set_safety_mode(SAFETY_MODE eSafetyMode, SAFETY_MODE_EVENT eSafetyEvent)
void bSetSafetyMode(Class& c) {
    c.def("set_safety_mode",
        [](DRAFramework::CDRFLEx& self,
           SAFETY_MODE eSafetyMode,
           SAFETY_MODE_EVENT eSafetyEvent)
        {
            return self.set_safety_mode(eSafetyMode, eSafetyEvent);
        },
        py::arg("eSafetyMode"), py::arg("eSafetyEvent"));
}

// bool set_auto_servo_off(bool bFuncEnable, float fElapseTime)
void bSetAutoServoOff(Class& c) {
    c.def("set_auto_servo_off",
        [](DRAFramework::CDRFLEx& self, bool bFuncEnable, float fElapseTime) {
            return self.set_auto_servo_off(bFuncEnable, fElapseTime);
        },
        py::arg("bFuncEnable"), py::arg("fElapseTime"));
}

// bool set_workpiece_weight(float fWeight, float fCog[3], COG_REFERENCE eCogRef, ADD_UP eAddUp, float fStartTime, float fTransitionTime)
void bSetWorkpieceWeight(Class& c) {
    c.def("set_workpiece_weight",
        [](DRAFramework::CDRFLEx& self,
           float fWeight,
           std::optional<py::array_t<float, py::array::c_style | py::array::forcecast>> fCog,
           COG_REFERENCE eCogRef,
           ADD_UP eAddUp,
           float fStartTime,
           float fTransitionTime)
        {
            float* cogPtr = COG_DEFAULT;
            if (fCog.has_value()) {
                if (fCog->size() != 3)
                    throw std::runtime_error("fCog must have exactly 3 elements");
                cogPtr = const_cast<float*>(fCog->data());
            }
            return self.set_workpiece_weight(fWeight, cogPtr, eCogRef, eAddUp, fStartTime, fTransitionTime);
        },
        py::arg("fWeight")          = 0.0f,
        py::arg("fCog")             = py::none(),
        py::arg("eCogRef")          = COG_REFERENCE_TCP,
        py::arg("eAddUp")           = ADD_UP_REPLACE,
        py::arg("fStartTime")       = -10000.0f,
        py::arg("fTransitionTime")  = -10000.0f);
}


//Robot I/O functions
// bool set_tool_digital_output(GPIO_TOOL_DIGITAL_INDEX eGpioIndex, bool bOnOff)
void bSetToolDigitalOutput(Class& c) {
    c.def("set_tool_digital_output",
        [](DRAFramework::CDRFLEx& self, GPIO_TOOL_DIGITAL_INDEX eGpioIndex, bool bOnOff) {
            return self.set_tool_digital_output(eGpioIndex, bOnOff);
        },
        py::arg("eGpioIndex"), py::arg("bOnOff"));
}

// bool get_tool_digital_output(GPIO_TOOL_DIGITAL_INDEX eGpioIndex)
void bGetToolDigitalOutput(Class& c) {
    c.def("get_tool_digital_output",
        [](DRAFramework::CDRFLEx& self, GPIO_TOOL_DIGITAL_INDEX eGpioIndex) {
            return self.get_tool_digital_output(eGpioIndex);
        },
        py::arg("eGpioIndex"));
}

// bool get_tool_digital_input(GPIO_TOOL_DIGITAL_INDEX eGpioIndex)
void bGetToolDigitalInput(Class& c) {
    c.def("get_tool_digital_input",
        [](DRAFramework::CDRFLEx& self, GPIO_TOOL_DIGITAL_INDEX eGpioIndex) {
            return self.get_tool_digital_input(eGpioIndex);
        },
        py::arg("eGpioIndex"));
}

// bool set_digital_output(GPIO_CTRLBOX_DIGITAL_INDEX eGpioIndex, bool bOnOff)
void bSetDigitalOutput(Class& c) {
    c.def("set_digital_output",
        [](DRAFramework::CDRFLEx& self, GPIO_CTRLBOX_DIGITAL_INDEX eGpioIndex, bool bOnOff) {
            return self.set_digital_output(eGpioIndex, bOnOff);
        },
        py::arg("eGpioIndex"), py::arg("bOnOff"));
}

// bool get_digital_output(GPIO_CTRLBOX_DIGITAL_INDEX eGpioIndex)
void bGetDigitalOutput(Class& c) {
    c.def("get_digital_output",
        [](DRAFramework::CDRFLEx& self, GPIO_CTRLBOX_DIGITAL_INDEX eGpioIndex) {
            return self.get_digital_output(eGpioIndex);
        },
        py::arg("eGpioIndex"));
}

// bool get_digital_input(GPIO_CTRLBOX_DIGITAL_INDEX eGpioIndex)
void bGetDigitalInput(Class& c) {
    c.def("get_digital_input",
        [](DRAFramework::CDRFLEx& self, GPIO_CTRLBOX_DIGITAL_INDEX eGpioIndex) {
            return self.get_digital_input(eGpioIndex);
        },
        py::arg("eGpioIndex"));
}

// bool set_mode_analog_input(GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex, GPIO_ANALOG_TYPE eAnalogType)
void bSetModeAnalogInput(Class& c) {
    c.def("set_mode_analog_input",
        [](DRAFramework::CDRFLEx& self,
           GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex,
           GPIO_ANALOG_TYPE eAnalogType)
        {
            return self.set_mode_analog_input(eGpioIndex, eAnalogType);
        },
        py::arg("eGpioIndex"),
        py::arg("eAnalogType") = GPIO_ANALOG_TYPE_CURRENT);
}

// bool set_mode_analog_output(GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex, GPIO_ANALOG_TYPE eAnalogType)
void bSetModeAnalogOutput(Class& c) {
    c.def("set_mode_analog_output",
        [](DRAFramework::CDRFLEx& self,
           GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex,
           GPIO_ANALOG_TYPE eAnalogType)
        {
            return self.set_mode_analog_output(eGpioIndex, eAnalogType);
        },
        py::arg("eGpioIndex"),
        py::arg("eAnalogType") = GPIO_ANALOG_TYPE_CURRENT);
}

// bool set_analog_output(GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex, float fValue)
void bSetAnalogOutput(Class& c) {
    c.def("set_analog_output",
        [](DRAFramework::CDRFLEx& self, GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex, float fValue) {
            return self.set_analog_output(eGpioIndex, fValue);
        },
        py::arg("eGpioIndex"), py::arg("fValue"));
}

// float get_analog_input(GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex)
void bGetAnalogInput(Class& c) {
    c.def("get_analog_input",
        [](DRAFramework::CDRFLEx& self, GPIO_CTRLBOX_ANALOG_INDEX eGpioIndex) {
            return self.get_analog_input(eGpioIndex);
        },
        py::arg("eGpioIndex"));
}

// bool add_modbus_signal(string strSymbol, string strIpAddress, unsigned short nPort,
//                        MODBUS_REGISTER_TYPE eRegType, unsigned short iRegIndex,
//                        unsigned short nRegValue, unsigned char nSlaveId)
void bAddModbusSignal(Class& c) {
    c.def("add_modbus_signal",
        [](DRAFramework::CDRFLEx& self,
           const std::string& strSymbol,
           const std::string& strIpAddress,
           unsigned short nPort,
           MODBUS_REGISTER_TYPE eRegType,
           unsigned short iRegIndex,
           unsigned short nRegValue,
           unsigned char nSlaveId)
        {
            return self.add_modbus_signal(strSymbol, strIpAddress, nPort,
                eRegType, iRegIndex, nRegValue, nSlaveId);
        },
        py::arg("strSymbol"),
        py::arg("strIpAddress"),
        py::arg("nPort"),
        py::arg("eRegType"),
        py::arg("iRegIndex"),
        py::arg("nRegValue")  = static_cast<unsigned short>(0),
        py::arg("nSlaveId")   = static_cast<unsigned char>(255));
}

// bool del_modbus_signal(string strSymbol)
void bDelModbusSignal(Class& c) {
    c.def("del_modbus_signal",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol) {
            return self.del_modbus_signal(strSymbol);
        },
        py::arg("strSymbol"));
}

// bool set_modbus_output(string strSymbol, unsigned short nValue)
void bSetModbusOutput(Class& c) {
    c.def("set_modbus_output",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol, unsigned short nValue) {
            return self.set_modbus_output(strSymbol, nValue);
        },
        py::arg("strSymbol"), py::arg("nValue"));
}

// unsigned short get_modbus_input(string strSymbol)
void bGetModbusInput(Class& c) {
    c.def("get_modbus_input",
        [](DRAFramework::CDRFLEx& self, const std::string& strSymbol) {
            return self.get_modbus_input(strSymbol);
        },
        py::arg("strSymbol"));
}

// bool flange_serial_open(int nPort, int baudrate, BYTE_SIZE eByteSize,
//                         PARITY_CHECK eParity, STOP_BITS eStopBits)
void bFlangeSerialOpen(Class& c) {
    c.def("flange_serial_open",
        [](DRAFramework::CDRFLEx& self,
           int nPort,
           int baudrate,
           BYTE_SIZE eByteSize,
           PARITY_CHECK eParity,
           STOP_BITS eStopBits)
        {
            return self.flange_serial_open(nPort, baudrate, eByteSize, eParity, eStopBits);
        },
        py::arg("nPort"),
        py::arg("baudrate")  = 115200,
        py::arg("eByteSize") = BYTE_SIZE_EIGHTBITS,
        py::arg("eParity")   = PARITY_CHECK_NONE,
        py::arg("eStopBits") = STOPBITS_ONE);
}

// bool flange_serial_close(int nPort)
void bFlangeSerialClose(Class& c) {
    c.def("flange_serial_close",
        [](DRAFramework::CDRFLEx& self, int nPort) {
            return self.flange_serial_close(nPort);
        },
        py::arg("nPort"));
}

// bool flange_serial_write(int nSize, char* pSendData, int nPort)
// pSendData is accepted as a Python bytes object; size is checked against nSize.
void bFlangeSerialWrite(Class& c) {
    c.def("flange_serial_write",
        [](DRAFramework::CDRFLEx& self, py::bytes pSendData, int nPort) {
            std::string buf = pSendData;
            return self.flange_serial_write(
                static_cast<int>(buf.size()),
                const_cast<char*>(buf.data()),
                nPort);
        },
        py::arg("pSendData"),
        py::arg("nPort") = 1);
}

// float get_tool_analog_input(int nCh)
void bGetToolAnalogInput(Class& c) {
    c.def("get_tool_analog_input",
        [](DRAFramework::CDRFLEx& self, int nCh) {
            return self.get_tool_analog_input(nCh);
        },
        py::arg("nCh"));
}

// bool set_tool_digital_output_level(int nLv)
void bSetToolDigitalOutputLevel(Class& c) {
    c.def("set_tool_digital_output_level",
        [](DRAFramework::CDRFLEx& self, int nLv) {
            return self.set_tool_digital_output_level(nLv);
        },
        py::arg("nLv"));
}

// bool set_tool_digital_output_type(int nPort, OUTPUT_TYPE eOutputType)
void bSetToolDigitalOutputType(Class& c) {
    c.def("set_tool_digital_output_type",
        [](DRAFramework::CDRFLEx& self, int nPort, OUTPUT_TYPE eOutputType) {
            return self.set_tool_digital_output_type(nPort, eOutputType);
        },
        py::arg("nPort"), py::arg("eOutputType"));
}

// bool set_mode_tool_analog_input(int nCh, GPIO_ANALOG_TYPE eAnalogType)
void bSetModeToolAnalogInput(Class& c) {
    c.def("set_mode_tool_analog_input",
        [](DRAFramework::CDRFLEx& self, int nCh, GPIO_ANALOG_TYPE eAnalogType) {
            return self.set_mode_tool_analog_input(nCh, eAnalogType);
        },
        py::arg("nCh"), py::arg("eAnalogType"));
}


//Program control functions
// bool drl_start(ROBOT_SYSTEM eRobotSystem, string strDrlProgram)
void bDrlStart(Class& c) {
    c.def("drl_start",
        [](DRAFramework::CDRFLEx& self,
           ROBOT_SYSTEM eRobotSystem,
           const std::string& strDrlProgram)
        {
            return self.drl_start(eRobotSystem, strDrlProgram);
        },
        py::arg("eRobotSystem"), py::arg("strDrlProgram"));
}

// bool drl_stop(unsigned char eStopType)
void bDrlStop(Class& c) {
    c.def("drl_stop",
        [](DRAFramework::CDRFLEx& self, unsigned char eStopType) {
            return self.drl_stop(eStopType);
        },
        py::arg("eStopType") = static_cast<unsigned char>(0));
}

// bool drl_pause()
void bDrlPause(Class& c) {
    c.def("drl_pause",
        [](DRAFramework::CDRFLEx& self) {
            return self.drl_pause();
        });
}

// bool drl_resume()
void bDrlResume(Class& c) {
    c.def("drl_resume",
        [](DRAFramework::CDRFLEx& self) {
            return self.drl_resume();
        });
}

// bool change_operation_speed(float fSpeed)
void bChangeOperationSpeed(Class& c) {
    c.def("change_operation_speed",
        [](DRAFramework::CDRFLEx& self, float fSpeed) {
            return self.change_operation_speed(fSpeed);
        },
        py::arg("fSpeed"));
}

// bool save_sub_program(int iTargetType, string strFileName, string strDrlProgram)
void bSaveSubProgram(Class& c) {
    c.def("save_sub_program",
        [](DRAFramework::CDRFLEx& self,
           int iTargetType,
           const std::string& strFileName,
           const std::string& strDrlProgram)
        {
            return self.save_sub_program(iTargetType, strFileName, strDrlProgram);
        },
        py::arg("iTargetType"), py::arg("strFileName"), py::arg("strDrlProgram"));
}

// bool tp_popup_response(POPUP_RESPONSE eRes)
void bTpPopupResponse(Class& c) {
    c.def("tp_popup_response",
        [](DRAFramework::CDRFLEx& self, POPUP_RESPONSE eRes) {
            return self.tp_popup_response(eRes);
        },
        py::arg("eRes"));
}

// bool tp_get_user_input_response(string strUserInput)
void bTpGetUserInputResponse(Class& c) {
    c.def("tp_get_user_input_response",
        [](DRAFramework::CDRFLEx& self, const std::string& strUserInput) {
            return self.tp_get_user_input_response(strUserInput);
        },
        py::arg("strUserInput"));
}


// Miscellaneous functions
// Overload 1: bool parallel_axis(float fTargetPos1[NUM_TASK], float fTargetPos2[NUM_TASK],
//                                float fTargetPos3[NUM_TASK], TASK_AXIS eTaskAxis,
//                                COORDINATE_SYSTEM eSourceRef)
// Overload 2: bool parallel_axis(float fTargetVec[3], TASK_AXIS eTaskAxis,
//                                COORDINATE_SYSTEM eSourceRef)
void bParallelAxis(Class& c) {
    c.def("parallel_axis",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos1,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos2,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos3,
           TASK_AXIS eTaskAxis,
           COORDINATE_SYSTEM eSourceRef)
        {
            return self.parallel_axis(
                checkArray1D(fTargetPos1, NUM_TASK, "fTargetPos1"),
                checkArray1D(fTargetPos2, NUM_TASK, "fTargetPos2"),
                checkArray1D(fTargetPos3, NUM_TASK, "fTargetPos3"),
                eTaskAxis, eSourceRef);
        },
        py::arg("fTargetPos1"), py::arg("fTargetPos2"), py::arg("fTargetPos3"),
        py::arg("eTaskAxis"), py::arg("eSourceRef") = COORDINATE_SYSTEM_BASE);

    c.def("parallel_axis",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetVec,
           TASK_AXIS eTaskAxis,
           COORDINATE_SYSTEM eSourceRef)
        {
            return self.parallel_axis(
                checkArray1D(fTargetVec, 3, "fTargetVec"),
                eTaskAxis, eSourceRef);
        },
        py::arg("fTargetVec"), py::arg("eTaskAxis"), py::arg("eSourceRef"));
}

// Overload 1: bool align_axis(float fTargetPos1[NUM_TASK], float fTargetPos2[NUM_TASK],
//                             float fTargetPos3[NUM_TASK], float fSourceVec[3],
//                             TASK_AXIS eTaskAxis, COORDINATE_SYSTEM eSourceRef)
// Overload 2: bool align_axis(float fTargetVec[3], float fSourceVec[3],
//                             TASK_AXIS eTaskAxis, COORDINATE_SYSTEM eSourceRef)
void bAlignAxis(Class& c) {
    c.def("align_axis",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos1,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos2,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos3,
           py::array_t<float, py::array::c_style | py::array::forcecast> fSourceVec,
           TASK_AXIS eTaskAxis,
           COORDINATE_SYSTEM eSourceRef)
        {
            return self.align_axis(
                checkArray1D(fTargetPos1, NUM_TASK, "fTargetPos1"),
                checkArray1D(fTargetPos2, NUM_TASK, "fTargetPos2"),
                checkArray1D(fTargetPos3, NUM_TASK, "fTargetPos3"),
                checkArray1D(fSourceVec, 3, "fSourceVec"),
                eTaskAxis, eSourceRef);
        },
        py::arg("fTargetPos1"), py::arg("fTargetPos2"), py::arg("fTargetPos3"),
        py::arg("fSourceVec"), py::arg("eTaskAxis"),
        py::arg("eSourceRef") = COORDINATE_SYSTEM_BASE);

    c.def("align_axis",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetVec,
           py::array_t<float, py::array::c_style | py::array::forcecast> fSourceVec,
           TASK_AXIS eTaskAxis,
           COORDINATE_SYSTEM eSourceRef)
        {
            return self.align_axis(
                checkArray1D(fTargetVec, 3, "fTargetVec"),
                checkArray1D(fSourceVec, 3, "fSourceVec"),
                eTaskAxis, eSourceRef);
        },
        py::arg("fTargetVec"), py::arg("fSourceVec"),
        py::arg("eTaskAxis"), py::arg("eSourceRef"));
}

// bool is_done_bolt_tightening(FORCE_AXIS eForceAxis, float fTargetTor, float fTimeout)
void bIsDoneBoltTightening(Class& c) {
    c.def("is_done_bolt_tightening",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis, float fTargetTor, float fTimeout)
        {
            return self.is_done_bolt_tightening(eForceAxis, fTargetTor, fTimeout);
        },
        py::arg("eForceAxis"),
        py::arg("fTargetTor") = 0.f,
        py::arg("fTimeout")   = 0.f);
}

// bool task_compliance_ctrl(float fTargetStiffness[NUM_TASK],
//                           COORDINATE_SYSTEM eForceReference, float fTargetTime)
void bTaskComplianceCtrl(Class& c) {
    c.def("task_compliance_ctrl",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetStiffness,
           COORDINATE_SYSTEM eForceReference,
           float fTargetTime)
        {
            return self.task_compliance_ctrl(
                checkArray1D(fTargetStiffness, NUM_TASK, "fTargetStiffness"),
                eForceReference, fTargetTime);
        },
        py::arg("fTargetStiffness"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL,
        py::arg("fTargetTime")     = 0.f);
}

// bool release_compliance_ctrl()
void bReleaseComplianceCtrl(Class& c) {
    c.def("release_compliance_ctrl",
        [](DRAFramework::CDRFLEx& self) {
            return self.release_compliance_ctrl();
        });
}

// bool set_stiffnessx(float fTargetStiffness[NUM_TASK],
//                     COORDINATE_SYSTEM eForceReference, float fTargetTime)
void bSetStiffnessx(Class& c) {
    c.def("set_stiffnessx",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetStiffness,
           COORDINATE_SYSTEM eForceReference,
           float fTargetTime)
        {
            return self.set_stiffnessx(
                checkArray1D(fTargetStiffness, NUM_TASK, "fTargetStiffness"),
                eForceReference, fTargetTime);
        },
        py::arg("fTargetStiffness"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL,
        py::arg("fTargetTime")     = 0.f);
}

// LPROBOT_POSE calc_coord(unsigned short nCnt, unsigned short nInputMode,
//                         COORDINATE_SYSTEM eTargetRef,
//                         float fTargetPos1[NUM_TASK], float fTargetPos2[NUM_TASK],
//                         float fTargetPos3[NUM_TASK], float fTargetPos4[NUM_TASK])
void bCalcCoord(Class& c) {
    c.def("calc_coord",
        [](DRAFramework::CDRFLEx& self,
           unsigned short nCnt,
           unsigned short nInputMode,
           COORDINATE_SYSTEM eTargetRef,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos1,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos2,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos3,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos4)
        {
            return self.calc_coord(nCnt, nInputMode, eTargetRef,
                checkArray1D(fTargetPos1, NUM_TASK, "fTargetPos1"),
                checkArray1D(fTargetPos2, NUM_TASK, "fTargetPos2"),
                checkArray1D(fTargetPos3, NUM_TASK, "fTargetPos3"),
                checkArray1D(fTargetPos4, NUM_TASK, "fTargetPos4"));
        },
        py::arg("nCnt"), py::arg("nInputMode"), py::arg("eTargetRef"),
        py::arg("fTargetPos1"), py::arg("fTargetPos2"),
        py::arg("fTargetPos3"), py::arg("fTargetPos4"));
}

// Overload 1: int set_user_cart_coord(int iReqId, float fTargetPos[NUM_TASK],
//                                     COORDINATE_SYSTEM eTargetRef)
// Overload 2: int set_user_cart_coord(float fTargetPos[3][NUM_TASK], float fTargetOrg[3],
//                                     COORDINATE_SYSTEM fTargetRef)
// Overload 3: int set_user_cart_coord(float fTargetVec[2][3], float fTargetOrg[3],
//                                     COORDINATE_SYSTEM fTargetRef)
// Overloads 2 & 3 are disambiguated by shape of the first array argument.
void bSetUserCartCoord(Class& c) {
    c.def("set_user_cart_coord",
        [](DRAFramework::CDRFLEx& self,
           int iReqId,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           COORDINATE_SYSTEM eTargetRef)
        {
            return self.set_user_cart_coord(iReqId,
                checkArray1D(fTargetPos, NUM_TASK, "fTargetPos"),
                eTargetRef);
        },
        py::arg("iReqId"), py::arg("fTargetPos"),
        py::arg("eTargetRef") = COORDINATE_SYSTEM_BASE);

    c.def("set_user_cart_coord",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetOrg,
           COORDINATE_SYSTEM fTargetRef)
        {
            // Disambiguate by shape: [3][NUM_TASK] vs [2][3]
            if (fTargetPos.ndim() != 2)
                throw std::runtime_error("fTargetPos must be a 2D array");
            checkArray1D(fTargetOrg, 3, "fTargetOrg");

            if (fTargetPos.shape(0) == 3 && fTargetPos.shape(1) == NUM_TASK) {
                return self.set_user_cart_coord(
                    reinterpret_cast<float(*)[NUM_TASK]>(const_cast<float*>(fTargetPos.data())),
                    const_cast<float*>(fTargetOrg.data()), fTargetRef);
            } else if (fTargetPos.shape(0) == 2 && fTargetPos.shape(1) == 3) {
                return self.set_user_cart_coord(
                    reinterpret_cast<float(*)[3]>(const_cast<float*>(fTargetPos.data())),
                    const_cast<float*>(fTargetOrg.data()), fTargetRef);
            } else {
                throw std::runtime_error(
                    "fTargetPos must have shape (3, NUM_TASK) or (2, 3)");
            }
        },
        py::arg("fTargetPos"), py::arg("fTargetOrg"),
        py::arg("fTargetRef") = COORDINATE_SYSTEM_BASE);
}

// int overwrite_user_cart_coord(bool bTargetUpdate, int iReqId,
//                               float fTargetPos[NUM_TASK],
//                               COORDINATE_SYSTEM eTargetRef)
void bOverwriteUserCartCoord(Class& c) {
    c.def("overwrite_user_cart_coord",
        [](DRAFramework::CDRFLEx& self,
           bool bTargetUpdate,
           int iReqId,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           COORDINATE_SYSTEM eTargetRef)
        {
            return self.overwrite_user_cart_coord(bTargetUpdate, iReqId,
                checkArray1D(fTargetPos, NUM_TASK, "fTargetPos"),
                eTargetRef);
        },
        py::arg("bTargetUpdate"), py::arg("iReqId"), py::arg("fTargetPos"),
        py::arg("eTargetRef") = COORDINATE_SYSTEM_BASE);
}

// LPUSER_COORDINATE get_user_cart_coord(int iReqId)
void bGetUserCartCoord(Class& c) {
    c.def("get_user_cart_coord",
        [](DRAFramework::CDRFLEx& self, int iReqId) {
            return self.get_user_cart_coord(iReqId);
        },
        py::arg("iReqId"));
}

// bool set_desired_force(float fTargetForce[NUM_TASK],
//                        unsigned char iTargetDirection[NUM_TASK],
//                        COORDINATE_SYSTEM eForceReference, float fTargetTime,
//                        FORCE_MODE eForceMode)
void bSetDesiredForce(Class& c) {
    c.def("set_desired_force",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetForce,
           py::array_t<unsigned char, py::array::c_style | py::array::forcecast> iTargetDirection,
           COORDINATE_SYSTEM eForceReference,
           float fTargetTime,
           FORCE_MODE eForceMode)
        {
            if (fTargetForce.size() != NUM_TASK)
                throw std::runtime_error("fTargetForce must have exactly NUM_TASK elements");
            if (iTargetDirection.size() != NUM_TASK)
                throw std::runtime_error("iTargetDirection must have exactly NUM_TASK elements");
            return self.set_desired_force(
                const_cast<float*>(fTargetForce.data()),
                const_cast<unsigned char*>(iTargetDirection.data()),
                eForceReference, fTargetTime, eForceMode);
        },
        py::arg("fTargetForce"), py::arg("iTargetDirection"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL,
        py::arg("fTargetTime")     = 0.f,
        py::arg("eForceMode")      = FORCE_MODE_ABSOLUTE);
}

// bool release_force(float fTargetTime)
void bReleaseForce(Class& c) {
    c.def("release_force",
        [](DRAFramework::CDRFLEx& self, float fTargetTime) {
            return self.release_force(fTargetTime);
        },
        py::arg("fTargetTime") = 0.f);
}

// bool check_position_condition_abs(FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
//                                   COORDINATE_SYSTEM eForceReference)
void bCheckPositionConditionAbs(Class& c) {
    c.def("check_position_condition_abs",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
           COORDINATE_SYSTEM eForceReference)
        {
            return self.check_position_condition_abs(
                eForceAxis, fTargetMin, fTargetMax, eForceReference);
        },
        py::arg("eForceAxis"), py::arg("fTargetMin"), py::arg("fTargetMax"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL);
}

// bool check_position_condition_rel(FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
//                                   float fTargetPos[NUM_TASK],
//                                   COORDINATE_SYSTEM eForceReference)
void bCheckPositionConditionRel(Class& c) {
    c.def("check_position_condition_rel",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           COORDINATE_SYSTEM eForceReference)
        {
            return self.check_position_condition_rel(
                eForceAxis, fTargetMin, fTargetMax,
                checkArray1D(fTargetPos, NUM_TASK, "fTargetPos"),
                eForceReference);
        },
        py::arg("eForceAxis"), py::arg("fTargetMin"), py::arg("fTargetMax"),
        py::arg("fTargetPos"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL);
}

// bool check_position_condition(FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
//                               float fTargetPos[NUM_TASK], MOVE_MODE eMode,
//                               COORDINATE_SYSTEM eForceReference)
void bCheckPositionCondition(Class& c) {
    c.def("check_position_condition",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           MOVE_MODE eMode,
           COORDINATE_SYSTEM eForceReference)
        {
            return self.check_position_condition(
                eForceAxis, fTargetMin, fTargetMax,
                checkArray1D(fTargetPos, NUM_TASK, "fTargetPos"),
                eMode, eForceReference);
        },
        py::arg("eForceAxis"), py::arg("fTargetMin"), py::arg("fTargetMax"),
        py::arg("fTargetPos"),
        py::arg("eMode")           = MOVE_MODE_ABSOLUTE,
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL);
}

// bool check_force_condition(FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
//                            COORDINATE_SYSTEM eForceReference)
void bCheckForceCondition(Class& c) {
    c.def("check_force_condition",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis, float fTargetMin, float fTargetMax,
           COORDINATE_SYSTEM eForceReference)
        {
            return self.check_force_condition(
                eForceAxis, fTargetMin, fTargetMax, eForceReference);
        },
        py::arg("eForceAxis"), py::arg("fTargetMin"), py::arg("fTargetMax"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL);
}

// Overload 1: bool check_orientation_condition(FORCE_AXIS eForceAxis,
//                 float fTargetMin[NUM_TASK], float fTargetMax[NUM_TASK],
//                 COORDINATE_SYSTEM eForceReference)
// Overload 2: bool check_orientation_condition(FORCE_AXIS eForceAxis,
//                 float fTargetMin, float fTargetMax, float fTargetPos[NUM_TASK],
//                 COORDINATE_SYSTEM eForceReference)
void bCheckOrientationCondition(Class& c) {
    c.def("check_orientation_condition",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetMin,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetMax,
           COORDINATE_SYSTEM eForceReference)
        {
            return self.check_orientation_condition(eForceAxis,
                checkArray1D(fTargetMin, NUM_TASK, "fTargetMin"),
                checkArray1D(fTargetMax, NUM_TASK, "fTargetMax"),
                eForceReference);
        },
        py::arg("eForceAxis"), py::arg("fTargetMin"), py::arg("fTargetMax"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL);

    c.def("check_orientation_condition",
        [](DRAFramework::CDRFLEx& self,
           FORCE_AXIS eForceAxis,
           float fTargetMin,
           float fTargetMax,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           COORDINATE_SYSTEM eForceReference)
        {
            return self.check_orientation_condition(eForceAxis,
                fTargetMin, fTargetMax,
                checkArray1D(fTargetPos, NUM_TASK, "fTargetPos"),
                eForceReference);
        },
        py::arg("eForceAxis"), py::arg("fTargetMin"), py::arg("fTargetMax"),
        py::arg("fTargetPos"),
        py::arg("eForceReference") = COORDINATE_SYSTEM_TOOL);
}

// LPROBOT_POSE coord_transform(float fTargetPos[NUM_TASK],
//                              COORDINATE_SYSTEM eInCoordSystem,
//                              COORDINATE_SYSTEM eOutCoordSystem)
void bCoordTransform(Class& c) {
    c.def("coord_transform",
        [](DRAFramework::CDRFLEx& self,
           py::array_t<float, py::array::c_style | py::array::forcecast> fTargetPos,
           COORDINATE_SYSTEM eInCoordSystem,
           COORDINATE_SYSTEM eOutCoordSystem)
        {
            return self.coord_transform(
                checkArray1D(fTargetPos, NUM_TASK, "fTargetPos"),
                eInCoordSystem, eOutCoordSystem);
        },
        py::arg("fTargetPos"), py::arg("eInCoordSystem"), py::arg("eOutCoordSystem"));
}

// bool set_palletizing_mode(unsigned char iMode)
void bSetPalletizingMode(Class& c) {
    c.def("set_palletizing_mode",
        [](DRAFramework::CDRFLEx& self, unsigned char iMode) {
            return self.set_palletizing_mode(iMode);
        },
        py::arg("iMode"));
}

// LPMODBUS_DATA_LIST query_modbus_data_list()
void bQueryModbusDataList(Class& c) {
    c.def("query_modbus_data_list",
        [](DRAFramework::CDRFLEx& self) {
            return self.query_modbus_data_list();
        });
}
