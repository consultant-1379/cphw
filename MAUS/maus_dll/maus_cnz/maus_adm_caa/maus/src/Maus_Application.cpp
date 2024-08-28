/*
 * Maus_Application.cpp
 *
 *  Created on: May 18, 2014
 *      Author: xdtthng
 */


#include <fstream>
#include <sstream>

#include <sys/poll.h>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <string>
#include <new>

#include "Maus_Application.h"
#include "Maus_SystemInformation.h"
#include "maus_trace.h"
#include "MAUFunx.h"
#include "Maus_AehEvent.h"
#include "MAUMsgQNames.h"
#include "MauMsg.h"
#include "maus_sha.h"

using namespace std;
extern BoolFlagT s_applicationAborting;
extern BoolFlagT s_abortChecksum;
extern Maus_Event s_gShutdown;
extern BoolFlagT s_apMausRunning;


// This is not used for production code.
// It is used during proof of concept
extern ApMaus* g_ApMaus;

// DSD Domain is MAUS, names are CP1MAUM and CP2MAUM
const std::string Maus_Application::DSD_SERVER_NAME[3] = {"MAUS", "CP1MAUM", "CP2MAUM"};

Maus_Application::Maus_Application(int instance, BoolFlagT& mausRunning):
			m_cmwEndEvent(),
			m_appThread(),
			m_sysInfoThread(),
			m_apMaus(),
			m_instance(instance),
			m_sysInfo(),
			m_gmServer(),
			m_apMausRunning(mausRunning),
			m_cs(),
			m_enableEvent(),
			m_disableEvent(),
			m_inQ(m_instance,MSGQ_NAME),
			m_outQ(m_instance,MSGQ_NAME),
			m_queueInitialised(false),
			m_maxMsgSize(0),
			m_msg(0),
			m_apMausAborting(false)
{
	TRACE_DEBUG(("Maus_Application::Maus_Application()"));

	//m_processMsg[0] = &Maus_Application::unknownMsg;
	//m_processMsg[1] = &Maus_Application::getOperStatusMsg;
	//m_processMsg[2] = &Maus_Application::getQuorumInfoMsg;
	//m_processMsg[3] = &Maus_Application::setActiveMaskMsg;

	m_processMsg[UNKNOWN] = &Maus_Application::unknownMsg;
	m_processMsg[GET_OPER_STATUS] = &Maus_Application::getOperStatusMsg;
	m_processMsg[GET_QUORRUM_INFO] = &Maus_Application::getQuorumInfoMsg;
	m_processMsg[SET_ACTIVE_MASK] = &Maus_Application::setActiveMaskMsg;
	m_processMsg[PERFORM_CHECKSUM] = &Maus_Application::performChecksumMsg;
	m_processMsg[SET_FC_STATE] = &Maus_Application::setFCStateMsg;
	m_processMsg[INSTALL_MIBS] = &Maus_Application::installMibs;
}

Maus_Application::~Maus_Application()
{
	TRACE_DEBUG(("Maus_Application::~Maus_Application()"));

	if (m_queueInitialised)
	{
		m_inQ.destroy(true);
		m_outQ.destroy(true);
		m_queueInitialised = false;
	}

	TRACE_DEBUG(("Maus_Application::~Maus_Application() returns"));
}

int Maus_Application::start()
{
	TRACE_DEBUG(("Maus_Application::start() from cmw"));

	// Start these global variables
	s_applicationAborting(false);
	s_abortChecksum(false);
	s_apMausRunning(false);
	s_gShutdown.reset();

	m_appThread = boost::thread(boost::bind(&Maus_Application::run, this));
	TRACE_DEBUG(("Maus_Application::start() returns"));
	return 0;
}

int Maus_Application::stop()
{
	TRACE_DEBUG(("Maus_Application::stop() from cmw"));

	s_applicationAborting(true);
	m_cmwEndEvent.set();
	s_gShutdown.set();

	// Wait for application thread to join
	if (m_appThread.joinable())
	{
		TRACE_DEBUG(("Maus_Application::stop() wait for application thread to join"));
		m_appThread.join();
	}

	// Move to start
	//s_applicationAborting(false);

	TRACE_DEBUG(("Maus_Application::stop() returns"));
	return 0;
}

void Maus_Application::run()
{
	TRACE_DEBUG(("Maus_Application::run()"));

	// Collect system information
	// --------------------------
	// Do not start the application if required information cannot be obtained
	// ///////////////////////////////////////////////////////////////////////

	mqd_t fdMsgQ;
	struct pollfd 	fds[4];
	int adminState;
	int mauType;

	// Wait for this thread to join before proceeding.
	m_sysInfo.reset(new SystemInformation(m_instance, m_cmwEndEvent.get()));
	m_sysInfoThread = boost::thread(boost::bind(&SystemInformation::run, m_sysInfo.get()));
	TRACE_DEBUG(("Maus_Application::run(), wait for join of sysInfoThread"));
	if (m_sysInfoThread.joinable())
	{
		m_sysInfoThread.join();
		TRACE_DEBUG(("Maus_Application::run() sysInfoThread joins"));
	}

	// System information collection is aborted
	if (m_sysInfo->m_shutdown)
	{
		TRACE_DEBUG(("Maus_Application::run() sys info collection is aborted"));
		//s_applicationAborting(false);
		return;
	}

	// Check if classic CP
	if (m_sysInfo->m_classicCpSystem)
	{
		TRACE_DEBUG(("Maus_Application::run() classic cp returns"));
		//s_applicationAborting(false);
		return;
	}

	// MAUS2 does not run for one cp system
	// or multiple cp system with only one dualsided CP defined
	if (m_instance == 2 && !m_sysInfo->m_multiCpSystem)
	{
		TRACE_WARNING(("Maus_Application::run() returns; not mcp "));
		//s_applicationAborting(false);
		return;
	}

	// Check for full initialization.
	// If no fully initialized, do not start ApMaus
	if (!m_sysInfo->m_initialised)
	{
		TRACE_ERROR(("Maus_Application::run() sys info is not fully initialized"));
		//s_applicationAborting(false);
		return;
	}

	// Start Application
	// -----------------
	// Create message queue
	// Start ApMaus
	// Start GMServer
	// ////////////////////

	// Create message queue
	m_maxMsgSize = MAUMsgQ::getMaxMsgSize();
	MauReturnCode::Return reti = m_inQ.create(MAUMsgQ::LocalMAUInboundQ);
	TRACE_DEBUG(("Maus_Application::Maus_Application() creates inQ with result <%d>", reti));

	MauReturnCode::Return reto = m_outQ.create(MAUMsgQ::LocalMAUOutboundQ);
	TRACE_DEBUG(("Maus_Application::Maus_Application() creates outQ with result <%d>", reto));
	m_queueInitialised = MauReturnCode::OK == reto || reti == MauReturnCode::OK;

	// Start ApMaus
	// Start only if mauType == MAUS and administrativeState == Enable
	if (m_sysInfo->m_mauType == 2 && m_sysInfo->m_adminState == UNLOCKED)
	{
		// If it is aborting then returns
		if (s_applicationAborting)
		{
			TRACE_DEBUG(("Maus_Application::run() detects Enable event, abort due to cmw stop"));
			return;
		}

		// Start ApMaus with checksum calculation
		this->startApMaus(true);
	}

	// Start GMServer

	m_gmServer.reset(new GMServer(DSD_SERVER_NAME[m_instance].c_str(), DSD_SERVER_NAME[0].c_str(), m_instance,
			m_apMausRunning, m_enableEvent, m_disableEvent));
	m_gmServer->start();

	// Cmw stop event from AMF
	fds[0].fd = m_cmwEndEvent.get();
	fds[0].events = POLLIN;

	// administrativeState == Enabled from CpMauM MO
	fds[1].fd = m_enableEvent.get();
	fds[1].events = POLLIN;

	// administrativeState == Disabled from CpMauM MO
	fds[2].fd = m_disableEvent.get();
	fds[2].events = POLLIN;

	// Messages from GMServer for all messages except for
	// Enable and Disable
	fdMsgQ = m_inQ.getFileDescriptor();
	TRACE_DEBUG(("Maus_Application::run() get file descriptor <%d>", fdMsgQ));
	fds[3].fd = fdMsgQ;
	fds[3].events = POLLIN;

	bool stop = false;
	int res;
	while (!stop)
	{
		//res = poll(fds, 3, 10000);
		res = poll(fds, 4, -1);

		if (res == 0)
		{
			// Time out

			continue;
		}

		if (res < 0)
		{
			if (s_applicationAborting)
			{
				TRACE_DEBUG(("Maus_Application::run() poll returns negative during abort"));
				stop = true;
			}
			TRACE_DEBUG(("Maus_Application::run() poll error <%d>", errno));
			continue;
		}

		// terminating event
		if (fds[0].revents & POLLIN )
		{
			// Stop the Application thread
			fds[0].revents = 0;
			TRACE_DEBUG(("Maus_Application::run() detects end event"));
			s_abortChecksum(false);
			this->stopApp();
			stop = true;
			continue;
		}

		// administrativeState == Enabled from CpMauM MO
		// mauType changes from MAUB to MAUS in Native
		if (fds[1].revents & POLLIN )
		{
			TRACE_DEBUG(("Maus_Application::run() detects Enable event"));
			fds[1].revents = 0;
			m_enableEvent.reset();

			if (m_instance == 2 )
			{
				if (!m_sysInfo->m_multiCpSystem)
				{
					TRACE_ERROR(("Maus_Application::run() returns; not mcp "));
					continue;
				}
				else if (m_sysInfo->m_cpCount != 2)
				{
					//TR_HY60540: Due to race condition, sometimes m_cpCount is not correctly set.
					//Hence fetching the dualsided cp count again 
					if ( (m_sysInfo->m_cpCount = MAUFunx::getDualSidedCpCount()) != 2)
					{
						TRACE_ERROR(("Maus_Application::run() returns; no cp2 "));
						continue;
					}
				}
			}


			if (m_sysInfo->m_infrastructure == 3)
			{
				mauType = 2;	// This is MAUS
			}
			else
			{
				mauType = MAUFunx::getMauType(m_instance);
			}

			//adminState = MAUFunx::getCpMauMAdminState(m_instance);
			adminState = MAUFunx::getSwMauAdminState(m_instance);

			// Will be removed later
			//m_sysInfo->m_mausInformation.fcState = MAUFunx::getCpMauMFCState(m_instance);
			m_sysInfo->m_mausInformation.fcState = -1;

			bool running = m_apMausRunning;
			TRACE_DEBUG(("Maus_Application::run() enable event, m_apMausRunning <%d>", running));
			TRACE_DEBUG(("Maus_Application::run() enable event, mauType <%d>", mauType));
			TRACE_DEBUG(("Maus_Application::run() enable event, adminState <%d>", adminState));

			// if ApMaus is not running and the conditions are ok then attempt to run ApMaus
			if (!m_apMausRunning && (mauType == 2 && adminState == UNLOCKED))
			{
				// If it is aborting then returns
				// While checksum is running or being aborted, there should not be an enable
				if (s_applicationAborting)
					//if (s_applicationAborting || s_abortChecksum)
				{
					TRACE_DEBUG(("Maus_Application::run() detects Enable event, abort due to cmw stop"));

					// Return immediately if cmw stops the application
					continue;
				}

				this->startApMaus();

			}

			TRACE_DEBUG(("Maus_Application::run() finish handling Enable event"));
			continue;
		}

		// administrativeState == Disabled from CpMauM MO
		// mauType changes from MAUS to MAUB in Native
		// unload from cfeted
		if (fds[2].revents & POLLIN )
		{
			fds[2].revents = 0;
			TRACE_DEBUG(("Maus_Application::run() detects Disable event"));
			m_disableEvent.reset();

			// If the shutting down of application is already started in cmw thread
			// then stop this run thread asap
			s_abortChecksum(true);
			if (s_applicationAborting)
			{
				TRACE_DEBUG(("Maus_Application::run() detects Disable event, abort due to cmw stop"));

				// Return immediately if cmw stops the application
				//s_abortChecksum(false);
				// set to false at stop event raised
				continue;
			}

			if (m_apMausRunning && !m_apMausAborting)
			{
				// If cmw is aborting then returns
				this->stopApMaus();
			}
			else
			{
				TRACE_DEBUG(("Maus_Application::run() detects Disable event, no action, maus already or being stopped"));
			}
			TRACE_DEBUG(("Maus_Application::run() finish handling Disable event"));
			s_abortChecksum(false);
			continue;
		}

		// Message Handling
		// Possible messages are
		// - Check for state
		// - read quorum information for mauls command
		// - set mask (from mauset)
		// - perform checksum
		// - set function change state
		// - install Mibs
		// refer to Maus_ApConn.h
		//
		if (fds[3].revents & POLLIN )
		{
			fds[3].revents = 0;

			TRACE_DEBUG(("Maus_Application::run() detects message from GMServer"));

			// If there is a message coming in while there is a shutdown, immediately
			// response to the message indicating that the system in shutting down
			//
			if (s_applicationAborting)
			{
				TRACE_DEBUG(("Maus_Application::run() detects message from GMServer, abort due to cmw stop"));
				rejectMsg(MauMsg::SystemShutdown);

				continue;
			}

			// Try to get a lock
			boost::interprocess::scoped_lock<boost::mutex> alock(m_cs, boost::interprocess::try_to_lock);
			if (!alock)
			{
				TRACE_DEBUG(("Maus_Application::run() detects message from GMServer, reject with function busy"));
				rejectMsg(MauMsg::ApplicationBusy);

				continue;
			}

			size_t size = MAUMsgQ::getMaxMsgSize();
			int res = m_inQ.receive(m_buffer, &size);
			TRACE_DEBUG(("Result from reading the message <%d>, 1 == OK", res));

			int* info = ::new (m_buffer) int[2];

			// Mapping message id info[1], 1, 3, 5, 7 to
			// Message index               1, 2, 3, 4, ... for indexing into an array
			// Message index = Message Id / 2 + 1.

			int msgIndex = (info[1] >> 1) + 1;

			TRACE_DEBUG(("Maus_Application::run() msgId is <%d>", info[1]));
			TRACE_DEBUG(("Maus_Application::run() msg index is <%d>", msgIndex));

			if (msgIndex < MIN_MSG_INDEX || msgIndex > MAX_MSG_INDEX)
			{
				(this->*m_processMsg[0])();
			}
			else
			{
				(this->*m_processMsg[msgIndex])();
			}

			TRACE_DEBUG(("Maus_Application::run() complete processing message from GMServer"));

			continue;
		}

	}
	//s_applicationAborting(false);
	TRACE_DEBUG(("Maus_Application::run() returns"));
}


int Maus_Application::startApMaus(bool checksum)
{
	TRACE_DEBUG(("Maus_Application::startApMaus()"));

	//boost::mutex::scoped_lock alock(m_cs);
	boost::lock_guard<boost::mutex> alock(m_cs);

	// For now
	if (m_apMaus)
	{
		TRACE_DEBUG(("Maus_Application::startApMaus(), instance already started, no action taken"));
		return 0;
	}

#if 0
	if (!m_queueInitialised)
	{
		// Create message queue
		m_maxMsgSize = MAUMsgQ::getMaxMsgSize();
		MauReturnCode::Return reti = m_inQ.create(MAUMsgQ::LocalMAUInboundQ);
		TRACE_DEBUG(("Maus_Application::Maus_Application() creates inQ with result <%d>", reti));

		MauReturnCode::Return reto = m_outQ.create(MAUMsgQ::LocalMAUOutboundQ);
		TRACE_DEBUG(("Maus_Application::Maus_Application() creates outQ with result <%d>", reto));
		m_queueInitialised = MauReturnCode::OK == reto || reti == MauReturnCode::OK;

		//if (m_gmServer)
		//{
		//	TRACE_DEBUG(("Maus_Application::startApMaus() flushing message queue"));
		//	m_gmServer->flushMsgQueue();
		//}
	}
#endif


	// Start ApMaus and Command Server
	m_cmwEndEvent.reset();

	// Start a new ApMaus
	m_apMaus.reset(new ApMaus(m_sysInfo->m_mausInformation, m_instance, m_sysInfo->m_slotId));
	if (!m_apMaus->loadSo(checksum))
	{
		TRACE_ERROR(("Maus_Application::startApMaus() failed to load so file, clear apMaus"));
		m_apMaus.reset();
		return 0;
	}
	m_apMaus->start();
	g_ApMaus = m_apMaus.get();

	// Set the global flag indicating Ap Maus is really running
	m_apMausRunning(true);

	TRACE_DEBUG(("Maus_Application::startApMaus() returns"));
	return 0;
}


int Maus_Application::stopApp()
{
	TRACE_DEBUG(("Maus_Application::stopApp()"));

	boost::lock_guard<boost::mutex> alock(m_cs);

	if (m_gmServer)
	{
		m_gmServer->stop();
		m_gmServer->join();
		m_gmServer.reset();
	}
	// There are two threads closing this application
	// This is from cmw. A lock is held for this operation
	if (m_apMausRunning)
	{
		if (m_apMaus)
		{
			m_apMaus->stop();
			m_apMaus.reset();
			g_ApMaus = 0;
		}
		m_apMausRunning(false);
	}

	TRACE_DEBUG(("Maus_Application::stopApp() returns"));
	return 0;
}

int Maus_Application::stopApMaus()
{
	TRACE_DEBUG(("Maus_Application::stopApMaus()"));

	//boost::mutex::scoped_lock alock(m_cs);
	boost::lock_guard<boost::mutex> alock(m_cs);
	m_apMausAborting(true);

#if 0
	TRACE_DEBUG(("Maus_Application::stopApMaus() attempt to remove msg queue"));
	if (m_queueInitialised)
	{
		TRACE_DEBUG(("Maus_Application::stopApMaus() destroys remove msg queue"));
		m_inQ.destroy(true);
		m_outQ.destroy(true);
		m_queueInitialised = false;
	}
#endif

	TRACE_DEBUG(("Maus_Application::stopApMaus() stops ApMaus"));

	// There are two threads closing this application
	// This is from CpMauM. A lock is held for this operation
	if (m_apMausRunning)
	{
		if (m_apMaus)
		{
			m_apMaus->stop();
			m_apMaus.reset();
			g_ApMaus = 0;

		}
		m_apMausRunning(false);
	}

	m_apMausAborting(false);
	TRACE_DEBUG(("Maus_Application::stopApMaus() returns"));
	return 0;
}


void Maus_Application::unknownMsg()
{
	TRACE_DEBUG(("Maus_Application::unknownMsg()"));

	int* info = ::new (m_buffer) int[4];
	info[0] = 4;			// Length
	info[1] = info[1] + 1;	// Return message id
	info[2] = 0;			// Oper state == OK
	info[3] = 0;			// returned value
	int res = m_outQ.send(m_buffer, 16);
	TRACE_DEBUG(("Maus_Application::unknownMsg(), msg send() returns <%d>", res));

	TRACE_DEBUG(("Maus_Application::unknownMsg() returns"));
}

// This message is not used in production code
// It is used during proof of concept
void Maus_Application::getOperStatusMsg()
{
	TRACE_DEBUG(("Maus_Application::getOperStatusMsg()"));

	int* info = ::new (m_buffer) int[4];
	info[0] = 4;			// Length
	info[1] = info[1] + 1;	// Return message id
	info[2] = 0;			// Oper state == OK
	info[3] = 9999;			// returned value
	int res = m_outQ.send(m_buffer, 16);
	TRACE_DEBUG(("Maus_Application::getOperStatusMsg(), msg send() returns <%d>", res));

	TRACE_DEBUG(("Maus_Application::getOperStatusMsg() returns"));
}

void Maus_Application::setActiveMaskMsg()
{
	TRACE_DEBUG(("Maus_Application::setActiveMaskMsg()"));

	using namespace MausDllApi;
	int rcmc;
	int* info = ::new (m_buffer) int[7];
	int mask = info[2];		// Mask to be set

	// Prepare for output
	info[0] = 4;			// Length
	info[1] = info[1] + 1;	// Return message id == 6

	if (m_apMaus)
	{
		rcmc = m_apMaus->setActiveMask(mask);
		if (rcmc != ApMaus::SUCCESS)
		{
			TRACE_DEBUG(("Maus_Application::setActiveMaskMsg() failed"));
			info[2] = rcmc == ApMaus::BUSY?
					MauMsg::MauCoreBusy : MauMsg::GetQuorumInfoFailed;
		}
		else
		{
			TRACE_DEBUG(("Maus_Application::setActiveMaskMsg() successful"));
			info[2] = MauMsg::Success;
		}
	}
	else
	{
		TRACE_DEBUG(("Maus_Application::setActiveMaskMsg() maus not running, cannot set mask"));
		info[2] = MauMsg::MauCoreNotRunning;
	}
	int res = m_outQ.send(m_buffer, 32);
	TRACE_DEBUG(("Maus_Application::setActiveMaskMsg(), msg send() returns <%d>", res));
	TRACE_DEBUG(("Maus_Application::setActiveMaskMsg() returns"));
}

void Maus_Application::getQuorumInfoMsg()
{
	TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg()"));

	using namespace MausDllApi;
	MausCore::MausQuorum qd;
	int rcmc;
	int* info = ::new (m_buffer) int[7];

	info[0] = 7;			// Length
	info[1] = info[1] + 1;	// Return message id == 4
	if (m_apMaus)
	{
		rcmc = m_apMaus->getQuorumdata(qd);
		if (rcmc != ApMaus::SUCCESS)
		{
			info[2] = rcmc == ApMaus::BUSY?
					MauMsg::MauCoreBusy : MauMsg::GetQuorumInfoFailed;
		}
		else
		{
			TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg() get quorum data ok"));
			TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg() connView <%d>", qd.connView));
			TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg() quorumView <%d>", qd.quorumView));
			TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg() activeMau <%d>", qd.activeMau));
			TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg() activeMask <%d>", qd.activeMask));
			info[2] = MauMsg::Success;		// Oper state == OK
			info[3] = qd.connView;			// connectivity view
			info[4] = qd.quorumView;		// quorum view
			info[5] = qd.activeMau;			// active index
			info[6] = qd.activeMask;		// active mask
		}
	}
	else
	{
		info[2] = MauMsg::MauCoreNotRunning;
	}
	int res = m_outQ.send(m_buffer, 32);
	TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg(), msg send() returns <%d>", res));

	TRACE_DEBUG(("Maus_Application::getQuorumInfoMsg() returns"));

}

void Maus_Application::rejectMsg(int cause)
{
	TRACE_DEBUG(("Maus_Application::rejectMsg(), with cause <%d>", cause));

	size_t size = MAUMsgQ::getMaxMsgSize();
	m_inQ.receive(m_buffer_reject, &size);
	int* info = ::new (m_buffer_reject) int[4];
	info[0] = 3;							// Length
	info[1] = info[1] + 1;					// Return message id
	info[2] = cause;						// Response value
	m_outQ.send(m_buffer_reject, 16);

	TRACE_DEBUG(("Maus_Application::rejectMsg() with msg id <%d>, returns", info[1]));
}

void Maus_Application::performChecksumMsg()
{
	TRACE_DEBUG(("Maus_Application::performChecksumMsg()"));
	size_t size = MAUMsgQ::getMaxMsgSize();
	m_inQ.receive(m_buffer, &size);
	int* info = ::new (m_buffer) int[4];
	info[0] = 3;	// Length
	info[1] = 8;	// Return message id

	int checksumIndicator = info[2];
	int configSection = (checksumIndicator == 1) ? info[3] : 1;
	TRACE_DEBUG(("Maus_Application::performChecksumMsg() checksumIndicator <%d>, configSection <%d>", checksumIndicator, configSection));

	info[2] = MAUFunx::performChecksum(m_instance, m_sysInfo->m_slotId, configSection);

	int res = m_outQ.send(m_buffer, 16);
	TRACE_DEBUG(("Maus_Application::performChecksumMsg(), performChecksum() returns <%d>", info[2]));
	TRACE_DEBUG(("Maus_Application::performChecksumMsg() send() returns <%d>, returns", res));
}

void Maus_Application::setFCStateMsg()
{
	TRACE_DEBUG(("Maus_Application::setFCStateMsg()"));

	using namespace MausDllApi;
	int rcmc;
	int* info = ::new (m_buffer) int[7];
	int fcState = info[2];		// FC State to be set

	// Prepare for output
	info[0] = 4;			// Length
	info[1] = info[1] + 1;	// Return message id == 10

	if (m_apMaus)
	{
		rcmc = m_apMaus->setFCState(fcState);
		if (rcmc != ApMaus::SUCCESS)
		{
			TRACE_DEBUG(("Maus_Application::setFCStateMsg() failed"));
			info[2] = rcmc == ApMaus::BUSY?
					MauMsg::MauCoreBusy : MauMsg::SetFCStateFailed;
		}
		else
		{
			TRACE_DEBUG(("Maus_Application::setFCStateMsg() successful"));
			info[2] = MauMsg::Success;
		}
	}
	else
	{
		TRACE_DEBUG(("Maus_Application::setFCStateMsg() maus not running, cannot set fc state"));
		info[2] = MauMsg::MauCoreNotRunning;
	}
	int res = m_outQ.send(m_buffer, 32);
	TRACE_DEBUG(("Maus_Application::setFCStateMsg(), msg send() returns <%d>", res));
	TRACE_DEBUG(("Maus_Application::setFCStateMsg() returns"));
}

void Maus_Application::installMibs()
{
	TRACE_DEBUG(("Maus_Application::installMibs()"));
	size_t size = MAUMsgQ::getMaxMsgSize();
	m_inQ.receive(m_buffer, &size);
	int* info = ::new (m_buffer) int[4];
	info[0] = 3;	// Length
	info[1] = 8;	// Return message id

	int opInd = info[2];
	int configSection = (opInd == 1) ? info[3] : 1;
	TRACE_DEBUG(("Maus_Application::installMibs() opInd <%d>, configSection <%d>", opInd, configSection));

	info[2] = MAUFunx::installMibs(m_instance, m_sysInfo->m_slotId, configSection);

	int res = m_outQ.send(m_buffer, 16);
	TRACE_DEBUG(("Maus_Application::installMibs(), installMibs() returns <%d>", info[2]));
	TRACE_DEBUG(("Maus_Application::installMibs() send() returns <%d>, returns", res));
}

