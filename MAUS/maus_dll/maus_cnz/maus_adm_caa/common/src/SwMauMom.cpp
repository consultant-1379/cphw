/*
 * SwMauMom.cpp
 *
 *  Created on: Mar 27, 2016
 *      Author: xdtthng
 */

#include "boost/assign/list_of.hpp"
#include "boost/algorithm/string.hpp"

#include "acs_apgcc_omhandler.h"
#include "SwMauMom.h"
#include "MAUFunx.h"
#include "maus_trace.h"

using namespace std;

// immCln	imm class name
// immImpl	imm object implementer
const string SwMauMom::immImpl_OiName 				= "CPHW_MAUSC_SwMau_OI";
const string SwMauMom::immCln_EquipmentSwMau 		= "AxeEquipmentSwMau";
const string SwMauMom::immCln_EquipmentDualSidedCp 	= "AxeEquipmentDualSidedCp";

const string SwMauMom::immAttr_dualSidedCpId		= "dualSidedCpId";
const string SwMauMom::immAttr_swMauId				= "swMauId";
const string SwMauMom::s_swMauInstance				= "swMauId=1";

const string SwMauMom::immAttr_administrativeState 	= "administrativeState";
const string SwMauMom::immAttr_operationalState 	= "operationalState";

map<SwMauMom::CpTypeT, std::string> SwMauMom::s_cpInstances = boost::assign::map_list_of
    (SwMauMom::Ncd, SwMauMom::immAttr_dualSidedCpId + "=Dual_Sided_Cp")
    (SwMauMom::Cp1, SwMauMom::immAttr_dualSidedCpId + "=CP1")
    (SwMauMom::Cp2, SwMauMom::immAttr_dualSidedCpId + "=CP2")
    (SwMauMom::CpInvalid, "");


map<SwMauMom::CpTypeT, std::string> SwMauMom::s_cpNames = boost::assign::map_list_of
    (SwMauMom::Ncd, "cp1")
    (SwMauMom::Cp1, "cp1")
    (SwMauMom::Cp2, "cp2")
    (SwMauMom::CpInvalid, "");


const string& SwMauMom::getCpInstance(CpTypeT type)
{
	return s_cpInstances[type];
}

const string& SwMauMom::getCpName(CpTypeT type)
{
	return s_cpNames[type];
}


#if 0

string SwMauMom::getCpDn(CpTypeT cpType)
{

	string cpDn = SwMauMom::getCpInstance(cpType);
	cpDn += ",";
	cpDn += SwMauMom::getParentInstance();
	return cpDn;
}

#endif


SwMauMom::CpTypeT SwMauMom::getCpType(const std::string& cpDn, int position)
{
	SwMauMom::CpTypeT cpType = SwMauMom::CpInvalid;

    vector<string> fields;
    boost::algorithm::split(fields, cpDn, boost::is_any_of(","));

    // Convert objectName to cpType
    string cpInstance(fields[position]);
    map<CpTypeT, string>::iterator it;
    for (it = SwMauMom::s_cpInstances.begin(); it != SwMauMom::s_cpInstances.end(); ++it)
    {
        if ((*it).second == cpInstance)
        {
            cpType = (*it).first;
            break;
        }
    }

    return cpType;

}

bool SwMauMom::getSwMauDn(vector<string>& dnList)
{
    OmHandler immHandle;

    ACS_CC_ReturnType ret = immHandle.Init();
    if (ret != ACS_CC_SUCCESS)
    {
        cout << "getSwMauDn() Init OMHandler failed with error code "
        		<< immHandle.getInternalLastError() << " error text "
                << immHandle.getInternalLastErrorText() << endl;

        return false;
    }

    ret = immHandle.getClassInstances(immCln_EquipmentSwMau.c_str(), dnList);

    if (ret != ACS_CC_SUCCESS)
    {
        cout << "getSwMauDn() get class instance failed with error code "
        		<< immHandle.getInternalLastError() << " error text "
                << immHandle.getInternalLastErrorText() << endl;
    	TRACE(("getSwMauDn() failed <%d> <%s>", immHandle.getInternalLastError(), immHandle.getInternalLastErrorText()));
    	return false;
    }

    return true;
}


#if 0

const string& SwMauMom::getParentInstance()
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
        cout << "getParentDn() Init OMHandler failed with error code "
        		<< immHandle.getInternalLastError() << " error text "
                << immHandle.getInternalLastErrorText() << endl;

        return parentInstance;
    }

    ret = immHandle.getClassInstances(immCln_EquipmentSwMau.c_str(), dnList);

    if (ret != ACS_CC_SUCCESS)
    {
        cout << "getParentDn() get class instance failed with error code "
        		<< immHandle.getInternalLastError() << " error text "
                << immHandle.getInternalLastErrorText() << endl;
    	TRACE(("getParentDn() failed <%d> <%s>", immHandle.getInternalLastError(), immHandle.getInternalLastErrorText()));
    }
    else
    {
        contactImm = true;
        parentInstance = dnList[0];
        cout << "size of dn list is " << dnList.size() << endl;
        cout << "MausModel::getParentDn() returns Parent DN <" << dnList[0] << "> " << endl;
        cout << "MausModel::getParentDn() returns Parent DN size <" << dnList.size() << "> " << endl;
        TRACE(("getParentDn() ok, dnList size <%d>", dnList.size()));
    }

    immHandle.Finalize();

    return parentInstance;

}

#endif


