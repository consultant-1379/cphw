/*
 * CmdErrMsg.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: xdtthng
 */

#include "CmdErrMsg.h"
#include "boost/assign/list_of.hpp"
#include "boost/algorithm/string.hpp"


std::map<Cmd_Error_Code, std::string> cmd_error_msg = boost::assign::map_list_of
	(eNoError, "Executed")
	(eExecError, "Error when executing (General fault)")
	(eSyntaxError, "Incorrect usage")
	(eMandatoryOption, "Option -cp is mandatory in this system configuration")
	(eInvalidValue, "Invalid value")
	(eClusterNotSupported, "Cluster is not supported")
	(eCSUnreachable, "Configuration service is not reachable")
	(eMausNotSupported, "Maus not yet supported for this configuration")
	(eCpNameNotDefined, "CP name undefined")

	(eBMsgQCreate, "Cannot create message queue to receive results from MAUS")
	(eEmptyMsg, "Zero byte received while expecting a message")
	(eCp1orCp2Supported, "Only CP1 or CP2 is supported")
	(eOmHandlerFault, "OM Handler Internal Fault")
	(eAllInstancesLocked, "Service already locked")
	(eLockingFailed, "Locking service instance failed")
	(eLockingOneOK, "Locking the first service instance ok; the second failed")
	(eAllInstancesUnlocked, "Service already unlocked")
	(eUnlockingFailed, "Unlocking service instance failed")
	(eUnlockingOneOK, "Unlocking the first service instance ok; the second failed")
	(eUserAbort, "User Abort")
	(eInstanceLocked, "Specified instance already locked")
	(eInstanceUnlocked, "Specified instance already unlocked")
	(eInvalidName, "Invalid name")
	(eMausDataNotAvaliable, "MAUS Data not available")
	(eCommandBusy, "Command Busy")
	(eCommandAborting, "Local MAUS instance closing down")
	(eDSDFatalError, "DSD Fatal Error")

	(eUnknown, "Internal Error")
	(eIllegalOption, "Illegal option in this system configuration") 
	(eServerUnreachable, "AP MAUS instances unreachable")
	(eConfigServiceError, "CP name is not defined")
	(eNotImplemented, "Feature not yet implemented")

	(eFailToSetEpMask, "Cannot set endpoint mask")
	(eFailToGetEpMask, "Cannot get endpoint mask")
	(eFailToGetAfterSetEpMask, "After set endpoint mask, cannot get the mask back");
