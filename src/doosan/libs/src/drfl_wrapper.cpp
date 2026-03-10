#define DRCF_VERSION 2

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
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

    //bind GPIO controlbox digital enums
    py::enum_<GPIO_CTRLBOX_DIGITAL_INDEX>(m, "GPIO_CTRLBOX_DIGITAL_INDEX")
        .value("Gpio_ctrlbox_digital_index_1", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_1)
        .value("Gpio_ctrlbox_digital_index_2", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_2)
        .value("Gpio_ctrlbox_digital_index_3", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_3)
        .value("Gpio_ctrlbox_digital_index_4", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_4)
        .value("Gpio_ctrlbox_digital_index_5", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_5)
        .value("Gpio_ctrlbox_digital_index_6", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_6)
        .value("Gpio_ctrlbox_digital_index_7", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_7)
        .value("Gpio_ctrlbox_digital_index_8", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_8)
        .value("Gpio_ctrlbox_digital_index_9", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_9)
        .value("Gpio_ctrlbox_digital_index_10", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_10)
        .value("Gpio_ctrlbox_digital_index_11", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_11)
        .value("Gpio_ctrlbox_digital_index_12", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_12)
        .value("Gpio_ctrlbox_digital_index_13", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_13)
        .value("Gpio_ctrlbox_digital_index_14", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_14)
        .value("Gpio_ctrlbox_digital_index_15", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_15)
        .value("Gpio_ctrlbox_digital_index_16", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_16)
        .value("Gpio_ctrlbox_digital_index_17", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_17)
        .value("Gpio_ctrlbox_digital_index_18", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_18)
        .value("Gpio_ctrlbox_digital_index_19", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_19)
        .value("Gpio_ctrlbox_digital_index_20", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_20)
        .value("Gpio_ctrlbox_digital_index_21", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_21)
        .value("Gpio_ctrlbox_digital_index_22", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_22)
        .value("Gpio_ctrlbox_digital_index_23", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_23)
        .value("Gpio_ctrlbox_digital_index_24", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_24)
        .value("Gpio_ctrlbox_digital_index_25", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_25)
        .value("Gpio_ctrlbox_digital_index_26", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_26)
        .value("Gpio_ctrlbox_digital_index_27", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_27)
        .value("Gpio_ctrlbox_digital_index_28", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_28)
        .value("Gpio_ctrlbox_digital_index_29", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_29)
        .value("Gpio_ctrlbox_digital_index_30", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_30)
        .value("Gpio_ctrlbox_digital_index_31", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_31)
        .value("Gpio_ctrlbox_digital_index_32", GPIO_CTRLBOX_DIGITAL_INDEX::GPIO_CTRLBOX_DIGITAL_INDEX_32)
        .export_values();

    py::enum_<MANAGE_ACCESS_CONTROL>(m, "MANAGE_ACCESS_CONTROL")
        .value("Manage_access_control_force_request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_FORCE_REQUEST)
        .value("Manage_access_control_request", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_REQUEST)
        .value("Manage_access_control_response_yes", MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_YES)
        .value("Manage_access_control_response_no",
            MANAGE_ACCESS_CONTROL::MANAGE_ACCESS_CONTROL_RESPONSE_NO)
        .export_values();

    py::class_<LPSYSTEM_VERSION>(m, "LP_SYSTEM_VERSION")
        .def(py::init<LPSYSTEM_VERSION>());

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
             py::arg("pos"), py::arg("vel"), py::arg("acc"), py::arg("time"), py::arg("move_mode"), py::arg("r"), py::arg("blend_speed_type") = 0.0,
             "Joint space motion")

        /*
         * Basic I/O functions
         */
        .def("set_digital_output", &DRAFramework::CDRFLEx::set_digital_output,
            py::arg("GPIO_index"), py::arg("set"),
            "Set ctrlbox output status")
        .def("get_digital_output", &DRAFramework::CDRFLEx::get_digital_output,
            py::arg("GPIO_index"),
            "Get ctrlbox output status");

}

// extern "C" {
//     // 1. Create the Doosan Robot object
//     DRAFramework::CDRFLEx* DRFL_Create() {
//         return new DRAFramework::CDRFLEx();
//     }

//     // 2. Destroy the object to free memory
//     void DRFL_Destroy(DRAFramework::CDRFLEx* drfl) {
//         if (drfl) {
//             delete drfl;
//         }
//     }

//     // 3. Wrap a function, for example: connecting to the robot
//     bool DRFL_Connect(DRAFramework::CDRFLEx* drfl, const char* ip, int port) {
//         // Assuming connect_rt_control or setup is what you want to call
//         // Replace with the actual connection/setup method from the .h file
//         return drfl->open_connection(ip, port);
//     }

//     // 4. Wrap another function, for example: getting the robot state
//     int DRFL_GetRobotState(DRAFramework::CDRFLEx* drfl) {
//         return drfl->GetRobotState();
//     }

//         // Add any other functions from the .h file you want to use here...
//     }
