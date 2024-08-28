/*
 * Maus_AehEvent.cpp
 *
 *  Created on: Nov 25, 2014
 *      Author: xdtthng
 */

#include <sstream>
#include "Maus_AehEvent.h"
#include "maus_trace.h"

const char* EventReporter::s_processName[MAX_PROC] = {"unknown", "cphw_maus1d", "cphw_maus2d",  "cphw_mauscd"};
const long	EventReporter::s_maus_event_base[MAX_PROC] = {0, 75000, 75100, 75200};

EventReporter::EventReporter(int n): m_instance(n), m_cs(), m_evRep()
{

}

void EventReporter::report(const int line,
		const char* problemData,
		const char* problemText,
		long eventNumber)
{
	using namespace std;

	boost::mutex::scoped_lock lock(m_cs);
	ostringstream procName, objRef;
	procName << getProcessName() << ":" << getpid();
	objRef << "Line/" << line;
	const long eventNum = getEventCode() + eventNumber;

	ACS_AEH_ReturnType ret = m_evRep.sendEventMessage(procName.str().c_str(),
				eventNum,
				"EVENT",
				"-",
				"APZ",
				objRef.str().c_str(),
				problemData,
				problemText);


	TRACE(("AEH Event, proc name <%s>, event number <%d>", procName.str().c_str(), eventNum));
	TRACE(("problemData <%s>", problemData));
	if (ret == ACS_AEH_error)
	{
		TRACE_ERROR(("acs_aeh_EH_EvReport error, code: %d", 0, m_evRep.getError()));
		TRACE_ERROR(("ACS error description: %s", 0, m_evRep.getErrorText()));
	}

}
