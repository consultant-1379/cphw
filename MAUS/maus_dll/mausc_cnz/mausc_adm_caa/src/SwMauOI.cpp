/*
 * SwMauOI.cpp
 *
 *  Created on: Apr 11, 2016
 *      Author: xdtthng
 */


#include <iostream>
#include <sstream>

#if 0
#include <sys/select.h>
#else
#include <sys/poll.h>
#endif

#include "SwMauOI.h"
#include "maus_trace.h"
#include "CpMauM.h"
#include "CptSwMau.h"
#include "MAUFunx.h"
#include "FileFunx.h"
#include "ParseIni.h"
#include "Maus_AehEvent.h"

using namespace std;


SwMauOI::SwMauOI(int a, int* mauType, SysInfo::EventT& e):
		acs_apgcc_objectimplementerinterface_V3(SwMauMom::immImpl_OiName),
		m_running(false),
		m_oiHandler(),
		m_endEvent(),
		m_oiThread(),
		m_architecture(a),
		m_mauChangeEvents(e),
		m_modAttr(),
		m_om()
{
	TRACE(("SwMauOI::SwMauOI() ctor"));

	m_mauType[0] = mauType[0];
	m_mauType[1] = mauType[1];

	TRACE(("SwMauOI::SwMauOI() return"));
}

SwMauOI::~SwMauOI()
{
	TRACE(("SwMauOI::~SwMauOI()"));

	if (m_oiThread.joinable())
	{
		TRACE(("SwMauOI::~SwMauOI() joining main thread"));
		m_oiThread.join();
	}

}


int SwMauOI::start()
{
	TRACE(("SwMauOI::start()"));
	m_oiThread = boost::thread(boost::bind(&SwMauOI::run, this));
	TRACE(("SwMauOI::start() returns"));
	return 0;
}

int SwMauOI::stop()
{
	TRACE(("SwMauOI::stop()"));
	m_endEvent.set();
	return 0;
}


ACS_CC_ReturnType SwMauOI::create(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **attr)
{

    TRACE(("############################################################"));
    TRACE(("------------------------------------------------------------"));

    TRACE(("SwMauOI::create(oiHandle, ccbId<%lu>, %s, %s, attr)", ccbId, ((className) ? className : "NULL"), ((parentName) ? parentName : "NULL")));

    TRACE(("className <%s>", className));
    TRACE(("parentName <%s>", parentName));
    TRACE(("attr->attrName <%s>", (*attr)->attrName));
    TRACE(("attr->attrValues <%s>", *(*attr)->attrValues));


    TRACE(("############################################################"));
    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    // Do nothing
    return result;
}

ACS_CC_ReturnType SwMauOI::deleted(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
    TRACE(("SwMauOI::deleted(oiHandle, ccbId, %s)", ((objName) ? objName : "NULL")));

    (void) ccbId;

    // Return this to stop the object being  deleted
    //ACS_CC_ReturnType result = ACS_CC_FAILURE;

    // Return this and the object is always deleted
    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    return result;
}

ACS_CC_ReturnType SwMauOI::complete(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId)
{
    TRACE(("SwMauOI::complete(oiHandle, ccbId<%lu>)", ccbId));

    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    // If error checking is needed, it should be rejected here
	TRACE(("m_modAttr.size() <%d>", m_modAttr.size()));
    for (size_t i = 0; i < m_modAttr.size(); ++i)
    {
    	if (m_modAttr[i].m_completeStatus)
    	{
    		//string errMsg = "Cannot change attribute due to PTCOI is in progress.";
    		string errMsg = "Cannot change administrativeState to LOCKED: "
    		"CPT command session in progress which requires MAUS in operational state";
    		setExitCode(3500, errMsg);
    		TRACE(("SwMauOI::complete() returns ACS_CC_FAILURE"));
    		return ACS_CC_FAILURE;
    	}
    }

    TRACE(("SwMauOI::complete() returns ACS_CC_SUCCESS"));
    return result;
}

// -------------------------------------------------------------
// The callback sequence is modify, complete, apply
// If there is any rejection, it should be rejected at complete
// -------------------------------------------------------------

ACS_CC_ReturnType SwMauOI::modify(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	const int LOCKED = 0;
	//const int UNLOCKED = 1;

    TRACE(("SwMauOI::modify(oiHandle, ccbId<%lu>, %s, attrMods)", ccbId, ((objName) ? objName : "NULL")));


    SwMauMom::CpTypeT cpType = SwMauMom::getCpType(objName);
	TRACE(("SwMauOI::modify() cptype<%d>", cpType));

	// If mauType is not MAUS then take no action
	if (m_mauType[cpType - 1] != 2)
	{
		TRACE(("SwMauOI::modify() take no action, m_mauType is <%d>, which is not MAUS", m_mauType[cpType - 1]));
		return ACS_CC_SUCCESS;
	}

    // Clean up at every run
    vector<ModAttr>().swap(m_modAttr);
	for (size_t i = 0; attrMods[i] ; ++i)
    {
    	ACS_APGCC_AttrValues modAttr = attrMods[i]->modAttr;
    	ModAttr attr;
        attr.m_cpType = cpType;
        attr.m_attr = *(int*)modAttr.attrValues[0];
        if (modAttr.attrName == SwMauMom::immAttr_administrativeState)
        {
        	TRACE(("SwMauOI::modify() administrativeState <%d>  --------", attr.m_attr));
        	attr.m_attrName = SwMauMom::AdministrativeState;

        	if (attr.m_attr == LOCKED)
        	{
        		CptSwMau cpt((int)cpType);
        		bool status = cpt.ptcoiExist();
        		TRACE(("SwMauOI::modify(), LOCKED with ptcoi session <%d>", status));
        		if (status)
        		{
        			attr.m_completeStatus = ModAttr::MOD_ATTR_ERROR;
        		}
        	}
        }
        else
        {
        	TRACE(("SwMauOI::modify() unknown attr <%s> ------", modAttr.attrName));
        	TRACE(("SwMauOI::modify() no action taken  -------------"));
        	continue;
        }
        m_modAttr.push_back(attr);
    }

    TRACE(("SwMauOI::modify() returns with m_modAttr.size() <%d>", m_modAttr.size()));
    return ACS_CC_SUCCESS;

}

void SwMauOI::abort(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId)
{
    TRACE(("SwMauOI::abort(oiHandle, %lu)", ccbId));
    //cout << "SwMauOI::abort()" << endl;

}

void SwMauOI::apply(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId)
{
    TRACE(("SwMauOI::apply(oiHandle, ccbId<%lu>)", ccbId));

	TRACE(("m_modAttr.size() <%d>", m_modAttr.size()));

    for (size_t i = 0; i < m_modAttr.size(); ++i)
    {
		int modAttrValue = m_modAttr[i].m_attr;
		int cpType = m_modAttr[i].m_cpType;
		bool opResult = false;

		TRACE(("m_modAttr[i].m_attrName <%d>", m_modAttr[i].m_attrName));

    	if (m_modAttr[i].m_attrName == SwMauMom::AdministrativeState)
    	{
    		TRACE(("SwMauOI::apply() administrativeState value <%d>  --------", modAttrValue));

    		CpMauM cpMau(cpType);
    		if (modAttrValue == SwMauMom::LOCKED)
    		{
    			opResult = cpMau.disable();
    		}
    		else
    		{
    			opResult = cpMau.enable();
    		}
    		//cout << "SwMauOI::apply()" << endl;
    		TRACE(("SwMauOI::apply() administrativeState <%d> with result <%d>", modAttrValue, opResult));
    	}
    }

    // Clean up after every run
    vector<ModAttr>().swap(m_modAttr);
    TRACE(("SwMauOI::apply() returns; m_modAttr.size() <%d> m_modAttr.capacity() <%d>", m_modAttr.size(), m_modAttr.capacity()));



}

// This is show
ACS_CC_ReturnType SwMauOI::updateRuntime(const char* p_objName, const char** attrNames)
{
    TRACE(("SwMauOI::updateRuntime(%s)", ((p_objName) ? p_objName : "NULL")));

    //ACS_CC_ReturnType result = ACS_CC_SUCCESS;


    // At the moment, the MO has only one runtime attribute, thus, there is no need to check attrNames
    //(void) attrNames;
    TRACE(("++++++++++++++++++++++++++++++++++++++++++++++"));
    TRACE(("SwMauOI::updateRuntime() attrNames <%s>", *attrNames));
    TRACE(("++++++++++++++++++++++++++++++++++++++++++++++"));

    SwMauMom::CpTypeT cpType = SwMauMom::getCpType(p_objName);
    TRACE(("SwMauOI::updateRuntime() for cpType <%d>", cpType));

    CpMauM cpMau(cpType);

    // OperationalState
    // ENABLED == 1, DISABLE == 0
    int opState = cpMau.getOperationalState();

    ACS_CC_ImmParameter attrVal;

    // Update the attribute
    attrVal.attrName = (char*) *attrNames;
    attrVal.attrType = ATTR_INT32T;
    attrVal.attrValuesNum = 1;
    void* newValue[1] = {reinterpret_cast<void*>(&opState)};

    attrVal.attrValues = newValue;

    if (this->modifyRuntimeObj(p_objName, &attrVal))
    {
        TRACE_ERROR(("Failed to modify runtime attribute %s. Error: %d - %s",
        		*attrNames,
                this->getInternalLastError(), this->getInternalLastErrorText()));

        return ACS_CC_FAILURE;
    }

    return ACS_CC_SUCCESS;

}

void SwMauOI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation,
        const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
        ACS_APGCC_AdminOperationParamType** paramType)
{
    TRACE(("SwMauOI::adminOperationCallback(%lu, %lu, %s, %lu, paramList)", oiHandle, invocation,
            ((p_objName) ? p_objName : "NULL"), operationId));

    // This callback is for handling action. This MOM has no actions for now.
    // It had action, but removed after review.
    (void) paramType;
}

void SwMauOI::run()
{
    TRACE(("SwMauOI::run()"));
    bool objCreatedOk = false;

    int admState[2] = {-1, -1};
    if (!m_om.createObject(admState))
    {
    	TRACE_ERROR(("SwMauOI::run() failed to create OM"));
    	TRACE_ERROR(("SwMauOI::run() continue"));
		EventReporter::instance().report(__LINE__,
				"Cannot create and sync CpMauM MO",
				"SwMauOI::run()");

		objCreatedOk = false;
		// Should give an event, but continues
		// This is also a normal situation
    	//return;
    }
    else
    {
    	objCreatedOk = true;
    }

    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    // Register as OI for Sw Mau.
    result = m_oiHandler.addClassImpl(this, SwMauMom::immCln_EquipmentSwMau.c_str());
    if (result != ACS_CC_SUCCESS)
    {
    	ostringstream os;
        os << "SwMauOI::run() failed to register class " << SwMauMom::immCln_EquipmentSwMau << endl;
        os << "Error code is " << getInternalLastError() << endl;
        os << "Error text is " << getInternalLastErrorText() << endl;

        TRACE_ERROR(("%s", os.str().c_str()));
		EventReporter::instance().report(__LINE__,
				os.str().c_str(),
				"SwMauOI::run()");

		TRACE_ERROR(("SwMauOI::run() continue, objCreatedOk <%d>", objCreatedOk));
        //return;
    }
    else
    {
    	TRACE_ERROR(("SwMauOI::run() register class ok <%s>, objCreatedOk <%d>", SwMauMom::immCln_EquipmentSwMau.c_str(), objCreatedOk));
    }

#if 0

    // There is no need to enable or disable
    // Enable or disable is done at object creation

    bool opResult = false;
    if (m_om.readAdminState(admState))
    {
    	TRACE(("SwMauOI::run() admState[0] <%d> admState[1] <%d>", admState[0], admState[1]));
    	for (int i = 0; i < 2; ++i)
    	{
    		if (admState[i] == -1 || m_mauType[i] != 2)
    			continue;

    		CpMauM cpMau(i+1);
            opResult = admState[i] == SwMauMom::LOCKED ? cpMau.disable(): cpMau.enable();
            TRACE(("SwMauOI::run(), for CP1, opResult <%d> when disable() or enable()", opResult));
    	}
    }

#endif

    struct pollfd fds[2 + SysInfo::MaxEvent];
	int index = 0;
	fds[index].fd = m_endEvent.get();
	fds[index].events = POLLIN;

	fds[++index].fd = this->getSelObj();
	fds[index].events = POLLIN;

	int j = 0;
	++index;
	for (int i = 0; i < SysInfo::MaxEvent; ++i)
	{
		j = i + index;
		fds[j].fd = m_mauChangeEvents[i].get();
		fds[j].events = POLLIN;
	}

	// Must not directly return from this loop
	bool stop = false;
	int res;
	while (!stop)
	{
		// 10 seconds = 10,000 milli seconds interval
		res = poll(fds, 2 + SysInfo::MaxEvent, 10000);

		if (res == 0)
		{
			// Time out
			continue;
		}

		if (res < 0)
		{
			TRACE(("SwMauOI::run() poll error <%d>", errno));
			continue;
		}

		// terminating event
	    if (fds[0].revents & POLLIN )
	    {
	    	//cout << "ready to call msg queue received, msg len set to " << *msg_len << endl;
	    	fds[0].revents = 0;
	    	TRACE(("SwMauOI::run() detects end event"));
	    	stop = true;
	    	continue;
	    }

		// CpMauM MO has signaled
	    if (fds[1].revents & POLLIN )
	    {
	    	fds[1].revents = 0;
	    	TRACE(("SwMauOI::run() OI callback signals"));
            result = this->dispatch(ACS_APGCC_DISPATCH_ONE);

            if (result != ACS_CC_SUCCESS)
            {
                TRACE(("Failed to dispatch IMM event"));
        		EventReporter::instance().report(__LINE__,
        				"Failed to dispatch IMM event",
        				"SwMauOI::run()");
                stop = true;
            }
            continue;
	    }

		// check if CS has notified the changes in mauType.
	    // If it does, Note this change
	    for (int i = 2; i < 2 + SysInfo::MaxEvent; ++i)
	    {
	    	j = i - 2;
		    if (fds[i].revents & POLLIN )
		    {
		    	fds[i].revents = 0;
		    	TRACE(("Event index <%d> is raised", i));
		    	TRACE(("Event file descriptor <%d> at j <%d>", m_mauChangeEvents[j].get(), j));
		    	m_mauChangeEvents[j].reset();

				switch (j)
				{
				case 0:
					// Change to MAUS for CP1
					TRACE(("Setting m_mauType[0] to 2"));
					m_mauType[0] = 2;
					break;
				case 1:
					// Change to MAUB for CP1
					TRACE(("Setting m_mauType[0] to 1"));
					m_mauType[0] = 1;
					break;
				case 2:
					// Change to MAUS for CP2
					TRACE(("Setting m_mauType[1] to 2"));
					m_mauType[1] = 2;
					break;
				default: // case 3
					TRACE(("Setting m_mauType[1] to 1"));
					m_mauType[1] = 1;
					break;
				}
		    }
	    }

	}

    // Clean up interaction with IMM
    m_oiHandler.removeClassImpl(this, SwMauMom::immCln_EquipmentSwMau.c_str());
    TRACE(("SwMauOI::run() returns"));

}

