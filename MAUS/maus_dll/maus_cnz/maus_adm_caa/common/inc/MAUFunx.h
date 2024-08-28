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
	bool isMultipleCP();
	int cpNameToCpId(const char*);
	int getDualSidedCpCount();
	int getNodeArchitecture();
	int getClassicCp();
	const char* getMsgQNameTag();

	//int getPrivateProfile(const char* section, const char* key, int default_val, const char* filename);
	//std::string getPrivateProfile(const char* section, const char* key, const char* defaultStr, const char* filename);

	int isSiOptionUsed();
	int isViewOptionUsed();

	int getMauType(int instance);

	// From old CpMauM
	// ----------------
	//int getCpMauMAdminState(int instance);
	//int getCpMauMFCState(int instance);
	//int getCpMauMFexRequest(int instance);
	//int getCpMauMAttribute(int instance, const std::string& name);

	// The new SwMau
	// -------------
	int getSwMauAdminState(int instance);


	int getNodeState();
	int getSlotId();
	int getSysNo(int instance, int slotId);
	int performChecksum(int instance, int slotId, int config = 1);
	int installMibs(int instance, int slotId, int config = 1);
	int getMibsName(std::string& mibsName, const std::string& path);

	std::string getMausLibName(int instance, int slotId, int config = 1);
	std::string getMausLibName(std::string& path, int instance, int slotId, int config = 1);
};


#endif /* MAUFUNX_H_ */
