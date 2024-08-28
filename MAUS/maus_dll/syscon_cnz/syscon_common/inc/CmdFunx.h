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
	int execCmd(const std::string& cmd, const std::string& dn);
	int execCmd(const std::string& cmd, std::vector<std::string>& result);
}

#endif /* CMDFUNX_H_ */
