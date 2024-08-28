/*
 * SysFunx.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: xdtthng
 */

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include "SysFunx.h"
#include "ACS_CS_API.h"

using namespace std;

namespace SysFunx
{

string s_rootPath = "/data/cphw/logs/syscon/";
string s_logFileName = "syslog.log";
string s_side[2] = {"cpa", "cpb"};

map<int, string> s_cpName = boost::assign::map_list_of
		(1001, "cp1")
		(1002, "cp2")
		(0xffff, "cp1")
		;

};

int SysFunx::isMultipleCP()
{

	bool multCPSys = false;

	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::isMultipleCPSystem(multCPSys);

	return res == ACS_CS_API_NS::Result_Success? multCPSys : -1;
}

int SysFunx::cpIdToCpName(int cpId, string& name)
{
	ACS_CS_API_Name  cpName;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getDefaultCPName(cpId, cpName);

	if (res != ACS_CS_API_NS::Result_Success){
		return res;
	}

	char nameStr[15];

	size_t len = cpName.length();
	res = cpName.getName(nameStr, len);

	if (res != ACS_CS_API_NS::Result_Success){
		return res;
	}

	// This might be removed
	for (char *p = nameStr; (*p = ::tolower(*p)); ++p) ;
	name = nameStr;

	return 0;

}

int SysFunx::cpNameToCpId(const char* cpname)
{
	ACS_CS_API_CP* cp = ACS_CS_API::createCPInstance();
	if (!cp)
		return -1;

	ACS_CS_API_Name name(cpname);
	CPID  cpid;

	ACS_CS_API_NS::CS_API_Result res = cp->getCPId(name, cpid);
	ACS_CS_API::deleteCPInstance(cp);

	//if (res != ACS_CS_API_NS::Result_Success)
	//	THROW_XCODE(CodeException::CE_CS_CPNAME_UNDEFINED);
	//return cpid;

	return res == ACS_CS_API_NS::Result_Success? cpid : -1;
}

string SysFunx::getLogPath(int cpId, int side, int multiCpSystem)
{
	static const char s_fileDelim = '/';

	(void) multiCpSystem;

	string path = s_rootPath;
	path += s_cpName[cpId];
	path += s_fileDelim;
	path += s_side[side];
	path += s_fileDelim;
	path += s_logFileName;

	return path;
}

int SysFunx::getNodeArchitecture()
{
	typedef ACS_CS_API_CommonBasedArchitecture Acs_ArchT;
	typedef Acs_ArchT::ArchitectureValue ArchValueT;

	//ACS_CS_API_CommonBasedArchitecture::ArchitectureValue infra;
	ArchValueT infra;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);

	return res == ACS_CS_API_NS::Result_Success? infra : -1;
}

