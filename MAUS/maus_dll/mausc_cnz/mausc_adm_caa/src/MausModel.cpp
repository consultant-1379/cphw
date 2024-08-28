/*
 * MausModel.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: xdtthng
 */

#include "boost/assign/list_of.hpp"
#include "boost/algorithm/string.hpp"

#include "acs_apgcc_omhandler.h"
#include "MausModel.h"
#include "MAUFunx.h"

using namespace std;

// immCln	imm class name
// immImpl	imm object implementer
const string MausModel::immImpl_OiName 				= "CPHW_MAUSC_CpMau_OI";
const string MausModel::immCln_CpMauM 				= "AxeMauManagementCpMauM";

//const string MausModel::immCln_CpMau 				= "AxeMauManagementCpMau";
const string MausModel::immCln_DualSidedCp			= "AxeMauManagementDualSidedCp";
//const string MausModel::immAttr_cpMauId 			= "cpMauId";
const string MausModel::immAttr_dualSidedCpId		= "dualSidedCpId";

const string MausModel::immAttr_operationalState 	= "operationalState";
//const string MausModel::immAttr_operationState 		= "operationState";

const string MausModel::immAttr_changeState 		= "functionChangeState";
const string MausModel::immAttr_administrativeState = "administrativeState";
const string MausModel::immAttr_fexRequest 			= "fexRequest";

map<MausModel::CpTypeT, std::string> MausModel::s_cpInstances = boost::assign::map_list_of
    (MausModel::Ncd, MausModel::immAttr_dualSidedCpId + "=Dual_Sided_Cp")
    (MausModel::Cp1, MausModel::immAttr_dualSidedCpId + "=CP1")
    (MausModel::Cp2, MausModel::immAttr_dualSidedCpId + "=CP2")
    (MausModel::CpInvalid, "");


map<MausModel::CpTypeT, std::string> MausModel::s_cpNames = boost::assign::map_list_of
    (MausModel::Ncd, "cp1")
    (MausModel::Cp1, "cp1")
    (MausModel::Cp2, "cp2")
    (MausModel::CpInvalid, "");


const string& MausModel::getCpInstance(CpTypeT type)
{
	return s_cpInstances[type];
}

const string& MausModel::getCpName(CpTypeT type)
{
	return s_cpNames[type];
}

string MausModel::getFexIniFileName(CpTypeT type)
{
	string dataPath = MAUFunx::getCphwDataPath();
	dataPath += MausModel::getCpName(type);
	dataPath += '/';
	dataPath += MAUFunx::getCphwFexIniFileName();
	return dataPath;
}

string MausModel::getFexIniFileName(CpTypeT type, string& path)
{
	string dataPath = MAUFunx::getCphwDataPath();
	dataPath += MausModel::getCpName(type);
	dataPath += '/';
	path = dataPath;
	dataPath += MAUFunx::getCphwFexIniFileName();
	return dataPath;
}

string MausModel::getCpDn(CpTypeT cpType)
{

	string cpDn = MausModel::getCpInstance(cpType);
	cpDn += ",";
	cpDn += MausModel::getParentInstance();
	return cpDn;
}

const string& MausModel::getParentInstance()
{
	static bool contactImm	= false;
	static string parentInstance = "";

	if (contactImm)
		return parentInstance;

    OmHandler immHandle;
    ACS_CC_ReturnType ret;
    vector<string> dnList;

    ret = immHandle.Init();
    if (ret != ACS_CC_SUCCESS)
    {
        //cout << "getParentDn() Init OMHandler failed with error code "
        //		<< immHandle.getInternalLastError() << " error text "
        //        << immHandle.getInternalLastErrorText() << endl;

        return parentInstance;
    }

    ret = immHandle.getClassInstances(immCln_CpMauM.c_str(), dnList);

    if (ret != ACS_CC_SUCCESS)
    {
        //cout << "getParentDn() get class instance failed with error code "
        //		<< immHandle.getInternalLastError() << " error text "
        //        << immHandle.getInternalLastErrorText() << endl;
    }
    else
    {
        contactImm = true;
        parentInstance = dnList[0];
        //cout << "MausModel::getParentDn() returns Parent DN <" << dnList[0] << "> " << endl;
    }

    immHandle.Finalize();

    return parentInstance;

}

MausModel::CpTypeT MausModel::getCpType(const std::string& cpDn)
{
	MausModel::CpTypeT cpType = MausModel::CpInvalid;

    // cpDn will have this form "dualSidedCpId=CP1,AxeMauManagementcpMauMId=1" or other form
    // Just take the string before the first "," for the next parsing
    // If there is no "," , will take the original objName for the next parsing
    vector<string> fields;
    boost::algorithm::split(fields, cpDn, boost::is_any_of(","));

    //cout << "MausModel::getCpType split ok; number of fields: " << fields.size() << endl;
    //for (int i = 0; i < fields.size(); ++i)
    //	cout << "fields[" << i << "] = " << fields[i] << endl;

    // Convert objectName to cpType
    string cpInstance(fields[0]);
    map<CpTypeT, string>::iterator it;
    for (it = MausModel::s_cpInstances.begin(); it != MausModel::s_cpInstances.end(); ++it)
    {
        if ((*it).second == cpInstance)
        {
            cpType = (*it).first;
            break;
        }
    }

    return cpType;

}
