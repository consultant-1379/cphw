/*
 * MAUFunx.h
 *
 *  Created on: Jul 13, 2014
 *      Author: xdtthng
 */

#ifndef MAUFUNX_H_
#define MAUFUNX_H_

#include <string>

namespace MAUFunx
{
	// Used for CpMauM
	int getDualSidedCpCount();
	int getMauType(int instance);
	int getNodeArchitecture();
	int cpIdToCpName(int cpId, char* name);
	int cpNameToCpId(const char*);

	std::string getCphwDataPath();
	std::string getCphwFexIniFileName();

};

#endif /* MAUFUNX_H_ */
