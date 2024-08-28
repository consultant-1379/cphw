/*
 * MAUFunx.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: xdtthng
 */


#include "MAUFunx.h"
#include "ACS_CS_API.h"

using namespace std;

namespace MAUFunx
{
	static const char* s_dataPath = "/data/cphw/data/";
	static const char* s_fexIniFileName = "fex.ini";
}

string MAUFunx::getCphwDataPath()
{
	return string(s_dataPath);
}

string MAUFunx::getCphwFexIniFileName()
{
	return string(s_fexIniFileName);
}

int MAUFunx::getDualSidedCpCount()
{
	unsigned int cpCount;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getDoubleSidedCPCount(cpCount);

	return ACS_CS_API_NS::Result_Success == res ? cpCount : -1;
}


int MAUFunx::getMauType(int instance)
{
	//TRACE(("MAUFunx::getMauType() using CS API"));

	// This uses CS read function as opposed to reading from imm data base
	ACS_CS_API_CP* cp = ACS_CS_API::createCPInstance();
	if (!cp)
		return -1;

	CPID cpid = 1000 + instance;
	ACS_CS_API_NS::MauType type;
	ACS_CS_API_NS::CS_API_Result res = cp->getMauType (cpid, type);
	ACS_CS_API::deleteCPInstance(cp);
	//TRACE(("MAUFunx::getMauType() res <%d> mauType <%d>", res, type));
	return res == ACS_CS_API_NS::Result_Success? type : -1;
}

int MAUFunx::getNodeArchitecture()
{
	typedef ACS_CS_API_CommonBasedArchitecture Acs_ArchT;
	typedef Acs_ArchT::ArchitectureValue ArchValueT;

	//ACS_CS_API_CommonBasedArchitecture::ArchitectureValue infra;
	ArchValueT infra;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);

	return res == ACS_CS_API_NS::Result_Success ? infra : -1;
}

int MAUFunx::cpIdToCpName(int cpId, char* name)
{
	ACS_CS_API_Name  cpName;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getDefaultCPName(cpId, cpName);

	if (res != ACS_CS_API_NS::Result_Success){
		return res;
	}

	size_t len = cpName.length();
	res = cpName.getName(name, len);

	if (res != ACS_CS_API_NS::Result_Success){
		return res;
	}

	// This might be removed
	for (char *p = name; (*p = ::tolower(*p)); ++p) ;

	return 0;
}

int MAUFunx::cpNameToCpId(const char* cpname)
{
	ACS_CS_API_CP* cp = ACS_CS_API::createCPInstance();
	if (!cp)
		return -1;

	ACS_CS_API_Name name(cpname);
	CPID  cpid;

	ACS_CS_API_NS::CS_API_Result res = cp->getCPId(name, cpid);
	ACS_CS_API::deleteCPInstance(cp);

	if (res != ACS_CS_API_NS::Result_Success)
		return -1;

	return cpid;
}
