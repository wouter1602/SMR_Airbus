#include "./drfl_structs.hpp"

#include "../API-DRFL/include/DRFL.h"

void bind_drfl_structs(py::module_ &m) {
  // ─────────────────────────────────────────────────────────────────────────
  // SYSTEM_VERSION
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<SYSTEM_VERSION>(m, "SYSTEM_VERSION")
      .def(py::init<>()) STR_PROP(SYSTEM_VERSION, _szSmartTp)
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
      .def(py::init<>()) ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualPos, float)
          ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualAbs, float)
              ARRAY_PROP(ROBOT_MONITORING_JOINT, _fActualVel, float) ARRAY_PROP(
                  ROBOT_MONITORING_JOINT, _fActualErr, float)
                  ARRAY_PROP(ROBOT_MONITORING_JOINT, _fTargetPos, float)
                      ARRAY_PROP(ROBOT_MONITORING_JOINT, _fTargetVel, float);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_TASK
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_TASK>(m, "ROBOT_MONITORING_TASK")
      .def(py::init<>())
      .def_property(
          "_fActualPos",
          [](const ROBOT_MONITORING_TASK &s) {
            return array2d_copy(s._fActualPos);
          },
          [](ROBOT_MONITORING_TASK &s, py::array_t<float> v) {
            array2d_set(s._fActualPos, v);
          }) ARRAY_PROP(ROBOT_MONITORING_TASK, _fActualVel, float)
          ARRAY_PROP(ROBOT_MONITORING_TASK, _fActualErr, float)
              ARRAY_PROP(ROBOT_MONITORING_TASK, _fTargetPos, float)
                  ARRAY_PROP(ROBOT_MONITORING_TASK, _fTargetVel, float)
      .def_readwrite("_iSolutionSpace", &ROBOT_MONITORING_TASK::_iSolutionSpace)
      .def_property(
          "_fRotationMatrix",
          [](const ROBOT_MONITORING_TASK &s) {
            return array2d_copy(s._fRotationMatrix);
          },
          [](ROBOT_MONITORING_TASK &s, py::array_t<float> v) {
            array2d_set(s._fRotationMatrix, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_WORLD
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_WORLD>(m, "ROBOT_MONITORING_WORLD")
      .def(py::init<>()) ARRAY_PROP(ROBOT_MONITORING_WORLD, _fActualW2B, float)
      .def_property(
          "_fActualPos",
          [](const ROBOT_MONITORING_WORLD &s) {
            return array2d_copy(s._fActualPos);
          },
          [](ROBOT_MONITORING_WORLD &s, py::array_t<float> v) {
            array2d_set(s._fActualPos, v);
          }) ARRAY_PROP(ROBOT_MONITORING_WORLD, _fActualVel, float)
          ARRAY_PROP(ROBOT_MONITORING_WORLD, _fActualETT, float)
              ARRAY_PROP(ROBOT_MONITORING_WORLD, _fTargetPos, float)
                  ARRAY_PROP(ROBOT_MONITORING_WORLD, _fTargetVel, float)
      .def_property(
          "_fRotationMatrix",
          [](const ROBOT_MONITORING_WORLD &s) {
            return array2d_copy(s._fRotationMatrix);
          },
          [](ROBOT_MONITORING_WORLD &s, py::array_t<float> v) {
            array2d_set(s._fRotationMatrix, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_USER
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_USER>(m, "ROBOT_MONITORING_USER")
      .def(py::init<>())
      .def_readwrite("_iActualUCN", &ROBOT_MONITORING_USER::_iActualUCN)
      .def_readwrite("_iParent", &ROBOT_MONITORING_USER::_iParent)
      .def_property(
          "_fActualPos",
          [](const ROBOT_MONITORING_USER &s) {
            return array2d_copy(s._fActualPos);
          },
          [](ROBOT_MONITORING_USER &s, py::array_t<float> v) {
            array2d_set(s._fActualPos, v);
          }) ARRAY_PROP(ROBOT_MONITORING_USER, _fActualVel, float)
          ARRAY_PROP(ROBOT_MONITORING_USER, _fActualETT, float)
              ARRAY_PROP(ROBOT_MONITORING_USER, _fTargetPos, float)
                  ARRAY_PROP(ROBOT_MONITORING_USER, _fTargetVel, float)
      .def_property(
          "_fRotationMatrix",
          [](const ROBOT_MONITORING_USER &s) {
            return array2d_copy(s._fRotationMatrix);
          },
          [](ROBOT_MONITORING_USER &s, py::array_t<float> v) {
            array2d_set(s._fRotationMatrix, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_TORQUE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_TORQUE>(m, "ROBOT_MONITORING_TORQUE")
      .def(py::init<>())
          ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fDynamicTor, float)
              ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fActualJTS, float)
                  ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fActualEJT, float)
                      ARRAY_PROP(ROBOT_MONITORING_TORQUE, _fActualETT, float);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_STATE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_STATE>(m, "ROBOT_MONITORING_STATE")
      .def(py::init<>())
      .def_readwrite("_iActualMode", &ROBOT_MONITORING_STATE::_iActualMode)
      .def_readwrite("_iActualSpace", &ROBOT_MONITORING_STATE::_iActualSpace);

  // ─────────────────────────────────────────────────────────────────────────
  // MONITORING_CONTROL  (typedef of _ROBOT_MONITORING_DATA)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MONITORING_CONTROL>(m, "MONITORING_CONTROL")
      .def(py::init<>())
      .def_readwrite("_tState", &MONITORING_CONTROL::_tState)
      .def_readwrite("_tJoint", &MONITORING_CONTROL::_tJoint)
      .def_readwrite("_tTask", &MONITORING_CONTROL::_tTask)
      .def_readwrite("_tTorque", &MONITORING_CONTROL::_tTorque);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_DATA_EX  /  MONITORING_CONTROL_EX
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_DATA_EX>(m, "ROBOT_MONITORING_DATA_EX")
      .def(py::init<>())
      .def_readwrite("_tState", &ROBOT_MONITORING_DATA_EX::_tState)
      .def_readwrite("_tJoint", &ROBOT_MONITORING_DATA_EX::_tJoint)
      .def_readwrite("_tTask", &ROBOT_MONITORING_DATA_EX::_tTask)
      .def_readwrite("_tTorque", &ROBOT_MONITORING_DATA_EX::_tTorque)
      .def_readwrite("_tWorld", &ROBOT_MONITORING_DATA_EX::_tWorld)
      .def_readwrite("_tUser", &ROBOT_MONITORING_DATA_EX::_tUser);
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
      .def_readwrite("BoardNo", &FLANGE_VERSION::BoardNo)
      .def_readwrite("PacketType", &FLANGE_VERSION::PacketType)
      .def_readwrite("res", &FLANGE_VERSION::res)
      .def_readwrite("iFlangeHwVer", &FLANGE_VERSION::iFlangeHwVer);

  // ─────────────────────────────────────────────────────────────────────────
  // MONITORING_FLANGE_IO_CONFIG
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MONITORING_FLANGE_IO_CONFIG>(m, "MONITORING_FLANGE_IO_CONFIG")
      .def(py::init<>())
          ARRAY_PROP(MONITORING_FLANGE_IO_CONFIG, _iActualAI, float)
      .def_readwrite("_iX1Rs485FAIPinMux",
                     &MONITORING_FLANGE_IO_CONFIG::_iX1Rs485FAIPinMux)
      .def_readwrite("_iX2Rs485FAIPinMux",
                     &MONITORING_FLANGE_IO_CONFIG::_iX2Rs485FAIPinMux)
      .def_readwrite("_iX1DOBjtType",
                     &MONITORING_FLANGE_IO_CONFIG::_iX1DOBjtType)
      .def_readwrite("_iX2DOBjtType",
                     &MONITORING_FLANGE_IO_CONFIG::_iX2DOBjtType)
      .def_readwrite("_iVoutLevel", &MONITORING_FLANGE_IO_CONFIG::_iVoutLevel)
      .def_readwrite("_iFAI0Mode", &MONITORING_FLANGE_IO_CONFIG::_iFAI0Mode)
      .def_readwrite("_iFAI1Mode", &MONITORING_FLANGE_IO_CONFIG::_iFAI1Mode)
      .def_readwrite("_iFAI2Mode", &MONITORING_FLANGE_IO_CONFIG::_iFAI2Mode)
      .def_readwrite("_iFAI3Mode", &MONITORING_FLANGE_IO_CONFIG::_iFAI3Mode)
      .def_readwrite("_szX1DataLength",
                     &MONITORING_FLANGE_IO_CONFIG::_szX1DataLength)
      .def_readwrite("_szX1Parity", &MONITORING_FLANGE_IO_CONFIG::_szX1Parity)
      .def_readwrite("_szX1StopBit", &MONITORING_FLANGE_IO_CONFIG::_szX1StopBit)
      .def_readwrite("_szX2DataLength",
                     &MONITORING_FLANGE_IO_CONFIG::_szX2DataLength)
      .def_readwrite("_szX2Parity", &MONITORING_FLANGE_IO_CONFIG::_szX2Parity)
      .def_readwrite("_szX2StopBit", &MONITORING_FLANGE_IO_CONFIG::_szX2StopBit)
      .def_readwrite("_iServoSafetyMode",
                     &MONITORING_FLANGE_IO_CONFIG::_iServoSafetyMode)
      .def_readwrite("_iInterruptSafetyMode",
                     &MONITORING_FLANGE_IO_CONFIG::_iInterruptSafetyMode)
      .def_property(
          "_szX1Baudrate",
          [](const MONITORING_FLANGE_IO_CONFIG &s) {
            return array_copy(s._szX1Baudrate);
          },
          [](MONITORING_FLANGE_IO_CONFIG &s, py::array_t<unsigned char> v) {
            array_set(s._szX1Baudrate, v);
          })
      .def_property(
          "_szX2Baudrate",
          [](const MONITORING_FLANGE_IO_CONFIG &s) {
            return array_copy(s._szX2Baudrate);
          },
          [](MONITORING_FLANGE_IO_CONFIG &s, py::array_t<unsigned char> v) {
            array_set(s._szX2Baudrate, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_SENSOR
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_SENSOR>(m, "ROBOT_MONITORING_SENSOR")
      .def(py::init<>()) ARRAY_PROP(ROBOT_MONITORING_SENSOR, _fActualFTS, float)
          ARRAY_PROP(ROBOT_MONITORING_SENSOR, _fActualCS, float)
              ARRAY_PROP(ROBOT_MONITORING_SENSOR, _fActualACS, float);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_MONITORING_AMODEL  /  MONITORING_AMODEL
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_MONITORING_AMODEL>(m, "ROBOT_MONITORING_AMODEL")
      .def(py::init<>())
      .def_readwrite("_tSensor", &ROBOT_MONITORING_AMODEL::_tSensor)
      .def_readwrite("_fSingularity", &ROBOT_MONITORING_AMODEL::_fSingularity);
  m.attr("MONITORING_AMODEL") = m.attr("ROBOT_MONITORING_AMODEL");

  // ─────────────────────────────────────────────────────────────────────────
  // MONITORING_FORCECONTROL
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MONITORING_FORCECONTROL>(m, "MONITORING_FORCECONTROL")
      .def(py::init<>())
          ARRAY_PROP(MONITORING_FORCECONTROL, _iActualBS, unsigned char)
              ARRAY_PROP(MONITORING_FORCECONTROL, _fActualCS, float)
      .def_readwrite("_fSingularity", &MONITORING_FORCECONTROL::_fSingularity)
          ARRAY_PROP(MONITORING_FORCECONTROL, _fToolActualETT, float)
              ARRAY_PROP(MONITORING_FORCECONTROL, _iForceControlMode,
                         unsigned char)
      .def_readwrite("_iReferenceCoord",
                     &MONITORING_FORCECONTROL::_iReferenceCoord)
      .def_readwrite("_iAutoAccMode", &MONITORING_FORCECONTROL::_iAutoAccMode)
          ARRAY_PROP(MONITORING_FORCECONTROL, _fActualHDT, float)
      .def_readwrite("_iSingularHandlingMode",
                     &MONITORING_FORCECONTROL::_iSingularHandlingMode)
      .def_readwrite("_isMoving", &MONITORING_FORCECONTROL::_isMoving);

  // ─────────────────────────────────────────────────────────────────────────
  // MONITORING_DATA_EX  (union members exposed via named properties)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MONITORING_DATA_EX>(m, "MONITORING_DATA_EX")
      .def(py::init<>())
      .def_readwrite("_tCtrl", &MONITORING_DATA_EX::_tCtrl)
      .def_readwrite("_tMisc", &MONITORING_DATA_EX::_tMisc)
      // union _tMiscEx – expose named struct member
      .def_property(
          "_tCtrlEx",
          [](const MONITORING_DATA_EX &s) -> const MONITORING_FORCECONTROL & {
            return s._tMiscEx._tCtrlEx;
          },
          [](MONITORING_DATA_EX &s, const MONITORING_FORCECONTROL &v) {
            s._tMiscEx._tCtrlEx = v;
          })
      // union _tModel
      .def_property(
          "_tAModel",
          [](const MONITORING_DATA_EX &s) -> const MONITORING_AMODEL & {
            return s._tModel._tAModel;
          },
          [](MONITORING_DATA_EX &s, const MONITORING_AMODEL &v) {
            s._tModel._tAModel = v;
          })
      // union _tFlangeIo
      .def_property(
          "_tConfig",
          [](const MONITORING_DATA_EX &s)
              -> const MONITORING_FLANGE_IO_CONFIG & {
            return s._tFlangeIo._tConfig;
          },
          [](MONITORING_DATA_EX &s, const MONITORING_FLANGE_IO_CONFIG &v) {
            s._tFlangeIo._tConfig = v;
          });

  // ─────────────────────────────────────────────────────────────────────────
  // READ_CTRLIO_INPUT
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<READ_CTRLIO_INPUT>(m, "READ_CTRLIO_INPUT")
      .def(py::init<>()) ARRAY_PROP(READ_CTRLIO_INPUT, _iActualDI,
                                    unsigned char) ARRAY_PROP(READ_CTRLIO_INPUT,
                                                              _fActualAI, float)
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
      .def_readwrite("_tInput", &MONITORING_CTRLIO::_tInput)
      .def_readwrite("_tOutput", &MONITORING_CTRLIO::_tOutput);

  // ─────────────────────────────────────────────────────────────────────────
  // READ_CTRLIO_INPUT_EX
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<READ_CTRLIO_INPUT_EX>(m, "READ_CTRLIO_INPUT_EX")
      .def(py::init<>()) ARRAY_PROP(READ_CTRLIO_INPUT_EX, _iActualDI,
                                    unsigned char)
          ARRAY_PROP(READ_CTRLIO_INPUT_EX, _fActualAI, float) ARRAY_PROP(
              READ_CTRLIO_INPUT_EX, _iActualSW, unsigned char)
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
      .def_readwrite("_tInput", &MONITORING_CTRLIO_EX::_tInput)
      .def_readwrite("_tOutput", &MONITORING_CTRLIO_EX::_tOutput)
      .def_readwrite("_tEncoder", &MONITORING_CTRLIO_EX::_tEncoder);

  // ─────────────────────────────────────────────────────────────────────────
  // READ_CTRLIO_INPUT_EX2 / OUTPUT_EX2 / MONITORING_CTRLIO_EX2
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<READ_CTRLIO_INPUT_EX2>(m, "READ_CTRLIO_INPUT_EX2")
      .def(py::init<>()) ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualDI,
                                    unsigned char)
          ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _fActualAI, float) ARRAY_PROP(
              READ_CTRLIO_INPUT_EX2, _iActualSW, unsigned char)
              ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualSI, unsigned char)
                  ARRAY_PROP(READ_CTRLIO_INPUT_EX2, _iActualAT, unsigned char);

  py::class_<READ_CTRLIO_OUTPUT_EX2>(m, "READ_CTRLIO_OUTPUT_EX2")
      .def(py::init<>())
          ARRAY_PROP(READ_CTRLIO_OUTPUT_EX2, _iTargetDO, unsigned char)
              ARRAY_PROP(READ_CTRLIO_OUTPUT_EX2, _fTargetAO, float)
                  ARRAY_PROP(READ_CTRLIO_OUTPUT_EX2, _iTargetAT, unsigned char);

  py::class_<MONITORING_CTRLIO_EX2>(m, "MONITORING_CTRLIO_EX2")
      .def(py::init<>())
      .def_readwrite("_tInput", &MONITORING_CTRLIO_EX2::_tInput)
      .def_readwrite("_tOutput", &MONITORING_CTRLIO_EX2::_tOutput)
      .def_readwrite("_tEncoder", &MONITORING_CTRLIO_EX2::_tEncoder);

  // ─────────────────────────────────────────────────────────────────────────
  // MODBUS_REGISTER
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MODBUS_REGISTER>(m, "MODBUS_REGISTER")
      .def(py::init<>()) STR_PROP(MODBUS_REGISTER, _szSymbol)
      .def_readwrite("_iValue", &MODBUS_REGISTER::_iValue);

  // ─────────────────────────────────────────────────────────────────────────
  // MONITORING_MODBUS
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MONITORING_MODBUS>(m, "MONITORING_MODBUS")
      .def(py::init<>())
      .def_readwrite("_iRegCount", &MONITORING_MODBUS::_iRegCount)
      .def_property(
          "_tRegister",
          [](const MONITORING_MODBUS &s) {
            py::list lst;
            for (int i = 0; i < MAX_MODBUS_TOTAL_REGISTERS; ++i)
              lst.append(s._tRegister[i]);
            return lst;
          },
          [](MONITORING_MODBUS &s, py::list lst) {
            if ((int)lst.size() != MAX_MODBUS_TOTAL_REGISTERS)
              throw std::out_of_range(
                  "Expected " + std::to_string(MAX_MODBUS_TOTAL_REGISTERS) +
                  " elements");
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
      .def_property(
          "_szParam",
          [](const LOG_ALARM &s) {
            py::list lst;
            for (int i = 0; i < 3; ++i)
              lst.append(std::string(s._szParam[i]));
            return lst;
          },
          [](LOG_ALARM &s, py::list lst) {
            if (lst.size() != 3)
              throw std::out_of_range("Expected 3 strings");
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
      .def_readwrite("_iTotalCount", &MESSAGE_PROGRESS::_iTotalCount);

  // ─────────────────────────────────────────────────────────────────────────
  // MESSAGE_POPUP / MONITORING_POPUP
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MESSAGE_POPUP>(m, "MESSAGE_POPUP")
      .def(py::init<>()) STR_PROP(MESSAGE_POPUP, _szText)
      .def_readwrite("_iLevel", &MESSAGE_POPUP::_iLevel)
      .def_readwrite("_iBtnType", &MESSAGE_POPUP::_iBtnType);
  m.attr("MONITORING_POPUP") = m.attr("MESSAGE_POPUP");

  // ─────────────────────────────────────────────────────────────────────────
  // MESSAGE_INPUT / MONITORING_INPUT
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MESSAGE_INPUT>(m, "MESSAGE_INPUT")
      .def(py::init<>()) STR_PROP(MESSAGE_INPUT, _szText)
      .def_readwrite("_iType", &MESSAGE_INPUT::_iType);
  m.attr("MONITORING_INPUT") = m.attr("MESSAGE_INPUT");

  // ─────────────────────────────────────────────────────────────────────────
  // CONTROL_BRAKE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONTROL_BRAKE>(m, "CONTROL_BRAKE")
      .def(py::init<>())
      .def_readwrite("_iTargetAxs", &CONTROL_BRAKE::_iTargetAxs)
      .def_readwrite("_bValue", &CONTROL_BRAKE::_bValue);

  // ─────────────────────────────────────────────────────────────────────────
  // MOVE_POSB
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MOVE_POSB>(m, "MOVE_POSB")
      .def(py::init<>())
      .def_property(
          "_fTargetPos",
          [](const MOVE_POSB &s) { return array2d_copy(s._fTargetPos); },
          [](MOVE_POSB &s, py::array_t<float> v) {
            array2d_set(s._fTargetPos, v);
          })
      .def_readwrite("_iBlendType", &MOVE_POSB::_iBlendType)
      .def_readwrite("_fBlendRad", &MOVE_POSB::_fBlendRad);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_POSE / ROBOT_VEL / ROBOT_FORCE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_POSE>(m, "ROBOT_POSE")
      .def(py::init<>()) ARRAY_PROP(ROBOT_POSE, _fPosition, float);

  py::class_<ROBOT_VEL>(m, "ROBOT_VEL")
      .def(py::init<>()) ARRAY_PROP(ROBOT_VEL, _fVelocity, float);

  py::class_<ROBOT_FORCE>(m, "ROBOT_FORCE")
      .def(py::init<>()) ARRAY_PROP(ROBOT_FORCE, _fForce, float);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_TASK_POSE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_TASK_POSE>(m, "ROBOT_TASK_POSE")
      .def(py::init<>()) ARRAY_PROP(ROBOT_TASK_POSE, _fTargetPos, float)
      .def_readwrite("_iTargetSol", &ROBOT_TASK_POSE::_iTargetSol);

  // ─────────────────────────────────────────────────────────────────────────
  // USER_COORDINATE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<USER_COORDINATE>(m, "USER_COORDINATE")
      .def(py::init<>())
      .def_readwrite("_iReqId", &USER_COORDINATE::_iReqId)
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
      .def(py::init<>()) ARRAY_PROP(CONFIG_TCP, _fTargetPos, float);

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_TOOL
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_TOOL>(m, "CONFIG_TOOL")
      .def(py::init<>())
      .def_readwrite("_fWeight", &CONFIG_TOOL::_fWeight)
          ARRAY_PROP(CONFIG_TOOL, _fXYZ, float)
              ARRAY_PROP(CONFIG_TOOL, _fInertia, float);

  // ─────────────────────────────────────────────────────────────────────────
  // MEASURE_TCP_RESPONSE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MEASURE_TCP_RESPONSE>(m, "MEASURE_TCP_RESPONSE")
      .def(py::init<>())
      .def_readwrite("_tTCP", &MEASURE_TCP_RESPONSE::_tTCP)
      .def_readwrite("_fError", &MEASURE_TCP_RESPONSE::_fError);

  // ─────────────────────────────────────────────────────────────────────────
  // FLANGE_SERIAL_DATA (union exposed through named sub-struct properties)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<FLANGE_SERIAL_DATA>(m, "FLANGE_SERIAL_DATA")
      .def(py::init<>())
      .def_readwrite("_iCommand", &FLANGE_SERIAL_DATA::_iCommand)
      .def_property(
          "_szBaudrate",
          [](const FLANGE_SERIAL_DATA &s) {
            return array_copy(s._tData._tConfig._szBaudrate);
          },
          [](FLANGE_SERIAL_DATA &s, py::array_t<unsigned char> v) {
            array_set(s._tData._tConfig._szBaudrate, v);
          })
      .def_property(
          "_szDataLength",
          [](const FLANGE_SERIAL_DATA &s) {
            return s._tData._tConfig._szDataLength;
          },
          [](FLANGE_SERIAL_DATA &s, unsigned char v) {
            s._tData._tConfig._szDataLength = v;
          })
      .def_property(
          "_szParity",
          [](const FLANGE_SERIAL_DATA &s) {
            return s._tData._tConfig._szParity;
          },
          [](FLANGE_SERIAL_DATA &s, unsigned char v) {
            s._tData._tConfig._szParity = v;
          })
      .def_property(
          "_szStopBit",
          [](const FLANGE_SERIAL_DATA &s) {
            return s._tData._tConfig._szStopBit;
          },
          [](FLANGE_SERIAL_DATA &s, unsigned char v) {
            s._tData._tConfig._szStopBit = v;
          })
      .def_property(
          "_iLength",
          [](const FLANGE_SERIAL_DATA &s) { return s._tData._tValue._iLength; },
          [](FLANGE_SERIAL_DATA &s, unsigned short v) {
            s._tData._tValue._iLength = v;
          })
      .def_property(
          "_szValue",
          [](const FLANGE_SERIAL_DATA &s) {
            return std::string(
                reinterpret_cast<const char *>(s._tData._tValue._szValue));
          },
          [](FLANGE_SERIAL_DATA &s, const std::string &v) {
            std::strncpy(reinterpret_cast<char *>(s._tData._tValue._szValue),
                         v.c_str(), MAX_SYMBOL_SIZE - 1);
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
      .def_readwrite("_iSize", &FLANGE_SER_RXD_INFO_EX::_iSize)
          ARRAY_PROP(FLANGE_SER_RXD_INFO_EX, _cRxd, unsigned char)
      .def_readwrite("_portNum", &FLANGE_SER_RXD_INFO_EX::_portNum);
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
      .def_readwrite("_iExternalDI", &RT_INPUT_DATA_LIST::_iExternalDI)
      .def_readwrite("_iExternalDO", &RT_INPUT_DATA_LIST::_iExternalDO)
          ARRAY_PROP(RT_INPUT_DATA_LIST, _fExternalAnalogInput, float)
              ARRAY_PROP(RT_INPUT_DATA_LIST, _fExternalAnalogOutput, float);

  // ─────────────────────────────────────────────────────────────────────────
  // RT_OUTPUT_DATA_LIST  (the big real-time data struct)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<RT_OUTPUT_DATA_LIST>(m, "RT_OUTPUT_DATA_LIST")
      .def(py::init<>())
      .def_readwrite("time_stamp", &RT_OUTPUT_DATA_LIST::time_stamp) ARRAY_PROP(
          RT_OUTPUT_DATA_LIST, actual_joint_position,
          float) ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_position_abs,
                            float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                              actual_joint_velocity, float)
          ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_joint_velocity_abs,
                     float) ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_tcp_position,
                                       float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                         actual_tcp_velocity,
                                                         float)
              ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_flange_position,
                         float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                           actual_flange_velocity, float)
                  ARRAY_PROP(RT_OUTPUT_DATA_LIST, actual_motor_torque,
                             float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                               actual_joint_torque, float)
                      ARRAY_PROP(RT_OUTPUT_DATA_LIST, raw_joint_torque,
                                 float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                   raw_force_torque, float)
                          ARRAY_PROP(RT_OUTPUT_DATA_LIST, external_joint_torque,
                                     float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                       external_tcp_force,
                                                       float)
                              ARRAY_PROP(
                                  RT_OUTPUT_DATA_LIST, target_joint_position,
                                  float) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                    target_joint_velocity,
                                                    float)
                                  ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                             target_joint_acceleration, float)
                                      ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                 target_motor_torque, float)
                                          ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                     target_tcp_position, float)
                                              ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                         target_tcp_velocity,
                                                         float)
      .def_property(
          "jacobian_matrix",
          [](const RT_OUTPUT_DATA_LIST &s) {
            return array2d_copy(s.jacobian_matrix);
          },
          [](RT_OUTPUT_DATA_LIST &s, py::array_t<float> v) {
            array2d_set(s.jacobian_matrix, v);
          }) ARRAY_PROP(RT_OUTPUT_DATA_LIST, gravity_torque, float)
      .def_property(
          "coriolis_matrix",
          [](const RT_OUTPUT_DATA_LIST &s) {
            return array2d_copy(s.coriolis_matrix);
          },
          [](RT_OUTPUT_DATA_LIST &s, py::array_t<float> v) {
            array2d_set(s.coriolis_matrix, v);
          })
      .def_property(
          "mass_matrix",
          [](const RT_OUTPUT_DATA_LIST &s) {
            return array2d_copy(s.mass_matrix);
          },
          [](RT_OUTPUT_DATA_LIST &s, py::array_t<float> v) {
            array2d_set(s.mass_matrix, v);
          })
      .def_readwrite("solution_space", &RT_OUTPUT_DATA_LIST::solution_space)
      .def_readwrite("singularity", &RT_OUTPUT_DATA_LIST::singularity)
      .def_readwrite("operation_speed_rate",
                     &RT_OUTPUT_DATA_LIST::operation_speed_rate)
          ARRAY_PROP(RT_OUTPUT_DATA_LIST, joint_temperature, float)
      .def_readwrite("controller_digital_input",
                     &RT_OUTPUT_DATA_LIST::controller_digital_input)
      .def_readwrite("controller_digital_output",
                     &RT_OUTPUT_DATA_LIST::controller_digital_output)
          ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_input_type,
                     unsigned char)
              ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_input, float)
                  ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_output_type,
                             unsigned char)
                      ARRAY_PROP(RT_OUTPUT_DATA_LIST, controller_analog_output,
                                 float)
      .def_readwrite("flange_digital_input",
                     &RT_OUTPUT_DATA_LIST::flange_digital_input)
      .def_readwrite("flange_digital_output",
                     &RT_OUTPUT_DATA_LIST::flange_digital_output)
          ARRAY_PROP(RT_OUTPUT_DATA_LIST, flange_analog_input, float)
              ARRAY_PROP(RT_OUTPUT_DATA_LIST, external_encoder_strobe_count,
                         unsigned char) ARRAY_PROP(RT_OUTPUT_DATA_LIST,
                                                   external_encoder_count,
                                                   unsigned int)
                  ARRAY_PROP(RT_OUTPUT_DATA_LIST, goal_joint_position, float)
                      ARRAY_PROP(RT_OUTPUT_DATA_LIST, goal_tcp_position, float)
      .def_readwrite("robot_mode", &RT_OUTPUT_DATA_LIST::robot_mode)
      .def_readwrite("robot_state", &RT_OUTPUT_DATA_LIST::robot_state)
      .def_readwrite("control_mode", &RT_OUTPUT_DATA_LIST::control_mode);
  // ─────────────────────────────────────────────────────────────────────────
  // JOINT_RANGE / CONFIG_JOINT_RANGE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<JOINT_RANGE>(m, "JOINT_RANGE")
      .def(py::init<>()) ARRAY_PROP(JOINT_RANGE, _fMaxVelocity, float)
          ARRAY_PROP(JOINT_RANGE, _fMaxRange, float)
              ARRAY_PROP(JOINT_RANGE, _fMinRange, float);

  py::class_<CONFIG_JOINT_RANGE>(m, "CONFIG_JOINT_RANGE")
      .def(py::init<>())
      .def_readwrite("_Normal", &CONFIG_JOINT_RANGE::_Normal)
      .def_readwrite("_Reduced", &CONFIG_JOINT_RANGE::_Reduced);
  // ─────────────────────────────────────────────────────────────────────────
  // GENERAL_RANGE / CONFIG_GENERAL_RANGE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<GENERAL_RANGE>(m, "GENERAL_RANGE")
      .def(py::init<>())
      .def_readwrite("_fMaxForce", &GENERAL_RANGE::_fMaxForce)
      .def_readwrite("_fMaxPower", &GENERAL_RANGE::_fMaxPower)
      .def_readwrite("_fMaxSpeed", &GENERAL_RANGE::_fMaxSpeed)
      .def_readwrite("_fMaxMomentum", &GENERAL_RANGE::_fMaxMomentum);

  py::class_<CONFIG_GENERAL_RANGE>(m, "CONFIG_GENERAL_RANGE")
      .def(py::init<>())
      .def_readwrite("_Normal", &CONFIG_GENERAL_RANGE::_Normal)
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
      .def_readwrite("_tToPoint", &LINE::_tToPoint);
  m.attr("LINE_2D") = m.attr("LINE");

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_SAFETY_FUNCTION (union – expose raw byte array + helper)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_SAFETY_FUNCTION>(m, "CONFIG_SAFETY_FUNCTION")
      .def(py::init<>())
      .def_property(
          "_iStopCode",
          [](const CONFIG_SAFETY_FUNCTION &s) {
            return array_copy(s._iStopCode);
          },
          [](CONFIG_SAFETY_FUNCTION &s, py::array_t<unsigned char> v) {
            array_set(s._iStopCode, v);
          });

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
      .def_property(
          "_iIO",
          [](const CONFIG_SAFETY_IO &s) { return array2d_copy(s._iIO); },
          [](CONFIG_SAFETY_IO &s, py::array_t<unsigned char> v) {
            array2d_set(s._iIO, v);
          });

  py::class_<CONFIG_SAFETY_IO_EX>(m, "CONFIG_SAFETY_IO_EX")
      .def(py::init<>())
      .def_property(
          "_iIO",
          [](const CONFIG_SAFETY_IO_EX &s) { return array2d_copy(s._iIO); },
          [](CONFIG_SAFETY_IO_EX &s, py::array_t<unsigned char> v) {
            array2d_set(s._iIO, v);
          });

  py::class_<CONFIG_SAFETY_IO_OP>(m, "CONFIG_SAFETY_IO_OP")
      .def(py::init<>())
      .def_property(
          "_iIO",
          [](const CONFIG_SAFETY_IO_OP &s) { return array2d_copy(s._iIO); },
          [](CONFIG_SAFETY_IO_OP &s, py::array_t<unsigned char> v) {
            array2d_set(s._iIO, v);
          })
      .def_readwrite("_iTBI_Op", &CONFIG_SAFETY_IO_OP::_iTBI_Op)
      .def_readwrite("_iReserved", &CONFIG_SAFETY_IO_OP::_iReserved)
      .def_property(
          "_iIO_Op",
          [](const CONFIG_SAFETY_IO_OP &s) { return array2d_copy(s._iIO_Op); },
          [](CONFIG_SAFETY_IO_OP &s, py::array_t<unsigned char> v) {
            array2d_set(s._iIO_Op, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_VIRTUAL_FENCE  (union buffer exposed as bytes)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_VIRTUAL_FENCE>(m, "CONFIG_VIRTUAL_FENCE")
      .def(py::init<>())
      .def_readwrite("_iTargetRef", &CONFIG_VIRTUAL_FENCE::_iTargetRef)
      .def_readwrite("_iFenceType", &CONFIG_VIRTUAL_FENCE::_iFenceType)
      .def_property(
          "_iBuffer",
          [](const CONFIG_VIRTUAL_FENCE &s) {
            return array_copy(s._tFenceObject._iBuffer);
          },
          [](CONFIG_VIRTUAL_FENCE &s, py::array_t<unsigned char> v) {
            array_set(s._tFenceObject._iBuffer, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_SAFE_ZONE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_SAFE_ZONE>(m, "CONFIG_SAFE_ZONE")
      .def(py::init<>())
      .def_readwrite("_iTargetRef", &CONFIG_SAFE_ZONE::_iTargetRef)
      .def_property(
          "_tLine",
          [](const CONFIG_SAFE_ZONE &s) {
            py::list lst;
            for (int i = 0; i < 2; ++i)
              lst.append(s._tLine[i]);
            return lst;
          },
          [](CONFIG_SAFE_ZONE &s, py::list v) {
            if (v.size() != 2)
              throw std::out_of_range("Expected 2 LINE elements");
            for (int i = 0; i < 2; ++i)
              s._tLine[i] = v[i].cast<LINE>();
          })
      .def_property(
          "_tPoint",
          [](const CONFIG_SAFE_ZONE &s) {
            py::list lst;
            for (int i = 0; i < 3; ++i)
              lst.append(s._tPoint[i]);
            return lst;
          },
          [](CONFIG_SAFE_ZONE &s, py::list v) {
            if (v.size() != 3)
              throw std::out_of_range("Expected 3 POINT_2D elements");
            for (int i = 0; i < 3; ++i)
              s._tPoint[i] = v[i].cast<POINT_2D>();
          });

  // ─────────────────────────────────────────────────────────────────────────
  // ENABLE_SAFE_ZONE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ENABLE_SAFE_ZONE>(m, "ENABLE_SAFE_ZONE")
      .def(py::init<>()) ARRAY_PROP(ENABLE_SAFE_ZONE, _iRegion, unsigned char);

  // ─────────────────────────────────────────────────────────────────────────
  // SAFETY_OBJECT_SPHERE / CAPSULE / CUBE / OBB / POLYPRISM
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<SAFETY_OBJECT_SPHERE>(m, "SAFETY_OBJECT_SPHERE")
      .def(py::init<>())
      .def_readwrite("_fRadius", &SAFETY_OBJECT_SPHERE::_fRadius)
      .def_readwrite("_tTargetPos", &SAFETY_OBJECT_SPHERE::_tTargetPos);

  py::class_<SAFETY_OBJECT_CAPSULE>(m, "SAFETY_OBJECT_CAPSULE")
      .def(py::init<>())
      .def_readwrite("_fRadius", &SAFETY_OBJECT_CAPSULE::_fRadius)
      .def_property(
          "_tTargetPos",
          [](const SAFETY_OBJECT_CAPSULE &s) {
            py::list lst;
            for (int i = 0; i < 2; ++i)
              lst.append(s._tTargetPos[i]);
            return lst;
          },
          [](SAFETY_OBJECT_CAPSULE &s, py::list v) {
            if (v.size() != 2)
              throw std::out_of_range("Expected 2 POINT_3D elements");
            for (int i = 0; i < 2; ++i)
              s._tTargetPos[i] = v[i].cast<POINT_3D>();
          });

  py::class_<SAFETY_OBJECT_CUBE>(m, "SAFETY_OBJECT_CUBE")
      .def(py::init<>())
      .def_property(
          "_tTargetPos",
          [](const SAFETY_OBJECT_CUBE &s) {
            py::list lst;
            for (int i = 0; i < 2; ++i)
              lst.append(s._tTargetPos[i]);
            return lst;
          },
          [](SAFETY_OBJECT_CUBE &s, py::list v) {
            if (v.size() != 2)
              throw std::out_of_range("Expected 2 POINT_3D elements");
            for (int i = 0; i < 2; ++i)
              s._tTargetPos[i] = v[i].cast<POINT_3D>();
          });

  py::class_<SAFETY_OBJECT_OBB>(m, "SAFETY_OBJECT_OBB")
      .def(py::init<>())
      .def_property(
          "_tTargetPos",
          [](const SAFETY_OBJECT_OBB &s) {
            py::list lst;
            for (int i = 0; i < 4; ++i)
              lst.append(s._tTargetPos[i]);
            return lst;
          },
          [](SAFETY_OBJECT_OBB &s, py::list v) {
            if (v.size() != 4)
              throw std::out_of_range("Expected 4 POINT_3D elements");
            for (int i = 0; i < 4; ++i)
              s._tTargetPos[i] = v[i].cast<POINT_3D>();
          });

  py::class_<SAFETY_OBJECT_POLYPRISM>(m, "SAFETY_OBJECT_POLYPRISM")
      .def(py::init<>())
      .def_readwrite("_iPointCount", &SAFETY_OBJECT_POLYPRISM::_iPointCount)
      .def_property(
          "_tPoint",
          [](const SAFETY_OBJECT_POLYPRISM &s) {
            py::list lst;
            for (int i = 0; i < 10; ++i)
              lst.append(s._tPoint[i]);
            return lst;
          },
          [](SAFETY_OBJECT_POLYPRISM &s, py::list v) {
            if (v.size() != 10)
              throw std::out_of_range("Expected 10 POINT_2D elements");
            for (int i = 0; i < 10; ++i)
              s._tPoint[i] = v[i].cast<POINT_2D>();
          })
      .def_readwrite("_fZLoLimit", &SAFETY_OBJECT_POLYPRISM::_fZLoLimit)
      .def_readwrite("_fZUpLimit", &SAFETY_OBJECT_POLYPRISM::_fZUpLimit);

  // SAFETY_OBJECT_DATA union – expose raw buffer
  py::class_<SAFETY_OBJECT_DATA>(m, "SAFETY_OBJECT_DATA")
      .def(py::init<>())
      .def_property(
          "_iBuffer",
          [](const SAFETY_OBJECT_DATA &s) { return array_copy(s._iBuffer); },
          [](SAFETY_OBJECT_DATA &s, py::array_t<unsigned char> v) {
            array_set(s._iBuffer, v);
          });

  // SAFETY_OBJECT
  py::class_<SAFETY_OBJECT>(m, "SAFETY_OBJECT")
      .def(py::init<>())
      .def_readwrite("_iTargetRef", &SAFETY_OBJECT::_iTargetRef)
      .def_readwrite("_iObjectType", &SAFETY_OBJECT::_iObjectType)
      .def_readwrite("_tObject", &SAFETY_OBJECT::_tObject);

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_PROTECTED_ZONE / CONFIG_COLLISION_MUTE_ZONE_PROPERTY / _ZONE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_PROTECTED_ZONE>(m, "CONFIG_PROTECTED_ZONE")
      .def(py::init<>())
          ARRAY_PROP(CONFIG_PROTECTED_ZONE, _iValidity, unsigned char)
      .def_property(
          "_tZone",
          [](const CONFIG_PROTECTED_ZONE &s) {
            py::list lst;
            for (int i = 0; i < 10; ++i)
              lst.append(s._tZone[i]);
            return lst;
          },
          [](CONFIG_PROTECTED_ZONE &s, py::list v) {
            if (v.size() != 10)
              throw std::out_of_range("Expected 10 SAFETY_OBJECT elements");
            for (int i = 0; i < 10; ++i)
              s._tZone[i] = v[i].cast<SAFETY_OBJECT>();
          });

  py::class_<CONFIG_COLLISION_MUTE_ZONE_PROPERTY>(
      m, "CONFIG_COLLISION_MUTE_ZONE_PROPERTY")
      .def(py::init<>())
          STR_PROP(CONFIG_COLLISION_MUTE_ZONE_PROPERTY, _szIdentifier)
      .def_readwrite("_iOnOff", &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_iOnOff)
      .def_readwrite("_iSafetyIO",
                     &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_iSafetyIO)
      .def_readwrite("_fSensitivity",
                     &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_fSensitivity)
      .def_readwrite("_tZone", &CONFIG_COLLISION_MUTE_ZONE_PROPERTY::_tZone);

  py::class_<CONFIG_COLLISION_MUTE_ZONE>(m, "CONFIG_COLLISION_MUTE_ZONE")
      .def(py::init<>())
          ARRAY_PROP(CONFIG_COLLISION_MUTE_ZONE, _iValidity, unsigned char)
      .def_property(
          "_tProperty",
          [](const CONFIG_COLLISION_MUTE_ZONE &s) {
            py::list lst;
            for (int i = 0; i < 10; ++i)
              lst.append(s._tProperty[i]);
            return lst;
          },
          [](CONFIG_COLLISION_MUTE_ZONE &s, py::list v) {
            if (v.size() != 10)
              throw std::out_of_range("Expected 10 elements");
            for (int i = 0; i < 10; ++i)
              s._tProperty[i] =
                  v[i].cast<CONFIG_COLLISION_MUTE_ZONE_PROPERTY>();
          });

  // ─────────────────────────────────────────────────────────────────────────
  // SAFETY_TOOL_ORIENTATION_LIMIT / CONFIG_TOOL_ORIENTATION_LIMIT_ZONE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<SAFETY_TOOL_ORIENTATION_LIMIT>(m, "SAFETY_TOOL_ORIENTATION_LIMIT")
      .def(py::init<>())
      .def_readwrite("_tTargetDir", &SAFETY_TOOL_ORIENTATION_LIMIT::_tTargetDir)
      .def_readwrite("_fTargetAng",
                     &SAFETY_TOOL_ORIENTATION_LIMIT::_fTargetAng);

  py::class_<CONFIG_TOOL_ORIENTATION_LIMIT_ZONE>(
      m, "CONFIG_TOOL_ORIENTATION_LIMIT_ZONE")
      .def(py::init<>()) ARRAY_PROP(CONFIG_TOOL_ORIENTATION_LIMIT_ZONE,
                                    _iValidity, unsigned char)
      .def_property(
          "_tZone",
          [](const CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s) {
            py::list lst;
            for (int i = 0; i < 10; ++i)
              lst.append(s._tZone[i]);
            return lst;
          },
          [](CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s, py::list v) {
            if (v.size() != 10)
              throw std::out_of_range("Expected 10 SAFETY_OBJECT elements");
            for (int i = 0; i < 10; ++i)
              s._tZone[i] = v[i].cast<SAFETY_OBJECT>();
          })
      .def_property(
          "_tLimit",
          [](const CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s) {
            py::list lst;
            for (int i = 0; i < 10; ++i)
              lst.append(s._tLimit[i]);
            return lst;
          },
          [](CONFIG_TOOL_ORIENTATION_LIMIT_ZONE &s, py::list v) {
            if (v.size() != 10)
              throw std::out_of_range("Expected 10 limit elements");
            for (int i = 0; i < 10; ++i)
              s._tLimit[i] = v[i].cast<SAFETY_TOOL_ORIENTATION_LIMIT>();
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_NUDGE / CONFIG_COCKPIT_EX / CONFIG_IDLE_OFF
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_NUDGE>(m, "CONFIG_NUDGE")
      .def(py::init<>())
      .def_readwrite("_bEnable", &CONFIG_NUDGE::_bEnable)
      .def_readwrite("_fInputForce", &CONFIG_NUDGE::_fInputForce)
      .def_readwrite("_fDelayTime", &CONFIG_NUDGE::_fDelayTime);

  py::class_<CONFIG_COCKPIT_EX>(m, "CONFIG_COCKPIT_EX")
      .def(py::init<>())
      .def_readwrite("_bEnable", &CONFIG_COCKPIT_EX::_bEnable)
          ARRAY_PROP(CONFIG_COCKPIT_EX, _iButton, unsigned char)
      .def_readwrite("_bRecoveryTeach", &CONFIG_COCKPIT_EX::_bRecoveryTeach);

  py::class_<CONFIG_IDLE_OFF>(m, "CONFIG_IDLE_OFF")
      .def(py::init<>())
      .def_readwrite("_bFuncEnable", &CONFIG_IDLE_OFF::_bFuncEnable)
      .def_readwrite("_fElapseTime", &CONFIG_IDLE_OFF::_fElapseTime);

  // ─────────────────────────────────────────────────────────────────────────
  // WRITE_MODBUS_DATA / WRITE_MODBUS_RTU_DATA / MODBUS_DATA / MODBUS_DATA_LIST
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<WRITE_MODBUS_DATA>(m, "WRITE_MODBUS_DATA")
      .def(py::init<>()) STR_PROP(WRITE_MODBUS_DATA, _szSymbol)
          STR_PROP(WRITE_MODBUS_DATA, _szIpAddr)
      .def_readwrite("_iPort", &WRITE_MODBUS_DATA::_iPort)
      .def_readwrite("_iSlaveID", &WRITE_MODBUS_DATA::_iSlaveID)
      .def_readwrite("_iRegType", &WRITE_MODBUS_DATA::_iRegType)
      .def_readwrite("_iRegIndex", &WRITE_MODBUS_DATA::_iRegIndex)
      .def_readwrite("_iRegValue", &WRITE_MODBUS_DATA::_iRegValue);
  m.attr("WRITE_MODBUS_TCP_DATA") = m.attr("WRITE_MODBUS_DATA");

  py::class_<WRITE_MODBUS_RTU_DATA>(m, "WRITE_MODBUS_RTU_DATA")
      .def(py::init<>()) STR_PROP(WRITE_MODBUS_RTU_DATA, _szSymbol)
          STR_PROP(WRITE_MODBUS_RTU_DATA, _szttyPort)
      .def_readwrite("_iSlaveID", &WRITE_MODBUS_RTU_DATA::_iSlaveID)
      .def_readwrite("_iBaudRate", &WRITE_MODBUS_RTU_DATA::_iBaudRate)
      .def_readwrite("_iByteSize", &WRITE_MODBUS_RTU_DATA::_iByteSize)
      .def_readwrite("_szParity", &WRITE_MODBUS_RTU_DATA::_szParity)
      .def_readwrite("_iStopBit", &WRITE_MODBUS_RTU_DATA::_iStopBit)
      .def_readwrite("_iRegType", &WRITE_MODBUS_RTU_DATA::_iRegType)
      .def_readwrite("_iRegIndex", &WRITE_MODBUS_RTU_DATA::_iRegIndex)
      .def_readwrite("_iRegValue", &WRITE_MODBUS_RTU_DATA::_iRegValue);

  py::class_<MODBUS_DATA>(m, "MODBUS_DATA")
      .def(py::init<>())
      .def_readwrite("_iType", &MODBUS_DATA::_iType)
      .def_property(
          "_tcp",
          [](const MODBUS_DATA &s) -> const WRITE_MODBUS_TCP_DATA & {
            return s._tData._tcp;
          },
          [](MODBUS_DATA &s, const WRITE_MODBUS_TCP_DATA &v) {
            s._tData._tcp = v;
          })
      .def_property(
          "_rtu",
          [](const MODBUS_DATA &s) -> const WRITE_MODBUS_RTU_DATA & {
            return s._tData._rtu;
          },
          [](MODBUS_DATA &s, const WRITE_MODBUS_RTU_DATA &v) {
            s._tData._rtu = v;
          });

  py::class_<MODBUS_DATA_LIST>(m, "MODBUS_DATA_LIST")
      .def(py::init<>())
      .def_readwrite("_nCount", &MODBUS_DATA_LIST::_nCount)
      .def_property(
          "_tRegister",
          [](const MODBUS_DATA_LIST &s) {
            py::list lst;
            for (int i = 0; i < MAX_MODBUS_TOTAL_REGISTERS; ++i)
              lst.append(s._tRegister[i]);
            return lst;
          },
          [](MODBUS_DATA_LIST &s, py::list v) {
            if ((int)v.size() != MAX_MODBUS_TOTAL_REGISTERS)
              throw std::out_of_range(
                  "Expected " + std::to_string(MAX_MODBUS_TOTAL_REGISTERS) +
                  " elements");
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
      .def_property(
          "_iIO",
          [](const CONFIG_CONFIGURABLE_IO &s) { return array2d_copy(s._iIO); },
          [](CONFIG_CONFIGURABLE_IO &s, py::array_t<unsigned char> v) {
            array2d_set(s._iIO, v);
          });

  py::class_<CONFIG_CONFIGURABLE_IO_EX>(m, "CONFIG_CONFIGURABLE_IO_EX")
      .def(py::init<>())
      .def_property(
          "_iIO",
          [](const CONFIG_CONFIGURABLE_IO_EX &s) {
            return array2d_copy(s._iIO);
          },
          [](CONFIG_CONFIGURABLE_IO_EX &s, py::array_t<unsigned char> v) {
            array2d_set(s._iIO, v);
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_TOOL_SHAPE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_TOOL_SHAPE>(m, "CONFIG_TOOL_SHAPE")
      .def(py::init<>())
          ARRAY_PROP(CONFIG_TOOL_SHAPE, _iValidity, unsigned char)
      .def_property(
          "_tShape",
          [](const CONFIG_TOOL_SHAPE &s) {
            py::list lst;
            for (int i = 0; i < 5; ++i)
              lst.append(s._tShape[i]);
            return lst;
          },
          [](CONFIG_TOOL_SHAPE &s, py::list v) {
            if (v.size() != 5)
              throw std::out_of_range("Expected 5 SAFETY_OBJECT elements");
            for (int i = 0; i < 5; ++i)
              s._tShape[i] = v[i].cast<SAFETY_OBJECT>();
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_TOOL_SYMBOL / CONFIG_TCP_SYMBOL / CONFIG_TOOL_LIST / etc.
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_TOOL_SYMBOL>(m, "CONFIG_TOOL_SYMBOL")
      .def(py::init<>()) STR_PROP(CONFIG_TOOL_SYMBOL, _szSymbol)
      .def_readwrite("_tTool", &CONFIG_TOOL_SYMBOL::_tTool);

  py::class_<CONFIG_TCP_SYMBOL>(m, "CONFIG_TCP_SYMBOL")
      .def(py::init<>()) STR_PROP(CONFIG_TCP_SYMBOL, _szSymbol)
      .def_readwrite("_tTCP", &CONFIG_TCP_SYMBOL::_tTCP);

  py::class_<CONFIG_TOOL_SHAPE_SYMBOL>(m, "CONFIG_TOOL_SHAPE_SYMBOL")
      .def(py::init<>()) STR_PROP(CONFIG_TOOL_SHAPE_SYMBOL, _szSymbol)
      .def_readwrite("_tToolShape", &CONFIG_TOOL_SHAPE_SYMBOL::_tToolShape);
  // Helper lambda for fixed-size arrays of compound structs (list-of-structs)
  auto make_struct_list_prop = [](auto &cls, const char *name, auto getter,
                                  auto setter) {
    cls.def_property(name, getter, setter);
  };
  (void)make_struct_list_prop;

  py::class_<CONFIG_TOOL_LIST>(m, "CONFIG_TOOL_LIST")
      .def(py::init<>())
      .def_readwrite("_iToolCount", &CONFIG_TOOL_LIST::_iToolCount)
      .def_property(
          "_tTooList",
          [](const CONFIG_TOOL_LIST &s) {
            py::list lst;
            for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i)
              lst.append(s._tTooList[i]);
            return lst;
          },
          [](CONFIG_TOOL_LIST &s, py::list v) {
            if ((int)v.size() != MAX_CONFIG_TOOL_SIZE)
              throw std::out_of_range("Size mismatch");
            for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i)
              s._tTooList[i] = v[i].cast<CONFIG_TOOL_SYMBOL>();
          });

  py::class_<CONFIG_TCP_LIST>(m, "CONFIG_TCP_LIST")
      .def(py::init<>())
      .def_readwrite("_iToolCount", &CONFIG_TCP_LIST::_iToolCount)
      .def_property(
          "_tTooList",
          [](const CONFIG_TCP_LIST &s) {
            py::list lst;
            for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i)
              lst.append(s._tTooList[i]);
            return lst;
          },
          [](CONFIG_TCP_LIST &s, py::list v) {
            if ((int)v.size() != MAX_CONFIG_TCP_SIZE)
              throw std::out_of_range("Size mismatch");
            for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i)
              s._tTooList[i] = v[i].cast<CONFIG_TCP_SYMBOL>();
          });

  py::class_<CONFIG_TOOL_SHAPE_LIST>(m, "CONFIG_TOOL_SHAPE_LIST")
      .def(py::init<>())
      .def_readwrite("_iToolCount", &CONFIG_TOOL_SHAPE_LIST::_iToolCount)
      .def_property(
          "_tTooList",
          [](const CONFIG_TOOL_SHAPE_LIST &s) {
            py::list lst;
            for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i)
              lst.append(s._tTooList[i]);
            return lst;
          },
          [](CONFIG_TOOL_SHAPE_LIST &s, py::list v) {
            if ((int)v.size() != MAX_CONFIG_TOOL_SIZE)
              throw std::out_of_range("Size mismatch");
            for (int i = 0; i < MAX_CONFIG_TOOL_SIZE; ++i)
              s._tTooList[i] = v[i].cast<CONFIG_TOOL_SHAPE_SYMBOL>();
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_USER_COORDINATE_EX / CONFIG_PAYLOAD_EX
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_USER_COORDINATE_EX>(m, "CONFIG_USER_COORDINATE_EX")
      .def(py::init<>())
      .def_readwrite("_iTargetRef", &CONFIG_USER_COORDINATE_EX::_iTargetRef)
          ARRAY_PROP(CONFIG_USER_COORDINATE_EX, _fTargetPos, float)
      .def_readwrite("_iUserID", &CONFIG_USER_COORDINATE_EX::_iUserID);

  py::class_<CONFIG_PAYLOAD_EX>(m, "CONFIG_PAYLOAD_EX")
      .def(py::init<>())
      .def_readwrite("_fWeight", &CONFIG_PAYLOAD_EX::_fWeight)
          ARRAY_PROP(CONFIG_PAYLOAD_EX, _fXYZ, float)
      .def_readwrite("_iCogReference", &CONFIG_PAYLOAD_EX::_iCogReference)
      .def_readwrite("_iAddUp", &CONFIG_PAYLOAD_EX::_iAddUp)
      .def_readwrite("_fStartTime", &CONFIG_PAYLOAD_EX::_fStartTime)
      .def_readwrite("_fTransitionTime", &CONFIG_PAYLOAD_EX::_fTransitionTime);

  // ─────────────────────────────────────────────────────────────────────────
  // SYSTEM_TIME / SYSTEM_IPADDRESS / SYSTEM_POWER / SYSTEM_CPUUSAGE /
  // SYSTEM_DISKSIZE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<SYSTEM_TIME>(m, "SYSTEM_TIME")
      .def(py::init<>()) STR_PROP(SYSTEM_TIME, _szDate)
          STR_PROP(SYSTEM_TIME, _szTime);

  py::class_<SYSTEM_IPADDRESS>(m, "SYSTEM_IPADDRESS")
      .def(py::init<>())
      .def_readwrite("_iUsage", &SYSTEM_IPADDRESS::_iUsage)
      .def_readwrite("_iIpType", &SYSTEM_IPADDRESS::_iIpType)
          STR_PROP(SYSTEM_IPADDRESS, _szHotsIp)
              STR_PROP(SYSTEM_IPADDRESS, _szSubnet)
                  STR_PROP(SYSTEM_IPADDRESS, _szGateway)
      .def_property(
          "_szDNS",
          [](const SYSTEM_IPADDRESS &s) {
            py::list lst;
            for (int i = 0; i < 2; ++i)
              lst.append(std::string(s._szDNS[i]));
            return lst;
          },
          [](SYSTEM_IPADDRESS &s, py::list v) {
            if (v.size() != 2)
              throw std::out_of_range("Expected 2 DNS strings");
            for (int i = 0; i < 2; ++i) {
              std::string sv = v[i].cast<std::string>();
              std::strncpy(s._szDNS[i], sv.c_str(), 15);
              s._szDNS[i][15] = '\0';
            }
          });

  py::class_<SYSTEM_POWER>(m, "SYSTEM_POWER")
      .def(py::init<>())
      .def_readwrite("_iTarget", &SYSTEM_POWER::_iTarget)
      .def_readwrite("_iPower", &SYSTEM_POWER::_iPower);

  py::class_<SYSTEM_CPUUSAGE>(m, "SYSTEM_CPUUSAGE")
      .def(py::init<>())
      .def_readwrite("_iTotalUsage", &SYSTEM_CPUUSAGE::_iTotalUsage)
      .def_readwrite("_iProcessUsage", &SYSTEM_CPUUSAGE::_iProcessUsage);

  py::class_<SYSTEM_DISKSIZE>(m, "SYSTEM_DISKSIZE")
      .def(py::init<>())
      .def_readwrite("_iTotalDiskSize", &SYSTEM_DISKSIZE::_iTotalDiskSize)
      .def_readwrite("_iUsedDiskSize", &SYSTEM_DISKSIZE::_iUsedDiskSize);

  // ─────────────────────────────────────────────────────────────────────────
  // JTS_PARAM_DATA / FTS_PARAM_DATA  (aliased to CALIBRATE_*)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<JTS_PARAM_DATA>(m, "JTS_PARAM_DATA")
      .def(py::init<>()) ARRAY_PROP(JTS_PARAM_DATA, _fOffset, float)
          ARRAY_PROP(JTS_PARAM_DATA, _fScale, float);
  m.attr("CALIBRATE_JTS_RESPONSE") = m.attr("JTS_PARAM_DATA");
  py::class_<FTS_PARAM_DATA>(m, "FTS_PARAM_DATA")
      .def(py::init<>()) ARRAY_PROP(FTS_PARAM_DATA, _fOffset, float);
  m.attr("CALIBRATE_FTS_RESPONSE") = m.attr("FTS_PARAM_DATA");

  // ─────────────────────────────────────────────────────────────────────────
  // INSTALL_SUB_SYSTEM
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<INSTALL_SUB_SYSTEM>(m, "INSTALL_SUB_SYSTEM")
      .def(py::init<>())
      .def_readwrite("_bProcessButton", &INSTALL_SUB_SYSTEM::_bProcessButton)
      .def_readwrite("_bFTS", &INSTALL_SUB_SYSTEM::_bFTS)
      .def_readwrite("_bCockpit", &INSTALL_SUB_SYSTEM::_bCockpit);

  // ─────────────────────────────────────────────────────────────────────────
  // SYSTEM_UPDATE_RESPONSE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<SYSTEM_UPDATE_RESPONSE>(m, "SYSTEM_UPDATE_RESPONSE")
      .def(py::init<>())
      .def_readwrite("_iProcess", &SYSTEM_UPDATE_RESPONSE::_iProcess)
          ARRAY_PROP(SYSTEM_UPDATE_RESPONSE, _iInverter, unsigned char);

  // ─────────────────────────────────────────────────────────────────────────
  // KT_5G_CONFIG_PARAM
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<KT_5G_CONFIG_PARAM>(m, "KT_5G_CONFIG_PARAM")
      .def(py::init<>())
      .def_readwrite("_bEnable", &KT_5G_CONFIG_PARAM::_bEnable)
          STR_PROP(KT_5G_CONFIG_PARAM, _szIpAddress)
      .def_readwrite("_nPort", &KT_5G_CONFIG_PARAM::_nPort)
          STR_PROP(KT_5G_CONFIG_PARAM, _szDeviceId)
              STR_PROP(KT_5G_CONFIG_PARAM, _szDevicePw)
                  STR_PROP(KT_5G_CONFIG_PARAM, _szGatewayId)
      .def_readwrite("_fPeriod", &KT_5G_CONFIG_PARAM::_fPeriod);
  // ─────────────────────────────────────────────────────────────────────────
  // VECTOR3D / POSITION / NORMAL_VECTOR
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<VECTOR3D>(m, "VECTOR3D")
      .def(py::init<>()) ARRAY_PROP(VECTOR3D, _fTargetPos, float);

  py::class_<POSITION>(m, "POSITION")
      .def(py::init<>()) ARRAY_PROP(POSITION, _fTargetPos, float);
  m.attr("VECTOR6D") = m.attr("POSITION");
  m.attr("NORMAL_VECTOR_RESPONSE") = m.attr("VECTOR3D");
  m.attr("POSITION_ADDTO_RESPONSE") = m.attr("POSITION");

  py::class_<NORMAL_VECTOR>(m, "NORMAL_VECTOR")
      .def(py::init<>())
      .def_property(
          "_fTargetPos",
          [](const NORMAL_VECTOR &s) { return array2d_copy(s._fTargetPos); },
          [](NORMAL_VECTOR &s, py::array_t<float> v) {
            array2d_set(s._fTargetPos, v);
          });

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
      .def_readwrite("_iCount", &WRITE_SERIAL_BURST::_iCount)
      .def_property(
          "_tPort",
          [](const WRITE_SERIAL_BURST &s) {
            py::list lst;
            for (int i = 0; i < MAX_DIGITAL_BURST_SIZE; ++i)
              lst.append(s._tPort[i]);
            return lst;
          },
          [](WRITE_SERIAL_BURST &s, py::list v) {
            if ((int)v.size() != MAX_DIGITAL_BURST_SIZE)
              throw std::out_of_range("Size mismatch");
            for (int i = 0; i < MAX_DIGITAL_BURST_SIZE; ++i)
              s._tPort[i] = v[i].cast<GPIO_PORT>();
          });
  m.attr("GPIO_SETOUTPUT_BURST") = m.attr("WRITE_SERIAL_BURST");

  // ─────────────────────────────────────────────────────────────────────────
  // MODBUS_REGISTER_MONITORING / WRITE_MODBUS_BURST / MODBUS_MULTI_REGISTER
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<MODBUS_REGISTER_MONITORING>(m, "MODBUS_REGISTER_MONITORING")
      .def(py::init<>()) STR_PROP(MODBUS_REGISTER_MONITORING, _szSymbol)
      .def_readwrite("_iRegValue", &MODBUS_REGISTER_MONITORING::_iRegValue);

  py::class_<WRITE_MODBUS_BURST>(m, "WRITE_MODBUS_BURST")
      .def(py::init<>())
      .def_readwrite("_iCount", &WRITE_MODBUS_BURST::_iCount)
      .def_property(
          "_tRegister",
          [](const WRITE_MODBUS_BURST &s) {
            py::list lst;
            for (int i = 0; i < MAX_MODBUS_BURST_SIZE; ++i)
              lst.append(s._tRegister[i]);
            return lst;
          },
          [](WRITE_MODBUS_BURST &s, py::list v) {
            if ((int)v.size() != MAX_MODBUS_BURST_SIZE)
              throw std::out_of_range("Size mismatch");
            for (int i = 0; i < MAX_MODBUS_BURST_SIZE; ++i)
              s._tRegister[i] = v[i].cast<MODBUS_REGISTER_MONITORING>();
          });
  m.attr("MODBUS_REGISTER_BURST") = m.attr("WRITE_MODBUS_BURST");

  py::class_<MODBUS_MULTI_REGISTER>(m, "MODBUS_MULTI_REGISTER")
      .def(py::init<>()) STR_PROP(MODBUS_MULTI_REGISTER, _szSymbol)
      .def_readwrite("_iRegCount", &MODBUS_MULTI_REGISTER::_iRegCount)
          ARRAY_PROP(MODBUS_MULTI_REGISTER, _iRegValue, unsigned short)
      .def_readwrite("_iRegIndex", &MODBUS_MULTI_REGISTER::_iRegIndex)
      .def_readwrite("_iSlaveID", &MODBUS_MULTI_REGISTER::_iSlaveID);

  py::class_<UPDATE_MODBUS_MULTI_REGISTER>(m, "UPDATE_MODBUS_MULTI_REGISTER")
      .def(py::init<>()) STR_PROP(UPDATE_MODBUS_MULTI_REGISTER, _szSymbol)
      .def_readwrite("_iRegCount", &UPDATE_MODBUS_MULTI_REGISTER::_iRegCount)
          ARRAY_PROP(UPDATE_MODBUS_MULTI_REGISTER, _iRegValue, unsigned short);
  // ─────────────────────────────────────────────────────────────────────────
  // LOCAL / SAFETY_ZONE_PROPERTY structs (compact form)
  // ─────────────────────────────────────────────────────────────────────────

  py::class_<LOCAL_ZONE_PROPERTY_JOINT_RANGE>(m,
                                              "LOCAL_ZONE_PROPERTY_JOINT_RANGE")
      .def(py::init<>()) ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_RANGE, _iOverride,
                                    unsigned char)
          ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_RANGE, _fMinRange, float)
              ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_RANGE, _fMaxRange, float);

  py::class_<LOCAL_ZONE_PROPERTY_JOINT_SPEED>(m,
                                              "LOCAL_ZONE_PROPERTY_JOINT_SPEED")
      .def(py::init<>())
          ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_SPEED, _iOverride, unsigned char)
              ARRAY_PROP(LOCAL_ZONE_PROPERTY_JOINT_SPEED, _fSpeed, float);

  py::class_<LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE>(
      m, "LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE")
      .def(py::init<>())
      .def_readwrite("_iOverride",
                     &LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE::_iOverride)
      .def_readwrite("_iStopMode",
                     &LOCAL_ZONE_PROPERTY_COLLISION_STOPMODE::_iStopMode);

  py::class_<LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE>(
      m, "LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE")
      .def(py::init<>())
      .def_readwrite("_iOverride",
                     &LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE::_iOverride)
      .def_readwrite("_iStopMode",
                     &LOCAL_ZONE_PROPERTY_TCPSLF_STOPMODE::_iStopMode);

  py::class_<LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION>(
      m, "LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION")
      .def(py::init<>())
      .def_readwrite("_iOverride",
                     &LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION::_iOverride)
          ARRAY_PROP(LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION, _fDirection, float)
      .def_readwrite("_fAngle", &LOCAL_ZONE_PROPERTY_TOOL_ORIENTATION::_fAngle);

  py::class_<SAFETY_ZONE_PROPERTY_SPACE_LIMIT>(
      m, "SAFETY_ZONE_PROPERTY_SPACE_LIMIT")
      .def(py::init<>())
      .def_readwrite("_iInspectionType",
                     &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_iInspectionType)
      .def_readwrite("_tJointRangeOverride",
                     &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_tJointRangeOverride)
      .def_readwrite("_iDynamicZoneEnable",
                     &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_iDynamicZoneEnable)
      .def_readwrite("_iInsideZoneDectection",
                     &SAFETY_ZONE_PROPERTY_SPACE_LIMIT::_iInsideZoneDectection);

  py::class_<SAFETY_ZONE_PROPERTY_LOCAL_ZONE>(m,
                                              "SAFETY_ZONE_PROPERTY_LOCAL_ZONE")
      .def(py::init<>())
      .def_readwrite("_tJointRangeOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tJointRangeOverride)
      .def_readwrite("_tJointSpeedOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tJointSpeedOverride)
      .def_readwrite("_tTcpForceOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpForceOverride)
      .def_readwrite("_tTcpPowerOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpPowerOverride)
      .def_readwrite("_tTcpSpeedOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpSpeedOverride)
      .def_readwrite("_tTcpMomentumOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tTcpMomentumOverride)
      .def_readwrite("_tCollisionOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tCollisionOverride)
      .def_readwrite("_tSpeedRate",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tSpeedRate)
      .def_readwrite("_tCollisionViolationStopmodeOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::
                         _tCollisionViolationStopmodeOverride)
      .def_readwrite(
          "_tForceViolationStopmodeOverride",
          &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tForceViolationStopmodeOverride)
      .def_readwrite(
          "_tToolOrientationLimitOverride",
          &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_tToolOrientationLimitOverride)
      .def_readwrite("_iDynamicZoneEnable",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iDynamicZoneEnable)
      .def_readwrite("_iLedOverride",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iLedOverride)
      .def_readwrite("_iNundgeEanble",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iNundgeEanble)
      .def_readwrite("_iAllowLessSafeWork",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iAllowLessSafeWork)
      .def_readwrite("_iOverrideReduce",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iOverrideReduce)
      .def_readwrite("_iInsideZoneDectection",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_iInsideZoneDectection)
      .def_readwrite("_bCollaborativeZone",
                     &SAFETY_ZONE_PROPERTY_LOCAL_ZONE::_bCollaborativeZone);

  py::class_<SAFETY_ZONE_PROPERTY_DATA>(m, "SAFETY_ZONE_PROPERTY_DATA")
      .def(py::init<>())
      .def_property(
          "_tSpaceLimitZone",
          [](const SAFETY_ZONE_PROPERTY_DATA &s)
              -> const SAFETY_ZONE_PROPERTY_SPACE_LIMIT & {
            return s._tSpaceLimitZone;
          },
          [](SAFETY_ZONE_PROPERTY_DATA &s,
             const SAFETY_ZONE_PROPERTY_SPACE_LIMIT &v) {
            s._tSpaceLimitZone = v;
          })
      .def_property(
          "_tLocalZone",
          [](const SAFETY_ZONE_PROPERTY_DATA &s)
              -> const SAFETY_ZONE_PROPERTY_LOCAL_ZONE & {
            return s._tLocalZone;
          },
          [](SAFETY_ZONE_PROPERTY_DATA &s,
             const SAFETY_ZONE_PROPERTY_LOCAL_ZONE &v) { s._tLocalZone = v; });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_USER_COORDINATE / SAFETY zone shapes
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_USER_COORDINATE>(m, "CONFIG_USER_COORDINATE")
      .def(py::init<>()) ARRAY_PROP(CONFIG_USER_COORDINATE, _fTargetPos, float)
      .def_readwrite("_iReqId", &CONFIG_USER_COORDINATE::_iReqId);

  py::class_<SAFETY_ZONE_SHAPE_SPHERE>(m, "SAFETY_ZONE_SHAPE_SPHERE")
      .def(py::init<>())
      .def_readwrite("_tCenter", &SAFETY_ZONE_SHAPE_SPHERE::_tCenter)
      .def_readwrite("_fRadius", &SAFETY_ZONE_SHAPE_SPHERE::_fRadius);

  py::class_<SAFETY_ZONE_SHAPE_CYLINDER>(m, "SAFETY_ZONE_SHAPE_CYLINDER")
      .def(py::init<>())
      .def_readwrite("_tCenter", &SAFETY_ZONE_SHAPE_CYLINDER::_tCenter)
      .def_readwrite("_fRadius", &SAFETY_ZONE_SHAPE_CYLINDER::_fRadius)
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

  py::class_<SAFETY_ZONE_SHAPE_TILTED_CUBOID>(m,
                                              "SAFETY_ZONE_SHAPE_TILTED_CUBOID")
      .def(py::init<>())
      .def_readwrite("_tOrigin", &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tOrigin)
      .def_readwrite("_tUAxisEnd", &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tUAxisEnd)
      .def_readwrite("_tVAxisEnd", &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tVAxisEnd)
      .def_readwrite("_tWAxisEnd",
                     &SAFETY_ZONE_SHAPE_TILTED_CUBOID::_tWAxisEnd);

  py::class_<SAFETY_ZONE_SHAPE_CAPSULE>(m, "SAFETY_ZONE_SHAPE_CAPSULE")
      .def(py::init<>())
      .def_readwrite("_tCenter1", &SAFETY_ZONE_SHAPE_CAPSULE::_tCenter1)
      .def_readwrite("_tCenter2", &SAFETY_ZONE_SHAPE_CAPSULE::_tCenter2)
      .def_readwrite("_fRadius", &SAFETY_ZONE_SHAPE_CAPSULE::_fRadius);

  py::class_<SAFETY_ZONE_SHAPE_DATA>(m, "SAFETY_ZONE_SHAPE_DATA")
      .def(py::init<>())
      .def_property(
          "_iBuffer",
          [](const SAFETY_ZONE_SHAPE_DATA &s) {
            return array_copy(s._iBuffer);
          },
          [](SAFETY_ZONE_SHAPE_DATA &s, py::array_t<unsigned char> v) {
            array_set(s._iBuffer, v);
          });

  py::class_<SAFETY_ZONE_SHAPE>(m, "SAFETY_ZONE_SHAPE")
      .def(py::init<>())
      .def_readwrite("_iCoordinate", &SAFETY_ZONE_SHAPE::_iCoordinate)
      .def_readwrite("_iShapeType", &SAFETY_ZONE_SHAPE::_iShapeType)
      .def_readwrite("_tShapeData", &SAFETY_ZONE_SHAPE::_tShapeData)
      .def_readwrite("_fMargin", &SAFETY_ZONE_SHAPE::_fMargin)
      .def_readwrite("_iValidSpace", &SAFETY_ZONE_SHAPE::_iValidSpace);

  py::class_<CONFIG_ADD_SAFETY_ZONE>(m, "CONFIG_ADD_SAFETY_ZONE")
      .def(py::init<>()) STR_PROP(CONFIG_ADD_SAFETY_ZONE, _szIdentifier)
          STR_PROP(CONFIG_ADD_SAFETY_ZONE, _szAlias)
      .def_readwrite("_iZoneType", &CONFIG_ADD_SAFETY_ZONE::_iZoneType)
      .def_readwrite("_tZoneProperty", &CONFIG_ADD_SAFETY_ZONE::_tZoneProperty)
      .def_readwrite("_tShape", &CONFIG_ADD_SAFETY_ZONE::_tShape);

  m.attr("CONFIG_SAFETY_ZONE") = m.attr("CONFIG_ADD_SAFETY_ZONE");

  py::class_<CONFIG_DELETE_SAFETY_ZONE>(m, "CONFIG_DELETE_SAFETY_ZONE")
      .def(py::init<>()) STR_PROP(CONFIG_DELETE_SAFETY_ZONE, _szIdentifier);

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_ENCODER_POLARITY / CONFIG_ENCODER_MODE
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_ENCODER_POLARITY>(m, "CONFIG_ENCODER_POLARITY")
      .def(py::init<>())
      .def_readwrite("_iChannel", &CONFIG_ENCODER_POLARITY::_iChannel)
          ARRAY_PROP(CONFIG_ENCODER_POLARITY, _iPolarity, unsigned char);

  py::class_<CONFIG_ENCODER_MODE>(m, "CONFIG_ENCODER_MODE")
      .def(py::init<>())
      .def_readwrite("_iChannel", &CONFIG_ENCODER_MODE::_iChannel)
      .def_readwrite("_iABMode", &CONFIG_ENCODER_MODE::_iABMode)
      .def_readwrite("_iZMode", &CONFIG_ENCODER_MODE::_iZMode)
      .def_readwrite("_iSMode", &CONFIG_ENCODER_MODE::_iSMode)
      .def_readwrite("_iInvMode", &CONFIG_ENCODER_MODE::_iInvMode)
      .def_readwrite("_nPulseAZ", &CONFIG_ENCODER_MODE::_nPulseAZ);

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_IO_FUNC / CONFIG_REMOTE_CONTROL
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_IO_FUNC>(m, "CONFIG_IO_FUNC")
      .def(py::init<>())
      .def_readwrite("_iPort", &CONFIG_IO_FUNC::_iPort)
      .def_readwrite("_bLevel", &CONFIG_IO_FUNC::_bLevel);

  py::class_<CONFIG_REMOTE_CONTROL>(m, "CONFIG_REMOTE_CONTROL")
      .def(py::init<>())
      .def_readwrite("_bEnable", &CONFIG_REMOTE_CONTROL::_bEnable)
      .def_property(
          "_tFunc",
          [](const CONFIG_REMOTE_CONTROL &s) { return array2d_copy(s._tFunc); },
          [](CONFIG_REMOTE_CONTROL &s, py::array_t<CONFIG_IO_FUNC> /*v*/) {
            // Compound type 2D array – use raw buffer copy
            throw std::runtime_error(
                "Use individual element assignment for CONFIG_IO_FUNC arrays");
          });

  // ─────────────────────────────────────────────────────────────────────────
  // PROGRAM_SYNTAX_CHECK / PROGRAM_EXECUTION_EX / PROGRAM_WATCH_VARIABLE /
  // PROGRAM_ERROR
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<PROGRAM_SYNTAX_CHECK>(m, "PROGRAM_SYNTAX_CHECK")
      .def(py::init<>())
      .def_readwrite("_iTextLength", &PROGRAM_SYNTAX_CHECK::_iTextLength);

  py::class_<PROGRAM_EXECUTION_EX>(m, "PROGRAM_EXECUTION_EX")
      .def(py::init<>())
      .def_readwrite("_iLineNumber", &PROGRAM_EXECUTION_EX::_iLineNumber)
      .def_readwrite("_fElapseTime", &PROGRAM_EXECUTION_EX::_fElapseTime)
          STR_PROP(PROGRAM_EXECUTION_EX, _szFile);

  py::class_<PROGRAM_WATCH_VARIABLE>(m, "PROGRAM_WATCH_VARIABLE")
      .def(py::init<>())
      .def_readwrite("_iDivision", &PROGRAM_WATCH_VARIABLE::_iDivision)
      .def_readwrite("_iType", &PROGRAM_WATCH_VARIABLE::_iType)
          STR_PROP(PROGRAM_WATCH_VARIABLE, _szName)
              STR_PROP(PROGRAM_WATCH_VARIABLE, _szData);

  py::class_<PROGRAM_ERROR>(m, "PROGRAM_ERROR")
      .def(py::init<>())
      .def_readwrite("_iError", &PROGRAM_ERROR::_iError)
      .def_readwrite("_nLine", &PROGRAM_ERROR::_nLine)
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
      .def_readwrite("_bEnable", &CONFIG_WELDING_INTERFACE::_bEnable)
      .def_property(
          "_tChOut",
          [](const CONFIG_WELDING_INTERFACE &s) {
            py::list lst;
            for (int i = 0; i < 2; ++i)
              lst.append(s._tChOut[i]);
            return lst;
          },
          [](CONFIG_WELDING_INTERFACE &s, py::list v) {
            if (v.size() != 2)
              throw std::out_of_range("Expected 2 elements");
            for (int i = 0; i < 2; ++i)
              s._tChOut[i] = v[i].cast<WELDING_CHANNEL>();
          })
      .def_property(
          "_tChIn",
          [](const CONFIG_WELDING_INTERFACE &s) {
            py::list lst;
            for (int i = 0; i < 2; ++i)
              lst.append(s._tChIn[i]);
            return lst;
          },
          [](CONFIG_WELDING_INTERFACE &s, py::list v) {
            if (v.size() != 2)
              throw std::out_of_range("Expected 2 elements");
            for (int i = 0; i < 2; ++i)
              s._tChIn[i] = v[i].cast<WELDING_CHANNEL>();
          })
      .def_readwrite("_iArcOnDO", &CONFIG_WELDING_INTERFACE::_iArcOnDO)
      .def_readwrite("_iGasOnDO", &CONFIG_WELDING_INTERFACE::_iGasOnDO)
      .def_readwrite("_iInchPDO", &CONFIG_WELDING_INTERFACE::_iInchPDO)
      .def_readwrite("_iInchNDO", &CONFIG_WELDING_INTERFACE::_iInchNDO);

  py::class_<ROBOT_WELDING_DATA>(m, "ROBOT_WELDING_DATA")
      .def(py::init<>())
      .def_readwrite("_iAdjAvail", &ROBOT_WELDING_DATA::_iAdjAvail)
      .def_readwrite("_fTargetVol", &ROBOT_WELDING_DATA::_fTargetVol)
      .def_readwrite("_fTargetCur", &ROBOT_WELDING_DATA::_fTargetCur)
      .def_readwrite("_fTargetVel", &ROBOT_WELDING_DATA::_fTargetVel)
      .def_readwrite("_fActualVol", &ROBOT_WELDING_DATA::_fActualVol)
      .def_readwrite("_fActualCur", &ROBOT_WELDING_DATA::_fActualCur)
      .def_readwrite("_fOffsetY", &ROBOT_WELDING_DATA::_fOffsetY)
      .def_readwrite("_fOffsetZ", &ROBOT_WELDING_DATA::_fOffsetZ)
      .def_readwrite("_iArcOnDO", &ROBOT_WELDING_DATA::_iArcOnDO)
      .def_readwrite("_iGasOnDO", &ROBOT_WELDING_DATA::_iGasOnDO)
      .def_readwrite("_iInchPDO", &ROBOT_WELDING_DATA::_iInchPDO)
      .def_readwrite("_iInchNPO", &ROBOT_WELDING_DATA::_iInchNPO)
      .def_readwrite("_iStatus", &ROBOT_WELDING_DATA::_iStatus);

  m.attr("MONITORING_WELDING") = m.attr("ROBOT_WELDING_DATA");

  // ─────────────────────────────────────────────────────────────────────────
  // POSITION_EX (complex union)
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<POSITION_EX>(m, "POSITION_EX")
      .def(py::init<>())
      .def_readwrite("_pos_type", &POSITION_EX::_pos_type)
      // posj union arm
      .def_property(
          "posj_pos",
          [](const POSITION_EX &s) { return array_copy(s._posj._pos); },
          [](POSITION_EX &s, py::array_t<float> v) {
            array_set(s._posj._pos, v);
          })
      // posx union arm
      .def_property(
          "posx_pos",
          [](const POSITION_EX &s) { return array_copy(s._posx._pos); },
          [](POSITION_EX &s, py::array_t<float> v) {
            array_set(s._posx._pos, v);
          })
      .def_property(
          "_ori_type", [](const POSITION_EX &s) { return s._posx._ori_type; },
          [](POSITION_EX &s, unsigned char v) { s._posx._ori_type = v; })
      .def_property(
          "_sol_space", [](const POSITION_EX &s) { return s._posx._sol_space; },
          [](POSITION_EX &s, unsigned char v) { s._posx._sol_space = v; })
      .def_property(
          "_multi_turn",
          [](const POSITION_EX &s) { return s._posx._multi_turn; },
          [](POSITION_EX &s, unsigned char v) { s._posx._multi_turn = v; });

  m.attr("CONFIG_TCP_EX") = m.attr("POSITION_EX");

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_TCP_SYMBOL_EX / CONFIG_TCP_LIST_EX
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_TCP_SYMBOL_EX>(m, "CONFIG_TCP_SYMBOL_EX")
      .def(py::init<>()) STR_PROP(CONFIG_TCP_SYMBOL_EX, _szSymbol)
      .def_readwrite("_tTCP", &CONFIG_TCP_SYMBOL_EX::_tTCP);

  py::class_<CONFIG_TCP_LIST_EX>(m, "CONFIG_TCP_LIST_EX")
      .def(py::init<>())
      .def_readwrite("_iToolCount", &CONFIG_TCP_LIST_EX::_iToolCount)
      .def_property(
          "_tTooList",
          [](const CONFIG_TCP_LIST_EX &s) {
            py::list lst;
            for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i)
              lst.append(s._tTooList[i]);
            return lst;
          },
          [](CONFIG_TCP_LIST_EX &s, py::list v) {
            if ((int)v.size() != MAX_CONFIG_TCP_SIZE)
              throw std::out_of_range("Size mismatch");
            for (int i = 0; i < MAX_CONFIG_TCP_SIZE; ++i)
              s._tTooList[i] = v[i].cast<CONFIG_TCP_SYMBOL_EX>();
          });

  // ─────────────────────────────────────────────────────────────────────────
  // CONFIG_WORLD_COORDINATE_EX / CONFIG_USER_COORDINATE_EX2
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<CONFIG_WORLD_COORDINATE_EX>(m, "CONFIG_WORLD_COORDINATE_EX")
      .def(py::init<>())
      .def_readwrite("_iType", &CONFIG_WORLD_COORDINATE_EX::_iType)
      .def_readwrite("_tPosition", &CONFIG_WORLD_COORDINATE_EX::_tPosition);

  py::class_<CONFIG_USER_COORDINATE_EX2>(m, "CONFIG_USER_COORDINATE_EX2")
      .def(py::init<>())
      .def_readwrite("_iTargetRef", &CONFIG_USER_COORDINATE_EX2::_iTargetRef)
      .def_readwrite("_tTargetPos", &CONFIG_USER_COORDINATE_EX2::_tTargetPos)
      .def_readwrite("_iUserID", &CONFIG_USER_COORDINATE_EX2::_iUserID);

  // ─────────────────────────────────────────────────────────────────────────
  // ROBOT_LINK_INFO
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<ROBOT_LINK_INFO>(m, "ROBOT_LINK_INFO")
      .def(py::init<>()) ARRAY_PROP(ROBOT_LINK_INFO, d, float)
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
      .def_property(
          "_fPositive",
          [](const MEASURE_FRICTION_RESPONSE &s) {
            return array2d_copy(s._fPositive);
          },
          [](MEASURE_FRICTION_RESPONSE &s, py::array_t<float> v) {
            array2d_set(s._fPositive, v);
          })
      .def_property(
          "_fNegative",
          [](const MEASURE_FRICTION_RESPONSE &s) {
            return array2d_copy(s._fNegative);
          },
          [](MEASURE_FRICTION_RESPONSE &s, py::array_t<float> v) {
            array2d_set(s._fNegative, v);
          }) ARRAY_PROP(MEASURE_FRICTION_RESPONSE, _fTemperature, float);

  py::class_<USER_COORDINATE_MATRIX_RESPONSE>(m,
                                              "USER_COORDINATE_MATRIX_RESPONSE")
      .def(py::init<>())
      .def_property(
          "_fOrientXYZ",
          [](const USER_COORDINATE_MATRIX_RESPONSE &s) {
            return array2d_copy(s._fOrientXYZ);
          },
          [](USER_COORDINATE_MATRIX_RESPONSE &s, py::array_t<float> v) {
            array2d_set(s._fOrientXYZ, v);
          }) ARRAY_PROP(USER_COORDINATE_MATRIX_RESPONSE, _fTranslXYZ, float);

  py::class_<POSITION_ADDTO>(m, "POSITION_ADDTO")
      .def(py::init<>()) ARRAY_PROP(POSITION_ADDTO, _fTargetPos, float)
          ARRAY_PROP(POSITION_ADDTO, _fTargetVal, float);

  py::class_<VIRTUAL_FENCE_RESPONSE>(m, "VIRTUAL_FENCE_RESPONSE")
      .def(py::init<>())
          ARRAY_PROP(VIRTUAL_FENCE_RESPONSE, _iCubeResult, unsigned char)
              ARRAY_PROP(VIRTUAL_FENCE_RESPONSE, _iPolyResult, unsigned char)
      .def_readwrite("_iCylinderResult",
                     &VIRTUAL_FENCE_RESPONSE::_iCylinderResult);

  py::class_<REPORT_TCP_CLIENT>(m, "REPORT_TCP_CLIENT")
      .def(py::init<>())
      .def_readwrite("_iId", &REPORT_TCP_CLIENT::_iId)
      .def_readwrite("_iCount", &REPORT_TCP_CLIENT::_iCount);

  py::class_<SERIAL_PORT_NAME>(m, "SERIAL_PORT_NAME")
      .def(py::init<>()) STR_PROP(SERIAL_PORT_NAME, _szPort)
          STR_PROP(SERIAL_PORT_NAME, _szName);

  py::class_<SERIAL_SEARCH>(m, "SERIAL_SEARCH")
      .def(py::init<>())
      .def_readwrite("_nCount", &SERIAL_SEARCH::_nCount)
      .def_property(
          "_tSerial",
          [](const SERIAL_SEARCH &s) {
            py::list lst;
            for (int i = 0; i < 10; ++i)
              lst.append(s._tSerial[i]);
            return lst;
          },
          [](SERIAL_SEARCH &s, py::list v) {
            if (v.size() != 10)
              throw std::out_of_range("Expected 10 elements");
            for (int i = 0; i < 10; ++i)
              s._tSerial[i] = v[i].cast<SERIAL_PORT_NAME>();
          });

  py::class_<MONITORING_MBUS_SLAVE_COIL>(m, "MONITORING_MBUS_SLAVE_COIL")
      .def(py::init<>())
      .def_readwrite("_nCtrlDigitalInput",
                     &MONITORING_MBUS_SLAVE_COIL::_nCtrlDigitalInput)
      .def_readwrite("_nCtrlDigitalOutput",
                     &MONITORING_MBUS_SLAVE_COIL::_nCtrlDigitalOutput)
      .def_readwrite("_nToolDigitalInput",
                     &MONITORING_MBUS_SLAVE_COIL::_nToolDigitalInput)
      .def_readwrite("_nToolDigitalOutput",
                     &MONITORING_MBUS_SLAVE_COIL::_nToolDigitalOutput)
      .def_readwrite("_nServoOnRobot",
                     &MONITORING_MBUS_SLAVE_COIL::_nServoOnRobot)
      .def_readwrite("_nEmergencyStopped",
                     &MONITORING_MBUS_SLAVE_COIL::_nEmergencyStopped)
      .def_readwrite("_nSafetyStopped",
                     &MONITORING_MBUS_SLAVE_COIL::_nSafetyStopped)
      .def_readwrite("_nDirectTeachButtonPress",
                     &MONITORING_MBUS_SLAVE_COIL::_nDirectTeachButtonPress)
      .def_readwrite("_nPowerButtonPress",
                     &MONITORING_MBUS_SLAVE_COIL::_nPowerButtonPress)
      .def_readwrite(
          "_nSafetyStoppedRequiredRecoveryMode",
          &MONITORING_MBUS_SLAVE_COIL::_nSafetyStoppedRequiredRecoveryMode);

  py::class_<MONITORING_MBUS_SLAVE_HOILDING_REGISTER>(
      m, "MONITORING_MBUS_SLAVE_HOILDING_REGISTER")
      .def(py::init<>())
      .def_readwrite(
          "_nCtrlDigitalInput",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlDigitalInput)
      .def_readwrite(
          "_nCtrlDigitalOutput",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlDigitalOutput)
      .def_readwrite(
          "_nCtrlAnalogInput1",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput1)
      .def_readwrite(
          "_nCtrlAnalogInput1Type",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput1Type)
      .def_readwrite(
          "_nCtrlAnalogInput2",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput2)
      .def_readwrite(
          "_nCtrlAnalogInput2Type",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogInput2Type)
      .def_readwrite(
          "_nCtrlAnalogOutput1",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput1)
      .def_readwrite(
          "_nCtrlAnalogOutput1Type",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput1Type)
      .def_readwrite(
          "_nCtrlAnalogOutput2",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput2)
      .def_readwrite(
          "_nCtrlAnalogOutput2Type",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlAnalogOutput2Type)
      .def_readwrite(
          "_nCtrlToolDigitalInput",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlToolDigitalInput)
      .def_readwrite(
          "_nCtrlToolDigitalOutput",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlToolDigitalOutput)
          ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nGPR,
                     unsigned short)
      .def_readwrite("_nCtrlMajorVer",
                     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlMajorVer)
      .def_readwrite("_nCtrlMinorVer",
                     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlMinorVer)
      .def_readwrite("_nCtrlPatchVer",
                     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nCtrlPatchVer)
      .def_readwrite("_nRobotState",
                     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nRobotState)
      .def_readwrite("_nServoOnRobot",
                     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nServoOnRobot)
      .def_readwrite(
          "_nEmergencyStopped",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nEmergencyStopped)
      .def_readwrite("_nSafetyStopped",
                     &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nSafetyStopped)
      .def_readwrite(
          "_nDirectTeachButtonPressed",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nDirectTeachButtonPressed)
      .def_readwrite(
          "_nPowerButtonPressed",
          &MONITORING_MBUS_SLAVE_HOILDING_REGISTER::_nPowerButtonPressed)
          ARRAY_PROP(
              MONITORING_MBUS_SLAVE_HOILDING_REGISTER, _nJointPosition,
              unsigned short) ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                                         _nJointVelocity, unsigned short)
              ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                         _nJointMotorCurrent, unsigned short)
                  ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                             _nJointMotorTemp, unsigned short)
                      ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                                 _nJointTorque, unsigned short)
                          ARRAY_PROP(MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                                     _nTaskPosition, unsigned short)
                              ARRAY_PROP(
                                  MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                                  _nTaskVelocity, unsigned short)
                                  ARRAY_PROP(
                                      MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                                      _nToolOffsetLength, unsigned short)
                                      ARRAY_PROP(
                                          MONITORING_MBUS_SLAVE_HOILDING_REGISTER,
                                          _nTaskExternalForce, unsigned short);

  py::class_<MONITORING_IE_GPR>(m, "MONITORING_IE_GPR")
      .def(py::init<>()) ARRAY_PROP(MONITORING_IE_GPR, _nGpr, unsigned char);

  py::class_<MONITORING_IE_SLAVE>(m, "MONITORING_IE_SLAVE")
      .def(py::init<>())
      .def_readwrite("_tMbusCoil", &MONITORING_IE_SLAVE::_tMbusCoil)
      .def_readwrite("_tMbusHoldingRegister",
                     &MONITORING_IE_SLAVE::_tMbusHoldingRegister)
      .def_readwrite("_tIndustrialEthernetGPR",
                     &MONITORING_IE_SLAVE::_tIndustrialEthernetGPR);

  py::class_<CONFIG_SAFETY_PARAM_ENABLE>(m, "CONFIG_SAFETY_PARAM_ENABLE")
      .def(py::init<>())
      .def_readwrite("_wPreviousCmdid",
                     &CONFIG_SAFETY_PARAM_ENABLE::_wPreviousCmdid)
      .def_readwrite("_iRefCrc32", &CONFIG_SAFETY_PARAM_ENABLE::_iRefCrc32);

  // ─────────────────────────────────────────────────────────────────────────
  // USER_COORD_EXTERNAL_FORCE structs
  // ─────────────────────────────────────────────────────────────────────────
  py::class_<USER_COORD_EXTERNAL_FORCE>(m, "USER_COORD_EXTERNAL_FORCE")
      .def(py::init<>())
      .def_readwrite("_iUserId", &USER_COORD_EXTERNAL_FORCE::_iUserId)
          ARRAY_PROP(USER_COORD_EXTERNAL_FORCE, _fExternalForce, float);

  py::class_<USER_COORD_EXTERNAL_FORCE_INFO>(m,
                                             "USER_COORD_EXTERNAL_FORCE_INFO")
      .def(py::init<>())
      .def_readwrite("bIsMonitoring",
                     &USER_COORD_EXTERNAL_FORCE_INFO::bIsMonitoring)
          ARRAY_PROP(USER_COORD_EXTERNAL_FORCE_INFO, iUserID, unsigned char);

  py::class_<DIGITAL_WELDING_COMM_STATE>(m, "DIGITAL_WELDING_COMM_STATE")
      .def(py::init<>())
      .def_readwrite("_cWeldingMachineOnline",
                     &DIGITAL_WELDING_COMM_STATE::_cWeldingMachineOnline)
      .def_readwrite("_cWeldingEipSlaveState",
                     &DIGITAL_WELDING_COMM_STATE::_cWeldingEipSlaveState);

  py::class_<ROBOT_LED_CONFIG>(m, "ROBOT_LED_CONFIG")
      .def(py::init<>())
      .def_readwrite("_szLedRule", &ROBOT_LED_CONFIG::_szLedRule)
      .def_readwrite("_szCommandColor", &ROBOT_LED_CONFIG::_szCommandColor)
      .def_property(
          "_szStateColor",
          [](const ROBOT_LED_CONFIG &s) {
            return array2d_copy(s._szStateColor);
          },
          [](ROBOT_LED_CONFIG &s, py::array_t<unsigned char> v) {
            array2d_set(s._szStateColor, v);
          });

  py::class_<COUNTER_BALANCE_PARAM_DATA>(m, "COUNTER_BALANCE_PARAM_DATA")
      .def(py::init<>())
      .def_readwrite("_fK", &COUNTER_BALANCE_PARAM_DATA::_fK)
      .def_readwrite("_fIrod", &COUNTER_BALANCE_PARAM_DATA::_fIrod)
      .def_readwrite("_fR", &COUNTER_BALANCE_PARAM_DATA::_fR)
      .def_readwrite("_fSi", &COUNTER_BALANCE_PARAM_DATA::_fSi);

  py::class_<CALIBRATION_PARAM_DATA>(m, "CALIBRATION_PARAM_DATA")
      .def(py::init<>())
      .def_readwrite("_Ax", &CALIBRATION_PARAM_DATA::_Ax)
      .def_readwrite("_Bx", &CALIBRATION_PARAM_DATA::_Bx)
      .def_readwrite("_Cx", &CALIBRATION_PARAM_DATA::_Cx)
      .def_readwrite("_Dx", &CALIBRATION_PARAM_DATA::_Dx);

  py::class_<LICENSE_TEXT_PARAM>(m, "LICENSE_TEXT_PARAM")
      .def(py::init<>())
      .def_readwrite("_bLicenseInController",
                     &LICENSE_TEXT_PARAM::_bLicenseInController)
          ARRAY_PROP(LICENSE_TEXT_PARAM, _szLicenseKey, unsigned char);

  py::class_<WEAVING_OFFSET>(m, "WEAVING_OFFSET")
      .def(py::init<>())
      .def_readwrite("_fOffsetY", &WEAVING_OFFSET::_fOffsetY)
      .def_readwrite("_fOffsetZ", &WEAVING_OFFSET::_fOffsetZ);

  py::class_<IETHERNET_SLAVE_DATA_EX>(m, "IETHERNET_SLAVE_DATA_EX")
      .def(py::init<>())
      .def_readwrite("_iGprType", &IETHERNET_SLAVE_DATA_EX::_iGprType)
      .def_readwrite("_iGprAddr", &IETHERNET_SLAVE_DATA_EX::_iGprAddr)
      .def_readwrite("_iInOut", &IETHERNET_SLAVE_DATA_EX::_iInOut);

  py::class_<IETHERNET_SLAVE_RESPONSE_DATA_EX>(
      m, "IETHERNET_SLAVE_RESPONSE_DATA_EX")
      .def(py::init<>())
      .def_readwrite("_iGprType", &IETHERNET_SLAVE_RESPONSE_DATA_EX::_iGprType)
      .def_readwrite("_iGprAddr", &IETHERNET_SLAVE_RESPONSE_DATA_EX::_iGprAddr)
      .def_readwrite("_iInOut", &IETHERNET_SLAVE_RESPONSE_DATA_EX::_iInOut)
          STR_PROP(IETHERNET_SLAVE_RESPONSE_DATA_EX, _szData);
}
