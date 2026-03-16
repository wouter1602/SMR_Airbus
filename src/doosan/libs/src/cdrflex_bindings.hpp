#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "../API-DRFL/include/DRFL.h"

namespace py = pybind11;
using arr_f = py::array_t<float, py::array::c_style | py::array::forcecast>;


// Connection functions
void bOpenConnection(py::class_<DRAFramework::CDRFLEx>& c);
void bCloseConnection(py::class_<DRAFramework::CDRFLEx>& c);

// Robot property functions
void bGetSystemVersion(py::class_<DRAFramework::CDRFLEx>& c);
void bGetLibraryVersion(py::class_<DRAFramework::CDRFLEx>& c);
void bGetRobotMode(py::class_<DRAFramework::CDRFLEx>& c);
void bSetRobotMode(py::class_<DRAFramework::CDRFLEx>& c);
void bGetRobotState(py::class_<DRAFramework::CDRFLEx>& c);
void bSetRobotControl(py::class_<DRAFramework::CDRFLEx>& c);
void bSetRobotSystem(py::class_<DRAFramework::CDRFLEx>& c);
void bGetRobotSpeedMode(py::class_<DRAFramework::CDRFLEx>& c);
void bSetRobotSpeedMode(py::class_<DRAFramework::CDRFLEx>& c);
void bGetProgramState(py::class_<DRAFramework::CDRFLEx>& c);
void bGetRobotSystem(py::class_<DRAFramework::CDRFLEx>& c);
void bSetSafeStopResetType(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentPose(py::class_<DRAFramework::CDRFLEx>& c);
void bGetcurrentPosj(py::class_<DRAFramework::CDRFLEx>& c);
void bGetDesiredPosj(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentVelj(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentPosx(py::class_<DRAFramework::CDRFLEx>& c);
void bGetDesiredPosx(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentToolFlangePosx(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentVelx(py::class_<DRAFramework::CDRFLEx>& c);
void bGetDesiredVelx(py::class_<DRAFramework::CDRFLEx>& c);
void bGetJointTorque(py::class_<DRAFramework::CDRFLEx>& c);
void bGetControlSpace(py::class_<DRAFramework::CDRFLEx>& c);
void bGetExternalTorque(py::class_<DRAFramework::CDRFLEx>& c);
void bGetToolForce(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentSolutionSpace(py::class_<DRAFramework::CDRFLEx>& c);
void bGetLastalarm(py::class_<DRAFramework::CDRFLEx>& c);
void bGetSolutionSpace(py::class_<DRAFramework::CDRFLEx>& c);
void bGetOrientationError(py::class_<DRAFramework::CDRFLEx>& c);
void bGetControlMode(py::class_<DRAFramework::CDRFLEx>& c);
void bGetCurrentRotm(py::class_<DRAFramework::CDRFLEx>& c);
void bGetSafetyConfiguration(py::class_<DRAFramework::CDRFLEx>& c);

// Robot callback functions
void bSetOnMonitoringState(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringData(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringDataEx(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringCtrlIo(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringCtrlIoEx(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringModbus(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnLogAlarm(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnTpPopup(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnTpLog(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnTpProgress(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnTpGetUserInput(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringAccessControl(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnHommingCompleted(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnTpInitializingCompleted(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringSpeedMode(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMasteringNeed(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnProgramStopped(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnDisconnected(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringRobotSystem(py::class_<DRAFramework::CDRFLEx>& c);
void bSetOnMonitoringSafetyState(py::class_<DRAFramework::CDRFLEx>& c);

// Robot control right functions
void bManageAccessControl(py::class_<DRAFramework::CDRFLEx>& c);

// Basic control functions
void bJog(py::class_<DRAFramework::CDRFLEx>& c);
void bMoveHome(py::class_<DRAFramework::CDRFLEx>& c);


// Robot motion functions
void bMovej(py::class_<DRAFramework::CDRFLEx>& c);
void bMovel(py::class_<DRAFramework::CDRFLEx>& c);
void bMovejx(py::class_<DRAFramework::CDRFLEx>& c);
void bMovec(py::class_<DRAFramework::CDRFLEx>& c);
void bMovesj(py::class_<DRAFramework::CDRFLEx>& c);
void bMovesx(py::class_<DRAFramework::CDRFLEx>& c);
void bMoveb(py::class_<DRAFramework::CDRFLEx>& c);
void bMoveSpiral(py::class_<DRAFramework::CDRFLEx>& c);
void bMovePeriodic(py::class_<DRAFramework::CDRFLEx>& c);
void bAmovej(py::class_<DRAFramework::CDRFLEx>& c);
void bAmovel(py::class_<DRAFramework::CDRFLEx>& c);
void bAmovec(py::class_<DRAFramework::CDRFLEx>& c);
void bAmovesj(py::class_<DRAFramework::CDRFLEx>& c);
void bAmovesx(py::class_<DRAFramework::CDRFLEx>& c);
void bAmoveb(py::class_<DRAFramework::CDRFLEx>& c);
void bAmoveSpiral(py::class_<DRAFramework::CDRFLEx>& c);
void bAmovePeriodic(py::class_<DRAFramework::CDRFLEx>& c);
void bStop(py::class_<DRAFramework::CDRFLEx>& c);
void bTrans(py::class_<DRAFramework::CDRFLEx>& c);
void bFkin(py::class_<DRAFramework::CDRFLEx>& c);
void bIkine(py::class_<DRAFramework::CDRFLEx>& c);
void bSetRefCoord(py::class_<DRAFramework::CDRFLEx>& c);
void bCheckMotion(py::class_<DRAFramework::CDRFLEx>& c);
void bEnableAlterMotion(py::class_<DRAFramework::CDRFLEx>& c);
void bAlterMotion(py::class_<DRAFramework::CDRFLEx>& c);
void bDisableAlterMotion(py::class_<DRAFramework::CDRFLEx>& c);
void bServoj(py::class_<DRAFramework::CDRFLEx>& c);
void bServol(py::class_<DRAFramework::CDRFLEx>& c);
void bSpeedj(py::class_<DRAFramework::CDRFLEx>& c);
void bSpeedl(py::class_<DRAFramework::CDRFLEx>& c);

// Robot settings functions
void bAddTool(py::class_<DRAFramework::CDRFLEx>& c);
void bDelTool(py::class_<DRAFramework::CDRFLEx>& c);
void bSetTool(py::class_<DRAFramework::CDRFLEx>& c);
void bGetTool(py::class_<DRAFramework::CDRFLEx>& c);
void bAddTcp(py::class_<DRAFramework::CDRFLEx>& c);
void bDelTcp(py::class_<DRAFramework::CDRFLEx>& c);
void bSetTcp(py::class_<DRAFramework::CDRFLEx>& c);
void bGetTcp(py::class_<DRAFramework::CDRFLEx>& c);
void bSetToolShape(py::class_<DRAFramework::CDRFLEx>& c);
void bGetWorkpieceWeight(py::class_<DRAFramework::CDRFLEx>& c);
void bResetWorkpieceWeight(py::class_<DRAFramework::CDRFLEx>& c);
void bSetSingularityHandling(py::class_<DRAFramework::CDRFLEx>& c);
void bSetupMonitoringVersion(py::class_<DRAFramework::CDRFLEx>& c);
void bConfigProgramWatchVariable(py::class_<DRAFramework::CDRFLEx>& c);
void bSetUserHome(py::class_<DRAFramework::CDRFLEx>& c);
void bServoOff(py::class_<DRAFramework::CDRFLEx>& c);
void bReleaseProtectiveStop(py::class_<DRAFramework::CDRFLEx>& c);
void bChangeCollisionSensitivity(py::class_<DRAFramework::CDRFLEx>& c);
void bSetSafetyMode(py::class_<DRAFramework::CDRFLEx>& c);
void bSetAutoServoOff(py::class_<DRAFramework::CDRFLEx>& c);
void bSetWorkpieceWeight(py::class_<DRAFramework::CDRFLEx>& c);

// Robot I/O functions
void bSetToolDigitalOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bGetToolDigitalOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bGetToolDigitalInput(py::class_<DRAFramework::CDRFLEx>& c);
void bSetDigitalOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bGetDigitalOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bGetDigitalInput(py::class_<DRAFramework::CDRFLEx>& c);
void bSetModeAnalogInput(py::class_<DRAFramework::CDRFLEx>& c);
void bSetModeAnalogOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bSetAnalogOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bGetAnalogInput(py::class_<DRAFramework::CDRFLEx>& c);
void bAddModbusSignal(py::class_<DRAFramework::CDRFLEx>& c);
void bDelModbusSignal(py::class_<DRAFramework::CDRFLEx>& c);
void bSetModbusOutput(py::class_<DRAFramework::CDRFLEx>& c);
void bGetModbusInput(py::class_<DRAFramework::CDRFLEx>& c);
void bFlangeSerialOpen(py::class_<DRAFramework::CDRFLEx>& c);
void bFlangeSerialClose(py::class_<DRAFramework::CDRFLEx>& c);
void bFlangeSerialWrite(py::class_<DRAFramework::CDRFLEx>& c);
void bGetToolAnalogInput(py::class_<DRAFramework::CDRFLEx>& c);
void bSetToolDigitalOutputLevel(py::class_<DRAFramework::CDRFLEx>& c);
void bSetToolDigitalOutputType(py::class_<DRAFramework::CDRFLEx>& c);
void bSetModeToolAnalogInput(py::class_<DRAFramework::CDRFLEx>& c);

// Robot program control functions
void bDrlStart(py::class_<DRAFramework::CDRFLEx>& c);
void bDrlStop(py::class_<DRAFramework::CDRFLEx>& c);
void bDrlPause(py::class_<DRAFramework::CDRFLEx>& c);
void bDrlResume(py::class_<DRAFramework::CDRFLEx>& c);
void bChangeOperationSpeed(py::class_<DRAFramework::CDRFLEx>& c);
void bSaveSubProgram(py::class_<DRAFramework::CDRFLEx>& c);
void bTpPopupResponse(py::class_<DRAFramework::CDRFLEx>& c);
void bTpGetUserInputResponse(py::class_<DRAFramework::CDRFLEx>& c);

// Miscellaneous functions
void bParallelAxis(py::class_<DRAFramework::CDRFLEx>& c);
void bAlignAxis(py::class_<DRAFramework::CDRFLEx>& c);
void bIsDoneBoltTightening(py::class_<DRAFramework::CDRFLEx>& c);
void bTaskComplianceCtrl(py::class_<DRAFramework::CDRFLEx>& c);
void bReleaseComplianceCtrl(py::class_<DRAFramework::CDRFLEx>& c);
void bSetStiffnessx(py::class_<DRAFramework::CDRFLEx>& c);
void bCalcCoord(py::class_<DRAFramework::CDRFLEx>& c);
void bSetUserCartCoord(py::class_<DRAFramework::CDRFLEx>& c);
void bOverwriteUserCartCoord(py::class_<DRAFramework::CDRFLEx>& c);
void bGetUserCartCoord(py::class_<DRAFramework::CDRFLEx>& c);
void bSetDesiredForce(py::class_<DRAFramework::CDRFLEx>& c);
void bReleaseForce(py::class_<DRAFramework::CDRFLEx>& c);
void bCheckPositionConditionAbs(py::class_<DRAFramework::CDRFLEx>& c);
void bCheckPositionConditionRel(py::class_<DRAFramework::CDRFLEx>& c);
void bCheckPositionCondition(py::class_<DRAFramework::CDRFLEx>& c);
void bCheckForceCondition(py::class_<DRAFramework::CDRFLEx>& c);
void bCheckOrientationCondition(py::class_<DRAFramework::CDRFLEx>& c);
void bCoordTransform(py::class_<DRAFramework::CDRFLEx>& c);
void bSetPalletizingMode(py::class_<DRAFramework::CDRFLEx>& c);
void bQueryModbusDataList(py::class_<DRAFramework::CDRFLEx>& c);
