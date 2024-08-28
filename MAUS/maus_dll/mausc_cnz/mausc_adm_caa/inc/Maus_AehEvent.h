/*
 * Maus_AehEvent.h
 *
 *  Created on: Nov 25, 2014
 *      Author: xdtthng
 */

/*
NAME
   File_name: Maus_AehEvent.h

COPYRIGHT Ericsson AB, Sweden 2015. All rights reserved.

   The Copyright to the computer program(s) herein is the property of Ericsson AB, Sweden.
   The program(s) may be used and/or copied only with the written permission from
   Ericsson AB or in accordance with the terms and conditions
   stipulated in the agreement/contract under which the program(s) have been
   supplied.

DESCRIPTION

   This class provide the wrapper for interfacing to ACS AEH Event Handling

DOCUMENT NO
   190 89-CAA 109

AUTHOR
   2014-11-25 by DEK/XDTTHNG Thanh Nguyen

SEE ALSO


Revision history
----------------
2014-11-25 Thanh Nguyen Created

*/


#ifndef MAUS_AEHEVENT_H_
#define MAUS_AEHEVENT_H_

#include <boost/thread/mutex.hpp>
#include "acs_aeh_evreport.h"

class EventReporter
{
public:
	enum {
		MAX_PROC = 4
	};

	// Singleton class
	static EventReporter& instance()
	{
		return s_instance;
	}

	// Write an AEH event
	void report(const int line,
			const char* problemData,
			const char* problemText,
			long eventNumber = 0);

private:

	// Construction of the AEH interface
	EventReporter(int n);

	// Copying and assignment are not supported
	EventReporter(const EventReporter&);
	EventReporter& operator=(const EventReporter&);

	static EventReporter 	s_instance;
	static const char*		s_processName[MAX_PROC];
	static const long		s_maus_event_base[MAX_PROC];

	int						m_instance;
	boost::mutex  			m_cs;
	acs_aeh_evreport 		m_evRep;

	const char*	getProcessName()
	{
		return s_processName[m_instance];
	}

	long getEventCode()
	{
		return s_maus_event_base[m_instance];;
	}
};

#endif // MAUS_AEHEVENT_H_
