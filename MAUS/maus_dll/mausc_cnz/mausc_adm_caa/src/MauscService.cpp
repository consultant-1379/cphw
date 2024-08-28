/*
NAME
    File_name: MauscService.cpp

Ericsson AB

    COPYRIGHT Ericsson AB, Sweden 2000. All rights reserved.

    The Copyright to the computer program(s) herein is the property of
    Ericsson AB, Sweden. The program(s) may be used and/or
    copied only with the written permission from Ericsson AB
    or in accordance with the terms and conditions stipulated in the
    agreement/contract under which the program(s) have been supplied.

DESCRIPTION

DOCUMENT NO
    190 89-CAA 109

AUTHOR
    2015-02-11 by XDT/DEK xdtthng


SEE ALSO
    -

Revision history
----------------
2015-02-11 xdtthng Created

*/


#include "boost/thread/mutex.hpp"
#include "boost/thread/condition_variable.hpp"
#include <boost/thread/thread.hpp>

#include "MauscService.h"
#include "maus_trace.h"
#include "Maus_AehEvent.h"

const char *MauscService::s_daemonName = "cphw_mauscd";


MauscService::MauscService() : ACS_APGCC_ApplicationManager(s_daemonName),
		m_oi(),
		m_observer(),
		m_maServer()
{
	// No trace in constructor
}

MauscService::~MauscService()
{
    TRACE(("MauscService::~MauscService()"));
}

void MauscService::run()
{
    TRACE(("MauscService::run()"));

    ACS_APGCC_HA_ReturnType returnCode = this->activate();
    TRACE(("MauscService::run() amf service loop returns code <%d>", returnCode));

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
    TRACE(("MauscService::run() returns"));

}


bool MauscService::start()
{
	TRACE(("MauscService::start()"));


	// Start TCP Server first
	// ----------------------
	m_maServer.reset(new MAServer);
	m_maServer->start();
	m_maServer->waitUntilRunning();

	// Remove the trace after basic test
	// ---------------------------------
	TRACE(("Event file descriptor [0] <%d>", m_change[0].get()));
	TRACE(("Event file descriptor [1] <%d>", m_change[1].get()));
	TRACE(("Event file descriptor [2] <%d>", m_change[2].get()));
	TRACE(("Event file descriptor [3] <%d>", m_change[3].get()));

	// Construct a table observer and populate it with required information
	int architecture;
	int mauType[2];
	SysInfo::getSysInfo(architecture, mauType);
	m_observer.reset(new CpTableObserver(architecture, mauType, m_change));
	if (!m_observer)
	{

		TRACE_ERROR(("MauscService::start(), failed to get memory for CpTableObserver"));
		EventReporter::instance().report(__LINE__,
				"Failed to get memory for CpTableObserver",
				"MauscService::start()");

		// See comment at the end of this method
		// return true;
	}
	else
	{
		m_observer->init();
	}

	m_oi.reset(new SwMauOI(architecture, mauType, m_change));
	if (!m_oi)
	{

		TRACE_ERROR(("MauscService::start(), failed to get memory for CpMauM OI"));
		EventReporter::instance().report(__LINE__,
				"Failed to get memory for CpMauM O",
				"MauscService::start()");

		// See comment at the end of this method
		return true;
	}


	if (!m_oi->start())		// At the moment MausOI::start() return 0
	{
		TRACE(("MauscService::start(), CpMauM OI is started successfully"));
	}
	else
	{
		TRACE_ERROR(("MauscService::start(), failed to start CpMauM OI"));
		EventReporter::instance().report(__LINE__,
				"Failed to start CpMauM OI",
				"MauscService::start()");
	}

	// For now always return true
	// Error is reported in TRACE and to Aeh event log
	return true;
}

void MauscService::stop()
{
    TRACE(("MauscService::stop() returns"));

    if (m_maServer)
    {
    	m_maServer->stop();
    	m_maServer.reset();
    }

	if (m_oi)
	{
		m_oi->stop();
		m_oi.reset();
	}

	if (m_observer)
	{
		m_observer.reset();
	}
}


ACS_APGCC_ReturnType MauscService::performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("MauscService::performStateTransitionToActiveJobs(%d)", previousHAState));

    ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

    // Previous state is ACTIVE, no need to process again
    if (ACS_APGCC_AMF_HA_ACTIVE == previousHAState)
    {
        return rc;
    }

	if (!this->start())
	{
		TRACE(("Application failed to create the instance"));
		rc = ACS_APGCC_FAILURE;
	}

	TRACE(("MauscService::performStateTransitionToActiveJobs() returns <%d>", rc));

	// This is a work around for now
	//rc = ACS_APGCC_SUCCESS;
    return rc;
}

ACS_APGCC_ReturnType MauscService::performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("MauscService::performStateTransitionToPassiveJobs(%d)", previousHAState));

    // Do nothing

    return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType MauscService::performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("MauscService::performStateTransitionToQueisingJobs(%d)", previousHAState));

    this->stop();

    return ACS_APGCC_SUCCESS;

}

ACS_APGCC_ReturnType MauscService::performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
    TRACE(("MauscService::performStateTransitionToQuiescedJobs(%d)", previousHAState));

    this->stop();
    
    return ACS_APGCC_SUCCESS;
}


ACS_APGCC_ReturnType MauscService::performComponentHealthCheck(void)
{
	TRACE(("MauscService::performComponentHealthCheck()"));
    ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

    return rc;
}


ACS_APGCC_ReturnType MauscService::performComponentTerminateJobs(void)
{
    TRACE(("MauscService::performComponentTerminateJobs(void)"));

    // Application has received terminate component callback due to
    // LOCK-INST admin opreration perform on SU.

    this->stop();

    return ACS_APGCC_SUCCESS;
}


ACS_APGCC_ReturnType MauscService::performComponentRemoveJobs(void)
{
    //newTRACE((LOG_LEVEL_INFO, "MauscService::performComponentRemoveJobs(void)", 0));

    // Application has received Removal callback. State of the application
    // is neither Active nor Standby. This is with the result of LOCK admin operation
    // performed on our SU.

    
    // Stop server
    this->stop();

    return ACS_APGCC_SUCCESS;
}


ACS_APGCC_ReturnType MauscService::performApplicationShutdownJobs(void)
{
    TRACE(("MauscService::performApplicationShutdownJobs(void)"));

    this->stop();

    return ACS_APGCC_SUCCESS;
}
