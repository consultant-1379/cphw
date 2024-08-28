/*
 * syscon_service.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: xdtthng
 */

#include "syscon_service.h"

using namespace std;

const char* Syscon_Service::s_daemonName = "cphw_syscond";

Syscon_Service::Syscon_Service() : ACS_APGCC_ApplicationManager(s_daemonName), m_syscon()
{
    TRACE(("SYSCON Run_as_service ==>> Syscon_Service::Syscon_Service() with daemon name <%s>", s_daemonName));
    TRACE(("Syscon_Service::Syscon_Service() returns"));
}

Syscon_Service::~Syscon_Service()
{
    TRACE(("Syscon_Service::~Syscon_Service()"));
    TRACE(("Syscon_Service::~Syscon_Service() returns"));
}

void Syscon_Service::run()
{
    TRACE(("Syscon_Service::run()"));

    ACS_APGCC_HA_ReturnType returnCode = this->activate();
    TRACE(("Syscon_Service::run() amf service loop returns code <%d>", returnCode));

    string msg;
    switch (returnCode) {
    case ACS_APGCC_HA_SUCCESS:
    	msg = "Gracefully closed";
    	break;
    case ACS_APGCC_HA_FAILURE:
    	msg = "HA Activation Failed";
    	break;
    default:
    	msg = "Failed to Gracefully close";
    	break;
    }
    TRACE(("%s", 0, msg.c_str()));
    TRACE(("Syscon_Service::run() returns"));

}

bool Syscon_Service::start()
{
	TRACE(("Syscon_Service::start()"));

	bool result = true;

	m_syscon.reset(new Syscon_Server);
	if (!m_syscon)
	{
		TRACE_DEBUG(("Application failed to create the instance"));
		result = false;
	}
	else
	{
		// Start point, this will return immediately
		m_syscon->start();
	}
	TRACE(("Syscon_Service::start() returns result <%d>", result));
	return result;
}

void Syscon_Service::stop()
{
    TRACE(("Syscon_Service::stop() returns"));

    if (m_syscon)
    {
    	m_syscon->stop();
    	m_syscon->join();
    	m_syscon.reset();
    }

	TRACE(("Syscon_Service::stop() returns"));
}


ACS_APGCC_ReturnType Syscon_Service::performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("Syscon_Service::performStateTransitionToActiveJobs(%d)", previousHAState));

    ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

    // Previous state is ACTIVE, no need to process again
    if (ACS_APGCC_AMF_HA_ACTIVE == previousHAState)
    {
    	TRACE(("Syscon_Service::performStateTransitionToActiveJobs() returns <%d>, no action, already active", rc));
        return rc;
    }

	if (!this->start())
	{
		TRACE(("Application failed to create the instance"));
		rc = ACS_APGCC_FAILURE;
	}

	TRACE(("Syscon_Service::performStateTransitionToActiveJobs() returns <%d>", rc));

    return rc;
}

ACS_APGCC_ReturnType Syscon_Service::performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("MauscService::performStateTransitionToPassiveJobs(%d)", previousHAState));

    // Do nothing

    TRACE(("Syscon_Service::performStateTransitionToPassiveJobs() returns; take no action"));
    return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType Syscon_Service::performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("Syscon_Service::performStateTransitionToQueisingJobs(%d)", previousHAState));

    this->stop();

    TRACE(("Syscon_Service::performStateTransitionToQueisingJobs() returns ACS_APGCC_SUCCESS"));
    return ACS_APGCC_SUCCESS;

}

ACS_APGCC_ReturnType Syscon_Service::performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("Syscon_Service::performStateTransitionToQuiescedJobs(%d)", previousHAState));

    this->stop();

    TRACE(("Syscon_Service::performStateTransitionToQuiescedJobs() returns ACS_APGCC_SUCCESS"));
    return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType Syscon_Service::performComponentHealthCheck(void)
{
	TRACE(("Syscon_Service::performComponentHealthCheck()"));
    ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

    return rc;
}

ACS_APGCC_ReturnType Syscon_Service::performComponentTerminateJobs(void)
{
    TRACE(("Syscon_Service::performComponentTerminateJobs(void)"));

    // Application has received terminate component callback due to
    // LOCK-INST admin opreration perform on SU.

    this->stop();

    TRACE(("Syscon_Service::performComponentTerminateJobs() returns ACS_APGCC_SUCCESS"));
    return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType Syscon_Service::performComponentRemoveJobs(void)
{
	TRACE(("Syscon_Service::performComponentRemoveJobs(void)"));

    // Stop server
    this->stop();

    TRACE(("Syscon_Service::performComponentRemoveJobs() returns ACS_APGCC_SUCCESS"));
    return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType Syscon_Service::performApplicationShutdownJobs(void)
{
    TRACE(("Syscon_Service::performApplicationShutdownJobs(void)"));

    this->stop();

    TRACE(("Syscon_Service::performApplicationShutdownJobs() returns ACS_APGCC_SUCCESS"));
    return ACS_APGCC_SUCCESS;
}


