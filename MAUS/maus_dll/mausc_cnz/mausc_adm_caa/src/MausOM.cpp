/*
 * MausOM.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: xdtthng
 */

#include <vector>
#include <string>

// Remove later
#include <iostream>

#include "MausOM.h"
#include "MAUFunx.h"
#include "FileFunx.h"
#include "maus_trace.h"
#include "Maus_AehEvent.h"
#include "ParseIni.h"

using namespace std;

MausOM::MausOM(): m_initialized(false), m_omHandler(),
		m_lastErrorCode(0),
		m_lastErrorText("")

{
	TRACE(("MausOM::MausOM() ctor"));
	(void) this->init();
}

bool MausOM::init()
{
	TRACE(("MausOM::init()"));
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
        TRACE_ERROR(("MausOM::init() failed to get OmHandler, error code <%d>, error text <%s>, "
        		"m_lastErrorCode, m_lastErrorText.c_str()"));
    }
    TRACE(("MausOM::init() returns <%d>", m_initialized));
    return m_initialized;

}

MausOM::~MausOM()
{
	TRACE(("MausOM::~MausOM() dtor"));

    if (m_initialized)
    {
        m_omHandler.Finalize();
    }

    m_initialized = false;

}

bool MausOM::createObject(MausModel::CpTypeT cpType)
{
	TRACE(("MausOM::createObject() with cpType <%d>", cpType));

    ACS_CC_ReturnType result;
    int numAttr = 4;
    vector<ACS_CC_ValuesDefinitionType> attrList;
    ACS_CC_ValuesDefinitionType attribute[numAttr];

	// Fill Id
    string cpInstance = MausModel::getCpInstance(cpType);
    TRACE(("cpInstance <%s>", cpInstance.c_str()));
    TRACE(("attrName <%s>", MausModel::immAttr_dualSidedCpId.c_str()));

    attribute[0].attrName = (char*)MausModel::immAttr_dualSidedCpId.c_str();
    attribute[0].attrType = ATTR_STRINGT;
    attribute[0].attrValuesNum = 1;
    void* valueInstanceId[1] = {(char*)cpInstance.c_str()};
    attribute[0].attrValues = valueInstanceId;

    // Fill the operationState attribute
    int adminState = 0;
    attribute[1].attrName = (char*)MausModel::immAttr_administrativeState.c_str();
    attribute[1].attrType = ATTR_INT32T;
    attribute[1].attrValuesNum = 1;
    void* valueAdminState[1] = {&adminState};
    attribute[1].attrValues = valueAdminState;

    // Fill the changeState attribute
    int changeState = 0;
    attribute[2].attrName = (char*)MausModel::immAttr_changeState.c_str();
    attribute[2].attrType = ATTR_INT32T;
    attribute[2].attrValuesNum = 1;
    void* valueChangeState[2] = {&changeState};
    attribute[2].attrValues = valueChangeState;

    // Fill the fexRequest attribute
    string iniFile = MausModel::getFexIniFileName(cpType);
    TRACE(("Fex ini file name <%s>", iniFile.c_str()));

    int fexReq = 0;
    attribute[3].attrName = (char*)MausModel::immAttr_fexRequest.c_str();
    attribute[3].attrType = ATTR_INT32T;
    attribute[3].attrValuesNum = 1;
    void* valuefexReq[2] = {&fexReq};
    attribute[3].attrValues = valuefexReq;


    // Add the attributes into the list
    for (int i = 0; i < numAttr; ++i)
    {
        attrList.push_back(attribute[i]);
    }

    TRACE(("createObject() immCln_DualSidedCp <%s> parent <%s>",
    		MausModel::immCln_DualSidedCp.c_str(),
    		MausModel::getParentInstance().c_str()
    		));

    result = m_omHandler.createObject(MausModel::immCln_DualSidedCp.c_str(), MausModel::getParentInstance().c_str(), attrList);

    string objectDn = MausModel::getCpInstance(cpType);
    if (result == ACS_CC_SUCCESS)
    {
        //cout << "Creation object " << objectDn << " completed" << endl;
    	TRACE(("Creation of object <%s> succeeded", objectDn.c_str()));
    }
    else
    {
        if (m_omHandler.getInternalLastError() == -14) // 14: ERR_ELEMENT_ALREADY_EXIST
        {
         	TRACE(("Object <%s> already exists", objectDn.c_str()));
    		string fileName = MausModel::getFexIniFileName(cpType);
    		if (FileFunx::fileExists(fileName.c_str()))
    		{
    		    fexReq = ParseIni::getPrivateProfile("fex", "fexRequest", 0, iniFile.c_str());
    		    TRACE(("fexRequest from ini file  <%d>", fexReq));
    		    if (setAttribute(fexReq, MausModel::immAttr_fexRequest, cpType))
    		    {
    		    	TRACE(("fexRequest attr is updated with value from ini file"));
    		    }
    		    else
    		    {
    		    	TRACE(("failed to update fexRequest attr with value from ini file"));
    		    }
    		}

            result = ACS_CC_SUCCESS;
        }
        else
        {
        	//cout << "Creation object " << objectDn << " failed" << endl;

            m_lastErrorCode = m_omHandler.getInternalLastError();
            m_lastErrorText = m_omHandler.getInternalLastErrorText();

            TRACE_ERROR(("Creation of object <%s> failed", objectDn.c_str()));
            TRACE_ERROR(("Error code is <%d>", m_lastErrorCode));
            TRACE_ERROR(("Error text is <%s>", m_lastErrorText.c_str()));
            //cout << "Error is " << m_lastErrorCode << endl;
            //cout << "Error message is <" << m_lastErrorText << ">" << endl;
        }
    }

    return result == ACS_CC_SUCCESS;
}

bool MausOM::createAndSyncObject()
{

    // Check If OM initialized
    if (!m_initialized)
    {
        TRACE_ERROR(("MausOM::createAndSyncObject(), cannot create IMM object due to uninitialized OM"));

		EventReporter::instance().report(__LINE__,
				"Cannot create IMM object due to uninitialized OM",
				"MausOM::createAndSyncObject()");

        return false;
    }

    if (!this->createObject(MausModel::Cp1))
    {
    	TRACE_ERROR(("MausOM::createAndSyncObject(), cannot create MO for CP1"));
    }

    if (MAUFunx::getDualSidedCpCount() == 2)
    {
    	if (!this->createObject(MausModel::Cp2))
    	{
        	TRACE_ERROR(("MausOM::createAndSyncObject(), cannot create MO for CP2"));

    	}
    }

	return true;
}

#if 0
//**********************************************************************
//  modifyStringAttrs()
//**********************************************************************
bool MausOM::modifyAttrs(const string& objectName, const string& attrName,
                                         const string& value, const string& transName)
{
    //newTRACE(("BUParamsOM::modifyStringAttrs(%s, %s)", 0, attrName.c_str(), value.c_str()));

    ACS_CC_ReturnType returnCode;

    // Modify
    ACS_CC_ImmParameter attrs;

    /*Fill the rdn Attribute */
    attrs.attrName = (char*)attrName.c_str();
    attrs.attrType = ATTR_STRINGT;
    attrs.attrValuesNum = 1;
    void* valueAttrs[1]={reinterpret_cast<void*>((char*)value.c_str())};
    attrs.attrValues = valueAttrs;

    returnCode = m_omHandler.modifyAttribute(objectName.c_str(), &attrs, transName);

    if(returnCode == ACS_CC_SUCCESS)
    {
        //TRACE(("Modify Attribute %s completed with transaction %s \n", 0,
        //        attrName.c_str(), transName.c_str()));

    	cout << "MausOM::modifyAttrs() with string value <" << value << "> ok" << endl;
    }
    else
    {
        //newTRACE((LOG_LEVEL_ERROR, "error - Modify attribute %s failed with error: %d - %s\n", 0,
        //        attrName.c_str(),
        //        m_omHandler.getInternalLastError(), m_omHandler.getInternalLastErrorText()));
       // EVENT((CPS_BUAP_Events::modifyAttr_failed, 0, ""));

    	cout << "MausOM::modifyAttrs() with string value <" << value << "> failed" << endl;
    }

    return (returnCode == ACS_CC_SUCCESS);
}

bool MausOM::modifyAttrs(const string& objectName, const string& attrName,
                         int value, const string& transName, bool unsignedIntUsed)
{
    //newTRACE(("BUParamsOM::modifyIntAttrs(%s, %d)", 0, attrName.c_str(), value));

    ACS_CC_ReturnType returnCode;

    // Modify
    ACS_CC_ImmParameter attrs;

    /*Fill the rdn Attribute */
    attrs.attrName = (char*)attrName.c_str();
    // Check the value type
    if (unsignedIntUsed)
    {
        attrs.attrType = ATTR_UINT32T;
    }
    else
    {
        attrs.attrType = ATTR_INT32T;
    }
    attrs.attrValuesNum = 1;
    void* valueAttrs[1]={reinterpret_cast<void*>(&value)};
    attrs.attrValues = valueAttrs;

    //returnCode = m_omHandler.modifyAttribute(objectName.c_str(), &attrs, transName);
    returnCode = m_omHandler.modifyAttribute(objectName.c_str(), &attrs);

    if(returnCode == ACS_CC_SUCCESS)
    {
        //TRACE(("Modify Attribute %s completed\n", 0, attrName.c_str()));
    	cout << "MausOM::modifyAttrs() with int value <" << value << "> ok" << endl;
    }
    else
    {
        //newTRACE((LOG_LEVEL_ERROR, "error - Modify attribute %s failed with error: %d - %s\n", 0,
        //        attrName.c_str(),
        //        m_omHandler.getInternalLastError(), m_omHandler.getInternalLastErrorText()));
        //EVENT((CPS_BUAP_Events::modifyAttr_failed, 0, ""));
    	cout << "MausOM::modifyAttrs() with int value <" << value << "> failed" << endl;
    }

    return (returnCode == ACS_CC_SUCCESS);
}
#endif

bool MausOM::setAttribute(int attribute, const std::string& name, MausModel::CpTypeT cpType)
{
	TRACE(("MausOM::setAttribute() name <%s> with value <%d>", name.c_str(), attribute));

	if (!m_initialized)
	{
		//cout << "cannot start object handler" << endl;
		TRACE(("MausOM::setAttribute() failed; om not initialized; returns false"));
		return false;
	}

	ACS_CC_ImmParameter param;
	param.attrName = (char*)name.c_str();
	param.attrType = ATTR_INT32T;
	param.attrValuesNum = 1;

    void* valueAttr[1] = {&attribute};
    param.attrValues = valueAttr;
    string transactionName("modifyCpMauMAttribute");
    string cpDn = MausModel::getCpDn(cpType);
    ACS_CC_ReturnType result = m_omHandler.modifyAttribute(cpDn.c_str(), &param, transactionName);

    if (result != ACS_CC_SUCCESS)
    {
    	TRACE(("MausOM::setAttribute(), modifyAttribute() failed with result <%d>; returns false", result));
        return false;
    }
    result = m_omHandler.applyRequest(transactionName);
    if (result != ACS_CC_SUCCESS)
    {
    	TRACE(("MausOM::setAttribute(), applyRequest() failed with result <%d>; returns false", result));
        return false;
    }

    TRACE(("MausOM::setAttribute() ok, returns true"));
    return true;
}


bool MausOM::readAttribute(int& attribute, const std::string& name, MausModel::CpTypeT cpType)
{
	TRACE(("MausOM::readAttribute() for name <%s> ", name.c_str()));

	attribute = -1;
	if (!m_initialized)
	{
		//cout << "cannot start object handler" << endl;
		TRACE(("MausOM::readAttribute() failed; om not initialized; returns false"));
		return false;
	}

	ACS_APGCC_ImmAttribute immAttribute;
	immAttribute.attrName = name;
	vector<ACS_APGCC_ImmAttribute *> attributes;
	attributes.push_back(&immAttribute);

	string cpDn = MausModel::getCpDn(cpType);
	ACS_CC_ReturnType result = m_omHandler.getAttribute(cpDn.c_str(), attributes);
	if (result != ACS_CC_SUCCESS)
	{
		//cout << "there is something wrong" << endl;
		TRACE(("MausOM::readAttribute(), getAttribute() failed with result <%d>; returns false", result));
		return false;
	}
	attribute = *((int*)immAttribute.attrValues[0]);
	TRACE(("MausOM::readAttribute(), attr val <%d>; return true", attribute));
	return true;
}

bool MausOM::readFexRequest(int& fexreq, MausModel::CpTypeT cpType)
{
	TRACE(("MausOM::readFexRequest()"));
	return readAttribute(fexreq, MausModel::immAttr_fexRequest, cpType);
}

bool MausOM::setFexRequest(int fexreq, MausModel::CpTypeT cpType)
{
	TRACE(("MausOM::setFexRequest()"));

	bool res = setAttribute(fexreq, MausModel::immAttr_fexRequest, cpType);
	if (res)
	{
		string dataPath;
		string fileName = MausModel::getFexIniFileName(cpType, dataPath);
		if (FileFunx::createDirectories(dataPath))
		{
			ParseIni::writePrivateProfile("fex", "fexRequest", fexreq, fileName.c_str());
		}
		else
		{
			//TRACE(());
		}
	}
	return res;
}

bool MausOM::readAdminState(int& adminState, MausModel::CpTypeT cpType)
{

	return readAttribute(adminState, MausModel::immAttr_administrativeState, cpType);

#if 0
	adminState = -1;
	if (!m_initialized)
	{
		//cout << "cannot start object handler" << endl;
		return false;
	}

	ACS_APGCC_ImmAttribute adminStateAttr;
	//adminStateAttr.attrName = "administrativeState";
	adminStateAttr.attrName = MausModel::immAttr_administrativeState;
	vector<ACS_APGCC_ImmAttribute *> attributes;
	attributes.push_back(&adminStateAttr);

	string cpDn = MausModel::getCpDn(cpType);
	ACS_CC_ReturnType result = m_omHandler.getAttribute(cpDn.c_str(), attributes);
	if (result != ACS_CC_SUCCESS)
	{
		//cout << "there is something wrong" << endl;
		//m_omHandler.Finalize();
		return false;
	}
	adminState = *((int*)adminStateAttr.attrValues[0]);
	return true;
#endif
}

bool MausOM::readChangeState(int& changeState, MausModel::CpTypeT cpType)
{

	return readAttribute(changeState, MausModel::immAttr_changeState, cpType);

#if 0
	changeState = -1;
	if (!m_initialized)
	{
		//cout << "cannot start object handler" << endl;
		return false;
	}

	ACS_APGCC_ImmAttribute changeStateAttr;
	//changeStateAttr.attrName = "changeState";
	changeStateAttr.attrName = MausModel::immAttr_changeState;
	vector<ACS_APGCC_ImmAttribute *> attributes;
	attributes.push_back(&changeStateAttr);

	string cpDn = MausModel::getCpDn(cpType);
	ACS_CC_ReturnType result = m_omHandler.getAttribute(cpDn.c_str(), attributes);
	if (result != ACS_CC_SUCCESS)
	{
		//cout << "there is something wrong" << endl;
		//m_omHandler.Finalize();
		return false;
	}
	changeState = *((int*)changeStateAttr.attrValues[0]);
	return true;
#endif
}
