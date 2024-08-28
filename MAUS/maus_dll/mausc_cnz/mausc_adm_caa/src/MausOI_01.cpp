/*
 * MausOI.cpp
 *
 *  Created on: Jan 17, 2015
 *      Author: xdtthng
 */


#include <iostream>
#include <sstream>

#if 0
#include <sys/select.h>
#else
#include <sys/poll.h>
#endif

#include "MausOI.h"
#include "maus_trace.h"
#include "CpMauM.h"
#include "MAUFunx.h"
#include "Maus_AehEvent.h"

using namespace std;

MausOI::MausOI(int a, int* mauType, SysInfo::EventT& e):
		acs_apgcc_objectimplementerinterface_V3(MausModel::immImpl_OiName),
		m_running(false),
		m_oiHandler(),
		m_endEvent(),
		m_oiThread(),
		m_architecture(a),
		m_mauChangeEvents(e),
		m_om()
{
	TRACE(("MausOI::MausOI() ctor"));
	m_mauType[0] = mauType[0];
	m_mauType[1] = mauType[1];

}

MausOI::~MausOI()
{
	TRACE(("MausOI::~MausOI()"));

	if (m_oiThread.joinable())
	{
		TRACE(("MausOI::~MausOI() joining main thread"));
		m_oiThread.join();
	}

}

int MausOI::start()
{
	TRACE(("MausOI::start()"));
	m_oiThread = boost::thread(boost::bind(&MausOI::run, this));
	TRACE(("MausOI::start() returns"));
	return 0;
}

int MausOI::stop()
{
	TRACE(("MausOI::stop()"));
	m_endEvent.set();
	return 0;
}

ACS_CC_ReturnType MausOI::create(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **)
{
    TRACE(("BUParamsOI::create(oiHandle, ccbId<%lu>, %s, %s, attr)", ccbId, ((className) ? className : "NULL"), ((parentName) ? parentName : "NULL")));

    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    // Do nothing
    return result;
}

ACS_CC_ReturnType MausOI::deleted(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
    TRACE(("MausOI::deleted(oiHandle, ccbId<%lu>, %s)", ccbId, ((objName) ? objName : "NULL")));

    ACS_CC_ReturnType result = ACS_CC_FAILURE;

    // Do nothing and object must not be deleted
    return result;
}

ACS_CC_ReturnType MausOI::complete(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId)
{
    TRACE(("MausOI::complete(oiHandle, ccbId<%lu>)", ccbId));

    //cout << "MausOI:complete()" << endl;

    ACS_CC_ReturnType result = ACS_CC_SUCCESS;
    //result = ACS_CC_FAILURE;
    return result;
}

// The callback sequence is modify, complete, apply
ACS_CC_ReturnType MausOI::modify(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
    TRACE(("MausOI::modify(oiHandle, ccbId<%lu>, %s, attrMods)", ccbId, ((objName) ? objName : "NULL")));

	MausModel::CpTypeT cpType = MausModel::getCpType(objName);
	TRACE(("MausOI::modify() cptype<%d>", cpType));

    bool opResult = true;
    CpMauM cpMau(cpType);
    ACS_APGCC_AttrValues modAttr = attrMods[0]->modAttr;
    int modAttrValue = *(int*)modAttr.attrValues[0];
    if (modAttrValue == 1)
    {
    	opResult = cpMau.disable();
    }
    else
    {
    	opResult = cpMau.enable();
    }
    TRACE(("MausOI::modify() changes attr <%s> to <%d> with result <%d>", modAttr.attrName, modAttrValue, opResult));

    return ACS_CC_SUCCESS;
}

void MausOI::abort(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId)
{
    TRACE(("MausOI::abort(oiHandle, %lu)", ccbId));
    //cout << "MausOI::abort()" << endl;

}

void MausOI::apply(ACS_APGCC_OiHandle, ACS_APGCC_CcbId ccbId)
{
    TRACE(("MausOI::apply(oiHandle, ccbId<%lu>)", ccbId));
    //cout << "MausOI::apply()" << endl;
}

ACS_CC_ReturnType MausOI::updateRuntime(const char* p_objName, const char** attrNames)
{
    TRACE(("MausOI::updateRuntime(%s)", ((p_objName) ? p_objName : "NULL")));

    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    // At the moment, the MO has only one runtime attribute, thus, there is no need to check attrNames
    (void) attrNames;

    MausModel::CpTypeT cpType = MausModel::getCpType(p_objName);
    TRACE(("MausOI::updateRuntime(from imm) for cpType <%d>", cpType));

    result = updateRuntime(cpType);
    TRACE(("MausOI::updateRuntime(cpType) returns <%d>", result));

    return result;
}

ACS_CC_ReturnType MausOI::updateRuntime(MausModel::CpTypeT cpType)
{
	TRACE(("MausOI::updateRuntime(cpType <%d>)", cpType));

	CpMauM cpMau(cpType);
	int adminState = -1;
	if (m_om.readAdminState(adminState, cpType))
	{
		TRACE(("MausOI::updateRuntime() with adminState <%d>", adminState));
		TRACE(("MausOI::updateRuntime() with m_mauType <%d>", m_mauType[cpType - 1]));
		if (adminState == 0 && m_mauType[cpType - 1] == 2)
		//if (m_mauType[cpType - 1] == 2)
		{
			cpMau.enable();
		}
		else
		{
			cpMau.disable();
		}
	}

	CpMauM::OperationalStateT opState = cpMau.getOperationalState();

	ACS_CC_ReturnType result = this->updateRuntimeAttr(MausModel::getCpDn(cpType),
			MausModel::immAttr_operationalState, opState, false);

	return result;
}

void MausOI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation,
        const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
        ACS_APGCC_AdminOperationParamType** paramType)
{
    TRACE(("MausOI::adminOperationCallback(%lu, %lu, %s, %lu, paramList)", oiHandle, invocation,
            ((p_objName) ? p_objName : "NULL"), operationId));

    // This callback is for handling action. This MOM has no actions for now.
    // It had action, but removed after review.
    (void) paramType;
}

void MausOI::run()
{
    TRACE(("MausOI::run()"));

    if (!m_om.createAndSyncObject())
    {
    	TRACE_ERROR(("MausOI::run() failed to create OM"));
		EventReporter::instance().report(__LINE__,
				"Cannot create and sync CpMauM MO",
				"MausOI::run()");

    	return;
    }


    ACS_CC_ReturnType result = ACS_CC_SUCCESS;

    // Register as OI for Classic class.
    result = m_oiHandler.addClassImpl(this, MausModel::immCln_DualSidedCp.c_str());
    if (result != ACS_CC_SUCCESS)
    {
    	ostringstream os;
        os << "MausOI::run() failed to register class " << MausModel::immCln_DualSidedCp << endl;
        os << "Error code is " << getInternalLastError() << endl;
        os << "Error text is " << getInternalLastErrorText() << endl;

        TRACE(("%s", os.str().c_str()));
		EventReporter::instance().report(__LINE__,
				os.str().c_str(),
				"MausOI::run()");

        return;
    }

    // Update runtime operational state from MAUS
    // Continue even if it fails to update
    int admState = -1;
    m_om.readAdminState(admState, MausModel::Cp1);
    if (admState == 0 && m_mauType[0] == 2)
    {
    	TRACE(("MausOI::run(), update runtime for Cp1"));
    	if (updateRuntime(MausModel::Cp1))
    	{
    		TRACE_ERROR(("MausOI::run(), cannot update operationalState for CP1"));
    	}
    }
    else
    {
    	TRACE(("MausOI::run(), does not update runtime for Cp1"));
        this->updateRuntimeAttr(MausModel::getCpDn(MausModel::Cp1),
        		MausModel::immAttr_operationalState, 1, false);
    }

    if (MAUFunx::getDualSidedCpCount() == 2)
    {
    	m_om.readAdminState(admState, MausModel::Cp2);
    	if (admState == 0 && m_mauType[1] == 2)
    	{
        	TRACE(("MausOI::run(), update runtime for Cp2"));
    		if (updateRuntime(MausModel::Cp2))
    		{
    			TRACE_ERROR(("MausOI::run(), cannot update operationalState for CP2"));
    		}
    	}
        else
        {
        	TRACE(("MausOI::run(), does not update runtime for Cp2"));
            this->updateRuntimeAttr(MausModel::getCpDn(MausModel::Cp2),
            		MausModel::immAttr_operationalState, 1, false);
        }
    }

#if 1

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
			TRACE(("MausOI::run() poll error <%d>", errno));
			continue;
		}

		// terminating event
	    if (fds[0].revents & POLLIN )
	    {
	    	//cout << "ready to call msg queue received, msg len set to " << *msg_len << endl;
	    	fds[0].revents = 0;
	    	TRACE(("MausOI::run() detects end event"));
	    	stop = true;
	    	continue;
	    }

		// CpMauM MO has signaled
	    if (fds[1].revents & POLLIN )
	    {
	    	fds[1].revents = 0;
	    	TRACE(("MausOI::run() OI callback signals"));
            result = this->dispatch(ACS_APGCC_DISPATCH_ONE);

            if (result != ACS_CC_SUCCESS)
            {
                TRACE(("Failed to dispatch IMM event"));
        		EventReporter::instance().report(__LINE__,
        				"Failed to dispatch IMM event",
        				"MausOI::run()");
                stop = true;
            }
            continue;
	    }

		// CS has notified the changes in mauType. Note this change
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

#else
    // Master file descriptor list or event list
    fd_set masterFDs;
    // Temp file descriptor list
    fd_set readFDs;
    // The maximum FD in the master FD list
    int maxFD = 0;
    // Return value
    int retval;
    // Timeout value for select loop
    struct timeval tv;

    // Reset the FD list
    FD_ZERO(&masterFDs);
    FD_ZERO(&readFDs);

    // Add stop event into the master file descriptor.
    FD_SET(m_endEvent.get(), &masterFDs);

    // Add callback event of the OI into the master file descriptor.
    FD_SET(this->getSelObj(), &masterFDs);
    maxFD = max(m_endEvent.get(), this->getSelObj());

	bool stop = false;

	while (!stop)
	{
        readFDs = masterFDs;

        // Select with 3 secs timeout
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        retval = select(maxFD + 1, &readFDs, NULL, NULL, &tv);

		if (retval == -1)
		{
			// Just try again for now
			continue;
		}
		else if (retval)
		{

		    // shutdown event from cmw
	        if (FD_ISSET(m_endEvent.get(), &readFDs))
	        {
	            m_endEvent.reset();
		    	stop = true;
		    	TRACE(("MausOI::run() shutdown event"));
		    	//cout << "MausOI::run() shutdown event" << endl;
		    	continue;
	        }



	        // Receive Callback from the IMM
	        if (FD_ISSET(this->getSelObj(), &readFDs))
	        {
		    	TRACE(("MausOI::run() IMM callback signals"));
	            result = this->dispatch(ACS_APGCC_DISPATCH_ONE);

	            if (result != ACS_CC_SUCCESS)
	            {
	                TRACE(("Failed to dispatch IMM event"));
	        		EventReporter::instance().report(__LINE__,
	        				"Failed to dispatch IMM event",
	        				"MausOI::run()");
	                stop = true;
	            }
	        }

		}
	}
#endif

    // Clean up interaction with IMM
    m_oiHandler.removeClassImpl(this, MausModel::immCln_DualSidedCp.c_str());
    TRACE(("MausOI::run() returns"));

}

ACS_CC_ReturnType MausOI::updateRuntimeAttr(const std::string& objectName,
                                      const std::string& attrName, int value, bool useUnsigned)
{
    //TRACE(("MausOI::updateRuntimeAttr(%s, %d)", attrName.c_str(), value));

    ACS_CC_ImmParameter attrVal;

    // Update the attribute
    attrVal.attrName = (char *)attrName.c_str();
    attrVal.attrType = useUnsigned? ATTR_UINT32T : ATTR_INT32T;
    attrVal.attrValuesNum = 1;
    void* newValue[1] = {reinterpret_cast<void*>(&value)};

    attrVal.attrValues = newValue;

    if (this->modifyRuntimeObj(objectName.c_str(), &attrVal))
    {
        TRACE_ERROR(("Failed to modify runtime attribute %s. Error: %d - %s",
                attrName.c_str(),
                this->getInternalLastError(), this->getInternalLastErrorText()));

        return ACS_CC_FAILURE;
    }

    return ACS_CC_SUCCESS;
}

