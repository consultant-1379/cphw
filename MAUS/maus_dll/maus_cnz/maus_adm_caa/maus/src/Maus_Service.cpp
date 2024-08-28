/*
 * Maus_Service.cpp
 *
 *  Created on: May 9, 2014
 *      Author: xdtthng
 */

//#include "halPlatformDefinitions.hxx"

#include "Maus_Service.h"
#include "maus_trace.h"

using namespace std;

// Use this to prevent another console application running during testing
// Will be removed during release
// Or it should be here permanently
int lock_application(const string& name);
int unlock_application(const string& name);
extern string s_lockName;
extern BoolFlagT s_apMausRunning;

Maus_Service::Maus_Service(const std::string& daemonName, int instance) :
		ACS_APGCC_ApplicationManager(daemonName.c_str()),
		//mRunning(0),
		m_instance(instance), m_daemonName(daemonName), m_maus()
{
}

Maus_Service::~Maus_Service()
{

}

void Maus_Service::start()
{

	TRACE_WARNING(("Maus_Service::run() activate %s", m_daemonName.c_str()));

	ACS_APGCC_HA_ReturnType errorCode = ACS_APGCC_HA_SUCCESS;

	errorCode = this->activate();

	if (errorCode == ACS_APGCC_HA_SUCCESS)
	{
		TRACE_WARNING(("Maus_Service::run(), HA Application gracefully closed!!"));
	}
	else if (errorCode == ACS_APGCC_HA_FAILURE)
	{
		TRACE_WARNING(("Maus_Service::run(), HA Application Failed to gracefully close!!"));
	}
	else
	{
		TRACE_WARNING(("Maus_Service::run(), HA Activation Failed!!"));
	}

	return;
}

#if 0
void Maus_Service::stop()
{
	mRunning = 0;
	return;
}

int Maus_Service::isRunning()
{
	return mRunning;
}
#endif

ACS_APGCC_ReturnType Maus_Service::performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
	TRACE_DEBUG(("Maus_Service::performStateTransitionToActiveJobs(<%d>)", previousHAState));

	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	if(ACS_APGCC_AMF_HA_ACTIVE == previousHAState)
	{
		TRACE_DEBUG(("Previous state = Active"));
	}
	else
	{
		if (lock_application(s_lockName) == 0)
		{
			//s_apMausRunning(false);
			m_maus.reset(new Maus_Application(m_instance, s_apMausRunning));
			if (!m_maus)
			{
				TRACE_DEBUG(("Application failed to create the instance"));
				rc = ACS_APGCC_FAILURE;
			}
			else
			{
				// Start point
				m_maus->start();
				//mRunning = 1;
			}
		}
		else {

			TRACE_DEBUG(("Application appears to have one instance running"));
			rc = ACS_APGCC_FAILURE;
		}

	}
	TRACE_DEBUG(("Maus_Service::performStateTransitionToActiveJobs() returns <%d>", rc));
	return rc;
}	// end of performStateTransitionToActiveJobs

ACS_APGCC_ReturnType Maus_Service::performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
	TRACE_DEBUG(("Maus_Service::performStateTransitionToQueisingJobs(<%d>)", previousHAState));

	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	// Inform the thread to go "stop" state
	if (m_maus)
	{
		m_maus->stop();
		m_maus.reset();
		unlock_application(s_lockName);
	}
	TRACE_DEBUG(("Maus_Service::performStateTransitionToQueisingJobs() returns <%d>", rc));
	return rc;
}	// end of performStateTransitionToQueisingJobs

ACS_APGCC_ReturnType Maus_Service::performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	TRACE_DEBUG(("Maus_Service::performStateTransitionToQuiescedJobs(<%d>)", previousHAState));

	// Inform the thread to go "stop" state
	if (m_maus)
	{
		m_maus->stop();
		m_maus.reset();
		unlock_application(s_lockName);
	}
	TRACE_DEBUG(("Maus_Service::performStateTransitionToQuiescedJobs() returns <%d>", rc));
	return rc;
}	// end of performStateTransitionToQuiescedJobs

ACS_APGCC_ReturnType Maus_Service::performComponentTerminateJobs(void)
{
	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	TRACE_DEBUG(("Maus_Service::performComponentTerminateJobs()"));

	// Inform the thread to go "stop" state
	if (m_maus)
	{
		m_maus->stop();
		m_maus.reset();
		unlock_application(s_lockName);
	}
	TRACE_DEBUG(("Maus_Service::performComponentTerminateJobs() returns <%d>", rc));
	return rc;
}	// end of performComponentTerminateJobs

ACS_APGCC_ReturnType Maus_Service::performComponentRemoveJobs(void)
{
	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	TRACE_DEBUG(("Maus_Service::performComponentRemoveJobs()"));

	// Inform the thread to go "stop" state
	if (m_maus)
	{
		m_maus->stop();
		m_maus.reset();
		unlock_application(s_lockName);
	}

	TRACE_DEBUG(("Maus_Service::performComponentRemoveJobs() returns <%d>", rc));
	return rc;
}	// end of performComponentRemoveJobs

ACS_APGCC_ReturnType Maus_Service::performApplicationShutdownJobs(void)
{
	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	TRACE_DEBUG(("Maus_Service::performApplicationShutdownJobs()"));

	// Inform the thread to go "stop" state
	if (m_maus)
	{
		m_maus->stop();
		m_maus.reset();
		unlock_application(s_lockName);
	}
	TRACE_DEBUG(("Maus_Service::performApplicationShutdownJobs() returns <%d>", rc));
	return rc;
}	// end of performApplicationShutdownJobs

ACS_APGCC_ReturnType Maus_Service::performComponentHealthCheck(void)
{
	static int count = 0;
	ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

	if (++count % 20 ==0)
		TRACE_DEBUG(("Maus_Service::performComponentHealthCheck()"));

	return rc;
}	// end of performApplicationShutdownJobs
