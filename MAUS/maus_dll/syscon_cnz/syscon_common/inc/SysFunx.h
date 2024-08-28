/*
 * SysFunx.h
 *
 *  Created on: Nov 2, 2015
 *      Author: xdtthng
 */

#ifndef SYSFUNX_H_
#define SYSFUNX_H_

#include <string>

namespace SysFunx
{
	int isMultipleCP();
	int cpNameToCpId(const char* cpname);
	int cpIdToCpName(int cpId, std::string& name);
	std::string getLogPath(int cpId, int side, int multiCpSystem);
	int getNodeArchitecture();
};

#endif /* SYSFUNX_H_ */
