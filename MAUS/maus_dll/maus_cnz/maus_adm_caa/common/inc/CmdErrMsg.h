/*
 * CmdErrMsg.h
 *
 *  Created on: Jul 13, 2014
 *      Author: xdtthng
 */

#ifndef CMDERRMSG_H_
#define CMDERRMSG_H_

#include <string>
#include <map>

enum Cmd_Error_Code
{
	eNoError          		= 0,
	eExecError          	= 1,	// Catch all exceptions in execute commannd
	eSyntaxError      		= 2, 	// Incorrect usage
	eMandatoryOption		= 3,	// -cp is mandatory in Multiple CP System
	eInvalidValue     		= 4,	// When an option value is out of range
	eClusterNotSupported	= 5,	// Cluster not supported
	eCSUnreachable			= 6,	// Configuration service is not reachable
	eMausNotSupported		= 7,	// Maus not supported
	eCpNameNotDefined		= 8,	// Cp name undefined

	eBMsgQCreate			= 50,	// Cannot create boost message queue
	eEmptyMsg				= 51,	// Zero byte received while expecting a message
	eCp1orCp2Supported		= 52,	// Only CP1 or CP2 is supported
	eOmHandlerFault			= 53,	// OM Handler faulty
	eAllInstancesLocked		= 54,	// All instances already locked
	eLockingFailed			= 55,	// Locking service instance failed
	eLockingOneOK			= 56, 	// Locking 2nd service instance failed, the first instance ok
	eAllInstancesUnlocked	= 57,	// All instances already unclocked
	eUnlockingFailed		= 58,	// Unlocking service instance failed
	eUnlockingOneOK			= 59,	// Unlocking 2nd service instance failed, the first instance ok
	eUserAbort				= 60,	// User Abort
	eInstanceLocked			= 61,	// Specific instance already locked
	eInstanceUnlocked		= 62,	// Specific instance already unlocked
	eInvalidName			= 63,	// Invalid string optarg
	eMausDataNotAvaliable	= 64,	// Cannot get connectivity view
	eCommandBusy			= 65,	// Command Busy
	eCommandAborting		= 66,	// Local MAUS instance closing down
	eDSDFatalError			= 67,	// DSD fatal error

	eUnknown          		= 100,	// Catch all in exception statement at main()
	eIllegalOption			= 116,	// Illegal option for a system confinguration
									// example, -cp cp1 is given in NCD
	eServerUnreachable		= 117,	// Local MAUS instance unreachable
	eConfigServiceError 	= 118,	// Error returned from Configuration Service (CS)
	eNotImplemented			= 119,	// Function not implemented
	
	eFailToSetEpMask		= 902,	// Cannot set endpoint mask
	eFailToGetEpMask		= 903,	// Cannot get endpoint mask
	eFailToGetAfterSetEpMask= 904, 	// Cannot get endpoint mask after setting
	eInconsistentGetSetEpMask		= 905	// Incosnstent set, get endpoint mask
};

extern std::map<Cmd_Error_Code, std::string> cmd_error_msg;

#endif // CMDERRMSG_H_
