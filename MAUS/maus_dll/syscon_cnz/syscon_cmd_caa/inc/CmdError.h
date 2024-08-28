/*
 * CmdError.h
 *
 *  Created on: Nov 2, 2015
 *      Author: xdtthng
 */

#ifndef CMDERROR_H_
#define CMDERROR_H_

#include <string>
#include <map>

enum Cmd_Error
{
	eNoError          		= 0,
	eExecError          	= 1,	// Catch all exceptions in execute commannd
	eSyntaxError      		= 2, 	// Incorrect usage
	eMandatoryOption		= 3,	// -cp is mandatory in Multiple CP System
	eInvalidValue     		= 4,	// When an option value is out of range
	eClusterNotSupported	= 5,	// Cluster not supported
	eCSUnreachable			= 6,	// Configuration service is not reachable
	eSideMandatoryOption	= 7,	// -cp is mandatory in Multiple CP System
	eCpNameNotDefined		= 8,	// Cp name undefined

	eCommandBusy			= 65,	// Command Busy
	eCommandAborting		= 66,	// Local service closing down
	eDSDFatalError			= 67,	// DSD fatal error
	eLogFileDoesNotExist	= 68,	// Log file for syslogls does not exist

	eUnknown          		= 100,	// Catch all in exception statement at main()
	eIllcmdInSystemConfig	= 115,	// Illegal command in this system configuration
	eIllegalOption			= 116,	// Illegal option for a system confinguration
									// example, -cp cp1 is given in NCD
	eServerUnreachable		= 117,	// syscon server unreachable
	eConfigServiceError 	= 118,	// Error returned from Configuration Service (CS)
	eNotImplemented			= 119,	// Function not implemented

	eServerConfigError			= 201,
	eTermConnectionExsits		= 202,
	eSolIPAddressNotDefined		= 203,
	eBothNetDown				= 300,
	eNetworkUnstable			= 301,
	eSolServerNotContactable	= 302,
	eLostContactToSolServer		= 303,
	eFailedToGetGepVersion		= 304,
	eGepVersionNotSupported		= 305,

};

extern std::map<Cmd_Error, std::string> cmd_error_msg;

#endif /* CMDERROR_H_ */
