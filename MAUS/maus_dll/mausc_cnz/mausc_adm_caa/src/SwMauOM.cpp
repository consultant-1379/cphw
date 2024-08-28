/*
 * SwMauOM.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: xdtthng
 */

#include <vector>
#include <string>

// Remove later
#include <iostream>

#include "SwMauOM.h"
#include "MAUFunx.h"
#include "FileFunx.h"
#include "maus_trace.h"
#include "Maus_AehEvent.h"
#include "ParseIni.h"
#include "CpMauM.h"
#include <ACS_CS_API.h>

//#include "acs_apgcc_oihandler_V3.h"

using namespace std;

SwMauOM::SwMauOM(): m_initialized(false), m_omHandler(),
		m_lastErrorCode(0),
		m_lastErrorText("")

{
	TRACE(("SwMauOM::SwMauOM() ctor"));
	(void) this->init();
	TRACE(("SwMauOM::SwMauOM() returns"));
}

bool SwMauOM::init()
{
	TRACE(("SwMauOM::init()"));
    m_initialized = false;

    ACS_CC_ReturnType result = m_omHandler.Init();

    if (result == ACS_CC_SUCCESS)
    {
        m_initialized = true;
    }
    else
    {
        m_lastErrorCode = m_omHandler.getInternalLastError();
        m_lastErrorText = m_omHandler.getInternalLastErrorText();
        TRACE_ERROR(("SwMauOM::init() failed to get OmHandler, error code <%d>, error text <%s>, "
        		"m_lastErrorCode, m_lastErrorText.c_str()"));
    }
    TRACE(("SwMauOM::init() returns <%d>", m_initialized));
    return m_initialized;

}

SwMauOM::~SwMauOM()
{
	TRACE(("SwMauOM::~SwMauOM() dtor"));

    if (m_initialized)
    {
        m_omHandler.Finalize();
    }

    m_initialized = false;
    TRACE(("SwMauOM::~SwMauOM() return"));
}

#if 0

bool SwMauOM::createObject(const string& parent)
{
	bool exist;
	int admin;
	return createObject(parent, exist, admin);
}

#endif

bool SwMauOM::createObject(const string& parent, bool& objExist, int& adminState)
{

	TRACE(("SwMauOM::createObject(parent, objExist)"));
	objExist = false;
	adminState = -1;

    int numAttr = 2;
    vector<ACS_CC_ValuesDefinitionType> attrList;
    ACS_CC_ValuesDefinitionType attribute[numAttr];

    //string swMauInstacne = "swMauId=1";
    //string immAttr = "swMauId";

    int index = 0;
    //attribute[index].attrName = (char*)immAttr.c_str();
    attribute[index].attrName = (char*)SwMauMom::immAttr_swMauId.c_str();
    attribute[index].attrType = ATTR_STRINGT;
    attribute[index].attrValuesNum = 1;
    //void* valueInstanceId[1] = {(char*)swMauInstacne.c_str()};
    void* valueInstanceId[1] = {(char*)SwMauMom::s_swMauInstance.c_str()};
    attribute[index].attrValues = valueInstanceId;

    ++index;
    int attrAdminState = SwMauMom::UNLOCKED;
    attribute[index].attrName = (char*)SwMauMom::immAttr_administrativeState.c_str();
    attribute[index].attrType = ATTR_INT32T;
    attribute[index].attrValuesNum = 1;
    void* valueAdminState[1] = {&attrAdminState};
    attribute[index].attrValues = valueAdminState;

    // Add the attributes into the list
    for (int i = 0; i < numAttr; ++i)
    {
        attrList.push_back(attribute[i]);
    }

    //string parent = "dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1";
    //string className = "AxeEquipmentSwMau";
    // ACS_CC_ReturnType result
    ACS_CC_ReturnType result = m_omHandler.createObject(SwMauMom::immCln_EquipmentSwMau.c_str(), parent.c_str(), attrList);

   // string objectDn = MausModel::getCpInstance(cpType);
    if (result == ACS_CC_SUCCESS)
    {
        //cout << "Creation object " << objectDn << " completed" << endl;
    	TRACE(("Creation of object <AxeEquipmentSwMau> succeeded"));
    	objExist = false;
    	adminState = attrAdminState;
    	SwMauMom::CpTypeT cpType = SwMauMom::getCpType(parent, 0);
    	TRACE(("Enable for cpType <%d>", cpType));
    	CpMauM maum(cpType);
    	if (adminState == SwMauMom::UNLOCKED)
    	{
    		maum.enable();
    	}
    }
    else
    {
        if (m_omHandler.getInternalLastError() == -14) // 14: ERR_ELEMENT_ALREADY_EXIST
        {
        	TRACE(("Object <AxeEquipmentSwMau> already exists"));
        	objExist = true;
        	adminState = attrAdminState;
        	result = ACS_CC_SUCCESS;
        }
        else
        {
        	//cout << "Creation object " << objectDn << " failed" << endl;

            m_lastErrorCode = m_omHandler.getInternalLastError();
            m_lastErrorText = m_omHandler.getInternalLastErrorText();

            TRACE_ERROR(("Creation of object <AxeSwMauSwMau> failed"));
            TRACE_ERROR(("Error code is <%d>", m_lastErrorCode));
            TRACE_ERROR(("Error text is <%s>", m_lastErrorText.c_str()));
            //cout << "Error is " << m_lastErrorCode << endl;
            //cout << "Error message is <" << m_lastErrorText << ">" << endl;
        }
    }

	TRACE(("SwMauOM::createObject(parent, objExist) return"));
	return result == ACS_CC_SUCCESS;
}


bool SwMauOM::createObject(SwMauMom::CpTypeT cpType)
{
	typedef ACS_CS_API_NS::MauType MauTypeT;

	TRACE(("SwMauOM::createObject()"));

    ACS_CC_ReturnType result = ACS_CC_FAILURE;
    MauTypeT mauType;

	TRACE(("SwMauOM::createObject() getClassInstances of AxeEquipmentDualSidedCp"));
	vector<string> parent_dnList;
	//string parent_className = "AxeEquipmentDualSidedCp";
	//result = m_omHandler.getClassInstances(parent_className.c_str(), parent_dnList);

	result = m_omHandler.getClassInstances(SwMauMom::immCln_EquipmentDualSidedCp.c_str(), parent_dnList);
	if (result != ACS_CC_SUCCESS)
	{
		TRACE(("SwMauOM::createObject() getClassInstances() failed"));
		TRACE(("SwMauOM::createObject() return"));
		return false;
	}

	bool exist = false;
	int admin = -1;
	for(unsigned int i = 0; i < parent_dnList.size(); ++i)
	{
		TRACE(("parent is <%s>", parent_dnList[i].c_str()));

		// Read mauType attribute
		ACS_APGCC_ImmAttribute mauTypeAttr;
		mauTypeAttr.attrName = "mauType";
		vector<ACS_APGCC_ImmAttribute *> attributes;
		attributes.push_back(&mauTypeAttr);

		result = m_omHandler.getAttribute(parent_dnList[i].c_str(), attributes);
		if (result == ACS_CC_SUCCESS)
		{
			//cout << "there is something wrong" << endl;
			//mautype = *((int*)mauTypeAttr.attrValues[0]);
			mauType = *((MauTypeT*)mauTypeAttr.attrValues[0]);
			TRACE(("mauType <%d>", mauType));
			if (mauType == ACS_CS_API_NS::MAUS)
			{
				if (SwMauMom::getCpType(parent_dnList[i], 0) == cpType)
				{
					TRACE(("Create SwMau for cpType <%d>", cpType));
					createObject(parent_dnList[i], exist, admin);
				}
			}
			else
			{
				TRACE(("SwMau object not created for MAUB"));
			}
		}
		else
		{
			TRACE(("Cannot get mauType"));
		}
	}


    TRACE(("SwMauOM::createObject() return"));
    return result == ACS_CC_SUCCESS;
}



bool SwMauOM::createObject(int* admState)
{
	typedef ACS_CS_API_NS::MauType MauTypeT;

	TRACE(("SwMauOM::createObject(int* admState)"));
	admState[0] = admState[1] = -1;

    ACS_CC_ReturnType result = ACS_CC_FAILURE;
    MauTypeT mauType;

	TRACE(("SwMauOM::createObject() getClassInstances of AxeEquipmentDualSidedCp"));
	vector<string> parent_dnList;
	//string parent_className = "AxeEquipmentDualSidedCp";
	//result = m_omHandler.getClassInstances(parent_className.c_str(), parent_dnList);

	result = m_omHandler.getClassInstances(SwMauMom::immCln_EquipmentDualSidedCp.c_str(), parent_dnList);
	if (result != ACS_CC_SUCCESS)
	{
		TRACE(("SwMauOM::createObject() getClassInstances() failed"));
		TRACE(("SwMauOM::createObject() return"));
		return false;
	}

	bool aObjExist = false;
	int aAdminState = -1;
	for(unsigned int i = 0; i < parent_dnList.size(); ++i)
	{
		TRACE(("class instance for AxeEquipment DualSidedCp is <%s>", parent_dnList[i].c_str()));

		// Read mauType attribute
		ACS_APGCC_ImmAttribute mauTypeAttr;
		mauTypeAttr.attrName = "mauType";
		vector<ACS_APGCC_ImmAttribute *> attributes;
		attributes.push_back(&mauTypeAttr);

		result = m_omHandler.getAttribute(parent_dnList[i].c_str(), attributes);
		if (result == ACS_CC_SUCCESS)
		{
			//cout << "there is something wrong" << endl;
			//mautype = *((int*)mauTypeAttr.attrValues[0]);
			mauType = *((MauTypeT*)mauTypeAttr.attrValues[0]);
			TRACE(("mauType <%d>", mauType));
			if (mauType == ACS_CS_API_NS::MAUS)
			{
				TRACE(("Create SwMau object for MAUS"));
				if (createObject(parent_dnList[i], aObjExist, aAdminState))
				{
					SwMauMom::CpTypeT cpType = SwMauMom::getCpType(parent_dnList[i], 0);
					TRACE((">>>> to be removed >>>>>>>"));
					TRACE((">>>> to be removed >>>>>>>"));

					TRACE(("Object created ok <%s>", parent_dnList[i].c_str()));
					TRACE(("cpType is <%d>", cpType));

					TRACE(("<<<< to be removed <<<<<<<"));
					TRACE(("<<<< to be removed <<<<<<<"));
				}
			}
			else
			{
				TRACE(("SwMau object not created for MAUB"));
			}
		}
		else
		{
			TRACE(("Cannot get mauType"));
		}
	}


    TRACE(("SwMauOM::createObject() return"));
    return result == ACS_CC_SUCCESS;
}


bool SwMauOM::readAdminState(int* state)
{
	TRACE(("SwMauOM::readAdminState()"));

	state[0] = state[1] = -1;
	if (!m_initialized)
	{
		//cout << "cannot start object handler" << endl;
		return false;
	}


	vector<string> dnList;
	ACS_CC_ReturnType result  = m_omHandler.getClassInstances(SwMauMom::immCln_EquipmentSwMau.c_str(), dnList);
	if (result != ACS_CC_SUCCESS)
	{
		TRACE(("SwMauOM::readAdminState()ffailed to get class instance"));
		return false;
	}

	TRACE(("SwMauOM::readAdminState() dnList.size <%d>", dnList.size()));

	SwMauMom::CpTypeT cpType;
	ACS_APGCC_ImmAttribute adminStateAttr;
	vector<ACS_APGCC_ImmAttribute *> attributes;
	for (unsigned int i = 0; i < dnList.size(); ++i)
	{
		cpType = SwMauMom::getCpType(dnList[i]);

		TRACE(("cpType <%d> dnList[i]<%s>", cpType, dnList[i].c_str()));

		adminStateAttr.attrName = SwMauMom::immAttr_administrativeState;
		vector<ACS_APGCC_ImmAttribute *>().swap(attributes);
		attributes.push_back(&adminStateAttr);
		if (cpType == SwMauMom::Cp1 || cpType == SwMauMom::Cp2)
		{
			result = m_omHandler.getAttribute(dnList[i].c_str(), attributes);
			if (result != ACS_CC_SUCCESS)
			{
				//cout << "there is something wrong" << endl;
				//m_omHandler.Finalize();
				continue;
			}
			state[cpType - 1] = *((int*)adminStateAttr.attrValues[0]);
		}
	}

	TRACE(("SwMauOM::readAdminState() state[0] <%d>  state[1] <%d>", state[0], state[1]));
	TRACE(("SwMauOM::readAdminState() return"));
	return true;

}


bool SwMauOM::deleteObject(SwMauMom::CpTypeT cpType)
{
	typedef ACS_CS_API_NS::MauType MauTypeT;

	TRACE(("SwMauOM::deleteObject()"));

    ACS_CC_ReturnType result = ACS_CC_FAILURE;

	vector<string> dnList;
	//string parent_className = "AxeEquipmentDualSidedCp";
	//result = m_omHandler.getClassInstances(parent_className.c_str(), parent_dnList);

	result = m_omHandler.getClassInstances(SwMauMom::immCln_EquipmentSwMau.c_str(), dnList);
	if (result != ACS_CC_SUCCESS)
	{
		TRACE(("SwMauOM::deleteObject() getClassInstances() failed"));
		TRACE(("SwMauOM::deleteObject() return"));
		return false;
	}

	TRACE(("dnList.size() <%d>", dnList.size()));

	for(unsigned int i = 0; i < dnList.size(); ++i)
	{
		TRACE(("parent is <%s>", dnList[i].c_str()));

		if (SwMauMom::getCpType(dnList[i]) == cpType)
		{
			TRACE(("Delete SwMau for cpType <%d>", cpType));
			result = m_omHandler.deleteObject(dnList[i].c_str(), ACS_APGCC_ONE);
			if (result == ACS_CC_SUCCESS)
			{
				TRACE(("Object removed ok <%s>", dnList[i].c_str()));
				break;
			}
			else
			{
				TRACE(("Object removed not ok <%s>", dnList[i].c_str()));
			}
		}
		else
		{
			TRACE(("No need to delete for <%s>", dnList[i].c_str()));
		}

	}

    TRACE(("SwMauOM::deleteObject() return <%d>", result));
    return result == ACS_CC_SUCCESS;
}


bool SwMauOM::createObject_old(SwMauMom::CpTypeT cpType)
{
	TRACE(("SwMauOM::createObject() cpType <%d>", cpType));

    ACS_CC_ReturnType result;

#if 0


#endif

#if 0

    // This is a permanent workaround; IO refused to implement the solution
    // If the object is already deleted previously it will failed
    string dn = "AxeSwMauswMauId=1";
    if (m_omHandler.deleteObject(dn.c_str(), ACS_APGCC_ONE) == ACS_CC_SUCCESS)
    {
    	TRACE(("SwMauOM Workaround: Successfully remove <%s> object ", dn.c_str()));
    }
    else {
    	TRACE(("SwMauOM Workaround: Failed to remove <%s> object ", dn.c_str()));
    }

#else

	TRACE(("Try to delete SwMau first"));
	string swMauCp1 = "AxeSwMauswMauId=1,dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1";
    if (m_omHandler.deleteObject(swMauCp1.c_str(), ACS_APGCC_ONE) == ACS_CC_SUCCESS)
    {
    	TRACE(("Successfully remove <%s> object ", swMauCp1.c_str()));
    }
    else {
    	TRACE(("Failed to remove <%s> object ", swMauCp1.c_str()));
    }

#endif

#if 1


    int numAttr = 2;
    vector<ACS_CC_ValuesDefinitionType> attrList;
    ACS_CC_ValuesDefinitionType attribute[numAttr];

    string swMauInstacne = "swMauId=1";
    string immAttr = "swMauId";

    int index = 0;
    attribute[index].attrName = (char*)immAttr.c_str();
    attribute[index].attrType = ATTR_STRINGT;
    attribute[index].attrValuesNum = 1;
    void* valueInstanceId[1] = {(char*)swMauInstacne.c_str()};
    attribute[index].attrValues = valueInstanceId;

    ++index;
    int adminState = SwMauMom::UNLOCKED;
    attribute[index].attrName = (char*)SwMauMom::immAttr_administrativeState.c_str();
    attribute[index].attrType = ATTR_INT32T;
    attribute[index].attrValuesNum = 1;
    void* valueAdminState[1] = {&adminState};
    attribute[index].attrValues = valueAdminState;

    // Add the attributes into the list
    for (int i = 0; i < numAttr; ++i)
    {
        attrList.push_back(attribute[i]);
    }

    string parent = "dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1";
    string className = "AxeEquipmentSwMau";
    result = m_omHandler.createObject(className.c_str(), parent.c_str(), attrList);

   // string objectDn = MausModel::getCpInstance(cpType);
    if (result == ACS_CC_SUCCESS)
    {
        //cout << "Creation object " << objectDn << " completed" << endl;
    	TRACE(("Creation of object <AxeEquipmentSwMau> succeeded"));
    }
    else
    {
        if (m_omHandler.getInternalLastError() == -14) // 14: ERR_ELEMENT_ALREADY_EXIST
        {
         	TRACE(("Object <AxeEquipmentSwMau> already exists"));
            result = ACS_CC_SUCCESS;
        }
        else
        {
        	//cout << "Creation object " << objectDn << " failed" << endl;

            m_lastErrorCode = m_omHandler.getInternalLastError();
            m_lastErrorText = m_omHandler.getInternalLastErrorText();

            TRACE_ERROR(("Creation of object <AxeSwMauSwMau> failed"));
            TRACE_ERROR(("Error code is <%d>", m_lastErrorCode));
            TRACE_ERROR(("Error text is <%s>", m_lastErrorText.c_str()));
            //cout << "Error is " << m_lastErrorCode << endl;
            //cout << "Error message is <" << m_lastErrorText << ">" << endl;
        }
    }

#endif

#if 0

    // Register as OI for Classic class.
    acs_apgcc_oihandler_V3 	m_oiHandler;
    result = m_oiHandler.addClassImpl(this, className.c_str());
    if (result != ACS_CC_SUCCESS)
    {
    	ostringstream os;
        os << "SwMauOM::run() failed to register class " << className << endl;
        os << "Error code is " << getInternalLastError() << endl;
        os << "Error text is " << getInternalLastErrorText() << endl;

        TRACE(("%s", os.str().c_str()));
		//EventReporter::instance().report(__LINE__,
		//		os.str().c_str(),
		//		"MausOI::run()");

        return;
    }

#endif

	TRACE(("SwMauOM::createObject() returns"));
	return result == ACS_CC_SUCCESS;
}


