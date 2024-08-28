/*
 * MAUFunx.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: xdtthng
 */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>

#include "MAUFunx.h"
#include "FileFunx.h"
#include "Exception.h"
#include "ACS_CS_API.h"

#include "acs_prc_api.h"

#include <acs_apgcc_paramhandling.h>
#include <ACS_APGCC_CommonLib.h>
#include "maus_trace.h"
#include "maus_sha.h"
#include "ParseIni.h"
#include "TempDir.h"
#include "SwMauMom.h"

#include <iostream>

using namespace std;

namespace MAUFunx
{
	static const char* s_slotFileName = "/etc/opensaf/slot_id";
	static const char* s_msgQNameTag = "mcp";
	static const char* s_profileName = "/data/cphw/data/apmaus/maus.ini";

	static const char* s_libmauConfName[3] = {"unknown", "maus1.conf", "maus2.conf"};
	static const char* s_libmauConfSection[3] = {"new", "current", "backup"};
	static const char* s_libmauConfKey[3][2] =  {
		{"new-APNodeA", "new-APNodeB"},
		{"current-APNodeA", "current-APNodeB"},
		{"backup-APNodeA", "backup-APNodeB"}
	};

	static const char* s_mibsName[3] = {"unknown", "mibs1", "mibs2"};
	static const char* s_mibsPath[3] = {
		"unknown",
		"/cluster/storage/system/config/cphw-maus/apa/",
		"/cluster/storage/system/config/cphw-maus/apb/"
	};


	//static const char* s_libmauConfPath = "/cluster/storage/system/config/cphw/maus/";
	static const char* s_libmauConfPath = "/cluster/storage/system/config/cphw-maus/";
	static const char* s_libmauPath = "/opt/ap/cphw/lib64/";
};

int MAUFunx::getMibsName(string& mibsName, const string& libName)
{
	enum MibsName_Error
	{
		OK = 0,
		Cannot_open_libtxt_file = 1
	};

	const boost::regex regExpr("\\s*?MIB_NAME\\s*?=\\s*?(mibs[^\\s]+?\\.zip)\\s*");

	TRACE_DEBUG(("MAUFunx::getMibsName(), path <%s>", libName.c_str()));
	mibsName = "";

	ifstream file(libName.c_str());
	if (!file.good())
	{
		TRACE_DEBUG(("MAUFunx::getMibsName(), failed to open path <%s>", libName.c_str()));
		return Cannot_open_libtxt_file;
	}

	string line;
	boost::smatch m;
	while(getline(file, line))
	{
		if (!boost::regex_match(line, m, regExpr))
			continue;

		if (m[1].matched)
		{
			mibsName = m[1].str();
			break;
		}
	}
	TRACE_DEBUG(("MAUFunx::getMibsName(), mibName <%s> length <%d>", mibsName.c_str(), mibsName.length()));

	return OK;
}

int MAUFunx::installMibs(int instance, int slotId, int config)
{
	TRACE_DEBUG(("MAUFunx::installMibs(), instance <%d>, slotId <%d>", instance, slotId));

	string soPath;
	string soName = MAUFunx::getMausLibName(soPath, instance, slotId, config);

	string libPath = soPath;
	libPath += soName;
	libPath += ".txt";
	TRACE_DEBUG(("MAUFunx::installMibs() so full path <%s>", libPath.c_str()));

	// ------------------------------------------------------------
	// Create the path to mibs directory if it does not exist
	// Remove existing destination mibs directory
	// Check if the lib file has corresponding mibs
	// Unzip new mibs and rename to either mibs1 or mibs2
	// ------------------------------------------------------------

	// Create the path to mibs directory if it does not exist
	// ---------------------------------------------------
	if(FileFunx::fileExists(s_mibsPath[slotId]) == false)
	{
		if(mkdir(s_mibsPath[slotId], S_IRWXU | S_IRGRP | S_IROTH)!= 0)
		{
			TRACE_DEBUG(("MAUFunx::installMibs() failed to create dir <%s> with errno = %d ", s_mibsPath[slotId], errno));
			return -1;
		}
	}

	// Remove existing destination mibs directory if exist
	// ---------------------------------------------------
	string tmibsDir = s_mibsPath[slotId];
	tmibsDir += s_mibsName[instance];

	if (FileFunx::fileExists(tmibsDir.c_str()))
	{
		TRACE_DEBUG(("MAUFunx::installMibs() mibs dir <%s> exists", tmibsDir.c_str()));

		// Remove this dir
		if (FileFunx::rmDir(tmibsDir.c_str()) != 0)
		{
			TRACE_DEBUG(("MAUFunx::installMibs() failed to remove <%s>", tmibsDir.c_str()));
			return 1;
		}
		else
		{
			TRACE_DEBUG(("MAUFunx::installMibs() Ok to remove <%s>", tmibsDir.c_str()));
		}
	}
	else
	{
		TRACE_DEBUG(("MAUFunx::installMibs() no action, mibs dir <%s> does not exists", tmibsDir.c_str()));
	}

	// Check if the lib file has corresponding mibs
	// ---------------------------------------------
	string mibsName;
	getMibsName(mibsName, libPath);
	if (!mibsName.length())
	{
		TRACE_DEBUG(("MAUFunx::installMibs() no corresponding mibs, nothing to be done"));
		return 0;
	}
	size_t posZip = mibsName.find(".zip");
	string mibsUnzipName = mibsName.substr(0, posZip);
	TRACE_DEBUG(("MAUFunx::installMibs() mibsUnzipName <%s>", mibsUnzipName.c_str()));

	// Unzip new mibs and rename to either mibs1 or mibs2
	// --------------------------------------------------
	TempDir cur(soPath);
	string cmd = "unzip -q ";
	cmd += mibsName;
	cmd += " -d ";
	cmd += s_mibsPath[slotId];
	cmd += " >/dev/null 2>/dev/null";

	TRACE_DEBUG(("MAUFunx::installMibs() exec cmd <%s>", cmd.c_str()));

	int tres = system(cmd.c_str());

	int res = WIFEXITED(tres)? WEXITSTATUS(tres) : -1;
	TRACE_DEBUG(("MAUFunx::installMibs() unzip returns <%d>", res));
	if (res != 0)
	{
		TRACE_DEBUG(("MAUFunx::installMibs() failed to unzip <%s>", mibsName.c_str()));
		return 2;
	}

	// Change current directory to mibs path
	TempDir cur2(string(s_mibsPath[slotId]));

	if (rename(mibsUnzipName.c_str(), s_mibsName[instance]) != 0)
	{
		TRACE_DEBUG(("MAUFunx::installMibs() fail to rename unzip mibs file"));
	}

	TRACE_DEBUG(("MAUFunx::installMibs() returns"));
	return 0;
}


int MAUFunx::performChecksum(int instance, int slotId, int config)
{
	TRACE_DEBUG(("MAUFunx::performChecksum(), instance <%d>, slotId <%d>", instance, slotId));

	string soPath;
	string soName = MAUFunx::getMausLibName(soPath, instance, slotId, config);

	string lib = soPath;
	lib += soName;
	MausSha sha(soPath, soName);
	string storedHash;
	int res = sha.readHash(storedHash);
	TRACE_DEBUG(("MAUFunx::performChecksum() res value from readHash <%d>", res));
	if (res != 0)
	{
		TRACE_DEBUG(("MAUFunx::performChecksum() failed to read the stored hash line"));
		// should return 1, but return 0 for testing now
		return 1;
	}

	if (storedHash.length() != 64)
	{
		TRACE_DEBUG(("MAUFunx::performChecksum() length of hash  <%d>, which is not 64", storedHash.length()));
		// should return 1, but return 0 for testing now
		return 2;
	}

	string computedHash;

	// This is temporary measure for now
	// If cannot compute the hash, then just proceed
	res = sha.computeHash(computedHash);
	if (res != 0)
	{
		TRACE_DEBUG(("MAUFunx::performChecksum() failed to compute the hash value"));
		// should return 1, but return 0 for testing now
		return 3;
	}

	if (storedHash != computedHash)
	{
		TRACE_DEBUG(("MAUFunx::performChecksum() has mismatched, load cannot proceed"));
		return 4;
	}

	return 0;
}

int MAUFunx::getSysNo(int instance, int)
{
	int sysId = instance + 1000;

	ACS_CS_API_HWC* hwcTable = ACS_CS_API::createHWCInstance();
	if (!hwcTable)
		return -1;

	ACS_CS_API_BoardSearch* boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
	if (!boardSearch)
		return -2;

	boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_CPUB);
	boardSearch->setSysId(sysId);
	//boardSearch->setSide(side);
	ACS_CS_API_IdList boardList;
	uint16_t sysno;
	int res = (hwcTable->getBoardIds(boardList, boardSearch) == ACS_CS_API_NS::Result_Success)?
			(hwcTable->getSysNo(sysno, boardList[0]), sysno) : -3;

	ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	ACS_CS_API::deleteHWCInstance(hwcTable);
	return res;
}

string MAUFunx::getMausLibName(string& path, int instance, int slotId, int config)
{
	//TRACE_DEBUG(("MAUFunx::getMausLibName(%d, %d, %d", instance, slotId, config));

	string confFileName = s_libmauConfPath;
	confFileName += s_libmauConfName[instance];
	string section = s_libmauConfSection[config];
	string key = s_libmauConfKey[config][slotId - 1];

	//TRACE_DEBUG(("MAUFunx::getMausLibName(), section is <%s>", section.c_str()));
	//TRACE_DEBUG(("MAUFunx::getMausLibName(), key is <%s>", key.c_str()));
	//TRACE_DEBUG(("MAUFunx::getMausLibName(), confFileName is <%s>", confFileName.c_str()));

	string lib = ParseIni::getPrivateProfile(section.c_str(), key.c_str(), string("").c_str(), confFileName.c_str());

	//TRACE_DEBUG(("MAUFunx::getMausLibName(), lib is <%s>", lib.c_str()));
	path = s_libmauPath;
	return lib;
}


string MAUFunx::getMausLibName(int instance, int slotId, int config)
{
	//TRACE_DEBUG(("MAUFunx::getMausLibName(%d, %d, %d", instance, slotId, config));

	string confFileName = s_libmauConfPath;
	confFileName += s_libmauConfName[instance];
	string section = s_libmauConfSection[config];
	string key = s_libmauConfKey[config][slotId - 1];
	string retStr = s_libmauPath;

	//TRACE_DEBUG(("MAUFunx::getMausLibName(), section is <%s>", section.c_str()));
	//TRACE_DEBUG(("MAUFunx::getMausLibName(), key is <%s>", key.c_str()));
	//TRACE_DEBUG(("MAUFunx::getMausLibName(), confFileName is <%s>", confFileName.c_str()));

	string lib = ParseIni::getPrivateProfile(section.c_str(), key.c_str(), string("").c_str(), confFileName.c_str());

	//TRACE_DEBUG(("MAUFunx::getMausLibName(), lib is <%s>", lib.c_str()));

	return retStr + lib;
}

// slotId == 1 for AP-A, slotId == 2 for AP-B
int MAUFunx::getSlotId()
{
	static int s_slotId = -1;

	if (s_slotId == -1)
	{
		ifstream slotFile(s_slotFileName);

		if (slotFile.good()) {
			int slot = -1;
			slotFile >> slot;
			s_slotId = slotFile.good() ? slot : -1;
		}
		slotFile.close();
	}
	return s_slotId;
}

int MAUFunx::getNodeArchitecture()
{
	typedef ACS_CS_API_CommonBasedArchitecture Acs_ArchT;
	typedef Acs_ArchT::ArchitectureValue ArchValueT;

	//ACS_CS_API_CommonBasedArchitecture::ArchitectureValue infra;
	ArchValueT infra;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);
	if (res != ACS_CS_API_NS::Result_Success)
		THROW_XCODE(CodeException::CE_CSUNREACHABLE);

	return infra;
}

bool MAUFunx::isMultipleCP() {

	bool multCPSys;

	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::isMultipleCPSystem(multCPSys);
	if (res != ACS_CS_API_NS::Result_Success)
		THROW_XCODE(CodeException::CE_CSUNREACHABLE);

	return multCPSys;
}

int MAUFunx::cpNameToCpId(const char* cpname)
{
	ACS_CS_API_CP* cp = ACS_CS_API::createCPInstance();
	if (!cp)
		THROW_XCODE(CodeException::CE_CSUNREACHABLE);

	ACS_CS_API_Name name(cpname);
	CPID  cpid;

	ACS_CS_API_NS::CS_API_Result res = cp->getCPId(name, cpid);
	ACS_CS_API::deleteCPInstance(cp);

	if (res != ACS_CS_API_NS::Result_Success)
		THROW_XCODE(CodeException::CE_CS_CPNAME_UNDEFINED);

	return cpid;
}

int MAUFunx::getDualSidedCpCount()
{
	unsigned int cpCount;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getDoubleSidedCPCount(cpCount);
	if (ACS_CS_API_NS::Result_Success != res)
		THROW_XCODE(CodeException::CE_CSUNREACHABLE);

	return cpCount;
}

#if 0

std::string MAUFunx::getPrivateProfile(const char* section, const char* key,
		const char* defaultStr, const char* filename)
{
	//TRACE_DEBUG(("MAUFunx::getPrivateProfile()"));

#if 1

	return ParseIni::getPrivateProfile(section, key, defaultStr, filename);

#else

    using boost::property_tree::ptree;

    ptree pt;
    try {
        read_ini(filename, pt);
    }
    catch (exception& e) {
    	TRACE_DEBUG(("MAUFunx::getPrivateProfile() failed to open file <%s> error <%s>", filename, e.what()));
        return string(defaultStr);
    }
    std::string keystr = section;
    keystr += ".";
    keystr += key;

    boost::optional<string> value = pt.get_optional<string>(keystr);

    return value? *value : string(defaultStr);

#endif

}

int MAUFunx::getPrivateProfile(const char* section, const char* key, int default_val, const char* filename)
{

#if 1

	return ParseIni::getPrivateProfile(section, key, default_val, filename);

#else

    using boost::property_tree::ptree;

    ptree pt;
    try {
        read_ini(filename, pt);

    }
    catch (exception& e) {
    	//cout << "exception from reading ini entry " << e.what() << endl;
        return default_val;
    }
    catch (...) {
        return default_val;
    }

    std::string keystr = section;
    keystr += ".";
    keystr += key;
    boost::optional<int> value = pt.get_optional<int>(keystr);

    return value? *value : default_val;

#endif

}

#endif

int MAUFunx::getMauType(int instance)
{
#if 1

	//TRACE_DEBUG(("MAUFunx::getMauType() using CS API"));

	// This uses CS read function as opposed to reading from imm data base
	ACS_CS_API_CP* cp = ACS_CS_API::createCPInstance();
	if (!cp)
		return -1;

	CPID cpid = 1000 + instance;
	ACS_CS_API_NS::MauType type;
	ACS_CS_API_NS::CS_API_Result res = cp->getMauType (cpid, type);
	ACS_CS_API::deleteCPInstance(cp);
	//TRACE_DEBUG(("MAUFunx::getMauType() res <%d> mauType <%d>", res, type));
	return res == ACS_CS_API_NS::Result_Success? type : -1;

#else

	// This is reading off imm data base
	int mautype = -1;
	OmHandler omHandler;

	try
	{
		string cpName = instance == 1 ? "CP1" : "CP2";

		ACS_CC_ReturnType result = omHandler.Init();

		if (result != ACS_CC_SUCCESS) {
			//cout << "cannot start object handler" << endl;
			return -1;
		}

		ACS_APGCC_ImmAttribute mauTypeAttr;
		mauTypeAttr.attrName = "mauType";
		vector<ACS_APGCC_ImmAttribute *> attributes;
		attributes.push_back(&mauTypeAttr);

		// "dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1"
		string dn = "dualSidedCpId=";
		dn += cpName;
		dn += ",logicalMgmtId=1,AxeEquipmentequipmentMId=1";

		result = omHandler.getAttribute(dn.c_str(), attributes);
		if (result != ACS_CC_SUCCESS)
		{
			//cout << "there is something wrong" << endl;
			omHandler.Finalize();
			return -1;
		}

		mautype = *((int*)mauTypeAttr.attrValues[0]);
		//cout << "mau type is " << mautype << endl;
	}
	catch (...) {
		// Do nothing here
		mautype = -1;
	}
	omHandler.Finalize();
	return mautype;

#endif
}


int MAUFunx::getSwMauAdminState(int instance)
{

	int attrValue = -1;
	OmHandler omHandler;

	try
	{
		ACS_CC_ReturnType result = omHandler.Init();

		if (result != ACS_CC_SUCCESS) {
			//cout << "cannot start object handler" << endl;
			return -1;
		}

		vector<string> dnList;
		result  = omHandler.getClassInstances(SwMauMom::immCln_EquipmentSwMau.c_str(), dnList);
		if (result != ACS_CC_SUCCESS)
		{
			return -1;
		}

		SwMauMom::CpTypeT cpType;
		ACS_APGCC_ImmAttribute adminStateAttr;
		vector<ACS_APGCC_ImmAttribute *> attributes;
		for (unsigned int i = 0; i < dnList.size(); ++i)
		{
			cpType = SwMauMom::getCpType(dnList[i]);
			adminStateAttr.attrName = SwMauMom::immAttr_administrativeState;
			vector<ACS_APGCC_ImmAttribute *>().swap(attributes);
			attributes.push_back(&adminStateAttr);
			if (instance == cpType)
			{
				result = omHandler.getAttribute(dnList[i].c_str(), attributes);
				if (result == ACS_CC_SUCCESS)
				{
					//cout << "there is something wrong" << endl;
					//m_omHandler.Finalize();
					attrValue = *((int*)adminStateAttr.attrValues[0]);
					break;
				}
			}
		}


	}
	catch (...) {
		// Do nothing here
		attrValue = -1;
	}

	omHandler.Finalize();
	return attrValue;

}


#if 0

int MAUFunx::getCpMauMAttribute(int instance, const string& name)
{
	int attrValue = -1;
	OmHandler omHandler;

	try
	{
		string cpName = instance == 1 ? "CP1" : "CP2";

		ACS_CC_ReturnType result = omHandler.Init();

		if (result != ACS_CC_SUCCESS) {
			//cout << "cannot start object handler" << endl;
			return -1;
		}

		ACS_APGCC_ImmAttribute adminStateAttr;
		adminStateAttr.attrName = name;
		vector<ACS_APGCC_ImmAttribute *> attributes;
		attributes.push_back(&adminStateAttr);

		// "dualSidedCpId=CP1,AxeMauManagementcpMauMId=1"
		string dn = "dualSidedCpId=";
		dn += cpName;
		dn += ",AxeMauManagementcpMauMId=1";

		result = omHandler.getAttribute(dn.c_str(), attributes);
		if (result != ACS_CC_SUCCESS)
		{
			//cout << "there is something wrong" << endl;
			omHandler.Finalize();
			return -1;
		}

		attrValue = *((int*)adminStateAttr.attrValues[0]);
		//cout << "mau type is " << mautype << endl;
	}
	catch (...) {
		// Do nothing here
		attrValue = -1;
	}
	omHandler.Finalize();
	return attrValue;
}

int MAUFunx::getCpMauMAdminState(int instance)
{
	static const string name = "administrativeState";
	return getCpMauMAttribute(instance, name);
}


int MAUFunx::getCpMauMFCState(int instance)
{
	static const string name = "functionChangeState";
	return getCpMauMAttribute(instance, name);
}



int MAUFunx::getCpMauMFexRequest(int instance)
{
	static const string name = "fexRequest";
	return getCpMauMAttribute(instance, name);

}

#endif

//
//
//===========================================================================
//  Parameter   cpAndProtocolType (old name: ACS_APCONFBIN_CpAndProtocolType)
//  Format  unsignedInt
//
//  Contains information about the CP type the AP is connected to and what
//  communication protocol to use on the processor test bus when communicating
//  with the CP
//
//           Value   CP type       Protocol
//           -----   -------       --------
//           0       APZ 212 3x    SDLC
//           1       APZ 212 3x    TCP/IP
//           2       APZ 212 40    TCP/IP
//           3       APZ 212 50    TCP/IP
//           4       APZ 212 55    TCP/IP
//           4       MSC-S BC SPX  TCP/IP
//
//	Return value
//  -1	Error
//	0	Not classic CP
//	1	Classic CP for all value less than 4
//
int MAUFunx::getClassicCp()
{
	int classicCp = -1;
	OmHandler omHandler;
	try
	{
		ACS_CC_ReturnType result = omHandler.Init();

		if (result != ACS_CC_SUCCESS) {
			return -1;
		}

	    std::vector<std::string> dnList;
	    result = omHandler.getClassInstances("AxeFunctions", dnList);

	    if (result != ACS_CC_SUCCESS)
	    {
	    	omHandler.Finalize();
	        return -1;
	    }
	    acs_apgcc_paramhandling phaObject;
	    const char *pAttrName = "apzProtocolType";
	    int apzProtocolTypeValue = 0;

	    result = phaObject.getParameter(dnList[0], pAttrName, &apzProtocolTypeValue);
	    if (result != ACS_CC_SUCCESS)
	    {
	    	omHandler.Finalize();
	        return -1;
	    }
	    classicCp = apzProtocolTypeValue < 4;
	}
	catch (...)
	{
		classicCp = -1;
	}
	omHandler.Finalize();
	return classicCp;
}

int MAUFunx::getNodeState()
{
	int nodeState = -1;
	try
	{
		// ACS_PRC_API returns
		// 1 active
		// 2 passive
		ACS_PRC_API prcApi;
		nodeState = prcApi.askForNodeState();
	}
	catch (...)
	{
		nodeState = -1;
	}

	return nodeState;
}


const char* MAUFunx::getMsgQNameTag()
{
	return s_msgQNameTag;
}

int MAUFunx::isSiOptionUsed()
{
#if 0
	return ParseIni::getPrivateProfile("Command", "SiOpt", 0, s_profileName);
#else
	return 1;
#endif
}

int MAUFunx::isViewOptionUsed()
{
	return ParseIni::getPrivateProfile("Command", "ViewOpt", 0, s_profileName);
}
