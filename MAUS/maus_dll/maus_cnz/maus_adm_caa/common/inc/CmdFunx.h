/*
 * CmdFunx.h
 *
 *  Created on: Jul 25, 2014
 *      Author: xdtthng
 */

#ifndef CMDFUNX_H_
#define CMDFUNX_H_

#include <string>
#include <vector>

namespace CmdFunx
{
	std::string getDn(const std::string& sname, int si);
	int isLocked(const std::string& dn);
	int execCmd(const std::string& cmd, const std::string& dn);
	int execCmd(const std::string& cmd, std::vector<std::string>& result);
	int lockSi(const std::string& dn);
	int unLockSi(const std::string& dn);
	bool confirm(const std::string& prompt);
	int getAmfSuSiAssignment(int siAss[2], const std::string& siName);
}

#endif /* CMDFUNX_H_ */
