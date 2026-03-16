/**
 * @brief Wraps the API-DRFL library to use in Python
 *
 *
 *
 */

#include "../API-DRFL/include/DRFL.h"

#include "./cdrflex_bindings.hpp"
#include "./drfl_structs.hpp"
#include "./drfl_enums.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(doosan_drfl, m) {
  m.doc() = "Python binding for Doosan Robotics API-DRFL using pybind11";

  /*********
   * ENUMS *
   *********/
  bind_drfl_enums(m);

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
