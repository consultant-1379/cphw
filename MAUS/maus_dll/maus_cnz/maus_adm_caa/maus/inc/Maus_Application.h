/*
 * Maus_Application.h
 *
 *  Created on: May 18, 2014
 *      Author: xdtthng
 */

/*
NAME
   File_name: Maus_Application.h

COPYRIGHT Ericsson AB, Sweden 2015. All rights reserved.

   The Copyright to the computer program(s) herein is the property of Ericsson AB, Sweden.
   The program(s) may be used and/or copied only with the written permission from
   Ericsson AB or in accordance with the terms and conditions
   stipulated in the agreement/contract under which the program(s) have been
   supplied.

DESCRIPTION

   This container handles "Active objects" for all applications. These objects are
   - ApMaus which is a MAUBase object
   - SystemInformation object
   - CommandServer Object
   - GMServer object.

   CpMauM operationalState is implemented by a global variable s_apMausRunning defined in main.
   s_apMausRunning is used by both Maus_Application and GMServer to indicate if ApMaus is
   started by Maus_Application.
   - s_apMausRunning is written to and read from by Maus_Application
   - s_apMausRunning read by GMServer. This information is passed to CpMauM MO

DOCUMENT NO
   190 89-CAA 109

AUTHOR
   2014-05-18 by DEK/XDTTHNG Thanh Nguyen

SEE ALSO
   GMServer.h

Revision history
----------------
2014-05-18 Thanh Nguyen Created

*/

#ifndef MAUS_APPLICATION_H_
#define MAUS_APPLICATION_H_

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "boost/shared_ptr.hpp"



#include "ApMaus.h"
#include "maus_event.h"
#include "Maus_SystemInformation.h"
#include "GMServer.h"
#include "AtomicFlag.h"
#include "MAUMsgQ.h"


class Maus_Application
{
public:

	enum OperStateT
	{
		LOCKED = 0,
		UNLOCKED = 1
	};

	Maus_Application(int instance, BoolFlagT& mausRunning);
	~Maus_Application();

	// Start all applications
	int start();

	// Stop all applications
	int stop();

private:

	// CMW event to stop all application threads
	Maus_Event	m_cmwEndEvent;

	// Application thread
	boost::thread	m_appThread;

	// System information thread
	boost::thread	m_sysInfoThread;

	// MAUBase thread
	boost::scoped_ptr<ApMaus> m_apMaus;
	int				m_instance;

	// Reading system configuration
	boost::scoped_ptr<SystemInformation> m_sysInfo;

	// Listens to message from CpMauM MO
	boost::scoped_ptr<GMServer> m_gmServer;

	// Reference to a global variable defined in main
	// This is commonly used by Maus_Application and GMServer
	// This flag indicates if ApMaus is started/stopped by Maus_Application
	BoolFlagT&					m_apMausRunning;

	// This mutex is to create blocking shutdown and initialisation of mau core
	boost::mutex				m_cs;


	// While the thread might be blocked by a mutex, check this to abort
	// This is used solely by Maus_Application run loop. It is set when receives
	// stop event from cmw.
	// extern BoolFlagT	s_applicationAborting;


	// Events from CpMauM MO
	Maus_Event		m_enableEvent;
	Maus_Event		m_disableEvent;

	// Message Queue for inter thread communication between Application and GMServer
	MAUMsgQ 		m_inQ;
	MAUMsgQ 		m_outQ;

	bool			m_queueInitialised;
	size_t  		m_maxMsgSize;
	char*   		m_msg;
	BoolFlagT		m_apMausAborting;

	char			m_buffer[MAUMsgQ::MaxMsgSize];
	char			m_buffer_reject[MAUMsgQ::MaxMsgSize];

	// This is the main event processing loop
	void run();

	// Start ApMaus only
	int startApMaus(bool checksum = false);

	// Stop ApMaus only
	int stopApMaus();

	// Stop all threads started by Maus_Application
	// Evoked from cmw stop event
	int stopApp();

	// Handling of DSD messages from GMServer
	void unknownMsg();
	void getOperStatusMsg();
	void getQuorumInfoMsg();
	void setActiveMaskMsg();
	void performChecksumMsg();
	void setFCStateMsg();
	void installMibs();
	void rejectMsg(int cause);

	typedef void (Maus_Application::*MsgProcFuncT)();


	static const int	UNKNOWN 			= 0;	// Unknown function index
	static const int	GET_OPER_STATUS 	= 1;
	static const int	GET_QUORRUM_INFO 	= 2;
	static const int	SET_ACTIVE_MASK 	= 3;
	static const int	PERFORM_CHECKSUM 	= 4;
	static const int	SET_FC_STATE 		= 5;
	static const int	INSTALL_MIBS 		= 6;

	static const int 	MIN_MSG_INDEX = GET_OPER_STATUS;
	static const int	MAX_MSG_INDEX = INSTALL_MIBS;

	static const std::string DSD_SERVER_NAME[3];
	MsgProcFuncT	m_processMsg[MAX_MSG_INDEX + 1];
};


#endif /* MAUS_APPLICATION_H_ */
