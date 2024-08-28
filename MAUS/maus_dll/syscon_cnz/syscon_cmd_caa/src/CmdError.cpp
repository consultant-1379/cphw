/*
 * CmdError.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: xdtthng
 */

#include "CmdError.h"
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::map<Cmd_Error, std::string> cmd_error_msg = boost::assign::map_list_of
	(eNoError, "Executed.")
	(eExecError, "Error when executing (General fault).")
	(eSyntaxError, "Incorrect usage.")
	(eMandatoryOption, "Illegal option usage in this system configuration:\n"
			"CP name must be specified for a multiple CP system.")
	(eInvalidValue, "Invalid value")
	(eClusterNotSupported, "Cluster is not supported.")
	(eCSUnreachable, "Configuration service is not reachable.")
	(eSideMandatoryOption, "Option -s is mandatory for this CP system.")

	(eCpNameNotDefined, "CP name is not defined.")

	(eCommandBusy, "Command Busy.")
	(eCommandAborting, "Local MAUS instance closing down.")
	(eDSDFatalError, "DSD Fatal Error.")
	(eLogFileDoesNotExist, "Log file does not exist.")

	(eUnknown, "Internal Error.")

	(eIllcmdInSystemConfig, "Illegal command in this system configuration.")
	(eIllegalOption, "Illegal option usage in this system configuration:\n"
			"CP name must not be specified for a one CP system.")
	(eServerUnreachable, "This command must be executed in the Active node.")
	(eConfigServiceError, "CP name is not defined.")
	(eNotImplemented, "Feature not yet implemented.")

	(eServerConfigError, "Input configuration error.")
	(eTermConnectionExsits, "Syscon already connected to the same CPUB.")
	(eSolIPAddressNotDefined, "SOL IP address is not defined for this CP.")
	(eBothNetDown, "Both CPUB subnets are down.")
	(eSolServerNotContactable, "Sol Server not contactable via both networks.")
	(eLostContactToSolServer, "Lost contact to Sol Server.")
	(eFailedToGetGepVersion, "Failed to get CPUB version.")
	(eGepVersionNotSupported, "CPUB version is not supported.")
	;
