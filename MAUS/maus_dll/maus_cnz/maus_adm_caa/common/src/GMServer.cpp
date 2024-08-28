

#include <string>
#include <new>
#include "GMServer.h"
#include "maus_trace.h"
#include "MAUFunx.h"
#include "Maus_Application.h"
#include "MauMsg.h"
#include "maus_event.h"
//#include "AtomicFlag.h"

using namespace std;

extern ApMaus* g_ApMaus;
//extern BoolFlagT s_abortChecksum;


GMServer::GMServer(const char* name, const char* domain, int n, BoolFlagT& appRunning,
		Maus_Event& enable, Maus_Event& disable) :
		DSDServer(name, domain),
		m_instance(n),
		m_apMausRunning(appRunning),
		m_enableEvent(enable),
		m_disableEvent(disable),
		m_endEvent(),
		m_connection(m_instance, m_endEvent.get()),
		m_msgHandling(false)
{
	TRACE_DEBUG(("GMServer::GMServer() ctor"));
	int res = m_connection.connect();
	TRACE_DEBUG(("GMServer::GMServer() m_connection result <%d>", res));
	TRACE_DEBUG(("GMServer::GMServer() ctor"));
}

GMServer::~GMServer()
{
	TRACE_DEBUG(("GMServer::~GMServer() dtor"));
}

ssize_t GMServer::incomingData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes)
{
	TRACE_DEBUG(("GMServer::incomingData()"));

	MauMsg::MsgHeader* header = reinterpret_cast<MauMsg::MsgHeader*>(buffer);
	TRACE_DEBUG(("GMServer::incomingData(), Primitive is <%d>", header->primitive()));

	ssize_t result = 0;
	switch(header->primitive())
	{
	case MauMsg::GetOperationState:
		result = processGetOperState(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::Enable:
		result = processEnable(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::Disable:
		result = processDisable(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::GetQuorumInformation:
		result = processGetQuorumInfo(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::SetActiveMask:
		result = setActiveMask(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::PerformChecksum:
		result = performChecksum(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::SetFCState:
		result = setFCState(handle, session, buffer, noOfBytes);
		break;
	case MauMsg::InstallMibs:
		result = installMibs(handle, session, buffer, noOfBytes);
		break;
	default:
		break;
	}

	return result;
}


void GMServer::start()
{
	TRACE_DEBUG(("GMServer::start()"));
	DSDServer::start();
	TRACE_DEBUG(("GMServer::start() returns"));
}

void GMServer::stop()
{
	TRACE_DEBUG(("GMServer::stop()"));
	while (m_msgHandling)
	{
		// Wait for finishing up with the message
		// Not used at the moment
	}
	m_endEvent.set();
	DSDServer::stop();
	TRACE_DEBUG(("GMServer::stop() returns"));
}

ssize_t GMServer::processGetOperState(acs_dsd::HANDLE handle, SessionPtr session, char*, ssize_t)
{
	TRACE_DEBUG(("GMServer::processGetOperState() handle <%d>", handle));
	bool running = m_apMausRunning;

	// OperationalState in SwMau
	// -------------------------
	// The source of operationalState is in MAUS1 or MAUS2
	// running == true  => operationalState == ENABLED  (1)
	// running == false => operationalState == DISABLED (0)
	MauMsg msg;
	const MauMsg::GetOperationStateRspMsg* rsp = new (msg.addr()) MauMsg::GetOperationStateRspMsg(0, running);

	ssize_t res = -999;
	if (mSessions.count(handle))
	{
		res = session->send(msg.addr(), BUFFER_SIZE);
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("rsp msg [%s]", ss.str().c_str()));
		TRACE_DEBUG(("Send result is <%d>", res));
	}

#if 0
	TRACE_DEBUG(("##########################################################"));
	TRACE_DEBUG(("##########################################################"));
	TRACE_DEBUG(("Testing of communication to Application"));
	int32_t opState;
	int resState = m_connection.getOperationalState(opState);
	TRACE_DEBUG(("Result of get oper state <%d> opState <%d>", resState, opState));
	TRACE_DEBUG(("##########################################################"));
	TRACE_DEBUG(("##########################################################"));
#endif

	return res;
}

ssize_t GMServer::processEnable(acs_dsd::HANDLE handle, SessionPtr session, char*, ssize_t)
{
	TRACE_DEBUG(("GMServer::processEnable() handle <%d>", handle));
	// Raise Enable signal
	m_enableEvent.set();
	MauMsg msg;

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		const MauMsg::EnableRspMsg* rsp = new (msg.addr()) MauMsg::EnableRspMsg(0);
		res = session->send(msg.addr(), BUFFER_SIZE);
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("rsp msg [%s]", ss.str().c_str()));
		TRACE_DEBUG(("Send result is <%d>", res));
	}
	TRACE_DEBUG(("GMServer::processEnable() handle <%d>, return <%d>", handle, res));
	return res;
}

ssize_t GMServer::processDisable(acs_dsd::HANDLE handle, SessionPtr session, char*, ssize_t)
{
	TRACE_DEBUG(("GMServer::processDisable() handle <%d>", handle));

	// Raise Disable signal
	m_disableEvent.set();
	MauMsg msg;

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		const MauMsg::DisableRspMsg* rsp = new (msg.addr()) MauMsg::DisableRspMsg(0);
		TRACE_DEBUG(("GMServer::processDisable() before calling session->send(buffer, BUFFER_SIZE)"));
		res = session->send(msg.addr(), BUFFER_SIZE);
		TRACE_DEBUG(("GMServer::processDisable() session->send(buffer, BUFFER_SIZE) returns"));
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("rsp msg [%s]", ss.str().c_str()));
		TRACE_DEBUG(("Send result is <%d>", res));
	}

	return res;
}


ssize_t GMServer::processGetQuorumInfo(acs_dsd::HANDLE handle, SessionPtr session, char*, ssize_t)
{
	TRACE_DEBUG(("GMServer::processGetQuorumInfo() with handle <%d>", handle));

	// ApMaus is not running
	MauMsg msg;
	MauMsg::GetQuorumInfomationRspMsg* rsp = new (msg.addr()) MauMsg::GetQuorumInfomationRspMsg(MauMsg::MauCoreNotRunning);
	if (m_apMausRunning)
	{
		// Get quorum information from ApMaus
#if 1
		// Successful reading quorum data
		int size = 4;
		int qinfo[4];
		int res = m_connection.getQuorumInfo(qinfo, size);
		if (res == MauMsg::Success)
		{
			rsp = ::new (msg.addr()) MauMsg::GetQuorumInfomationRspMsg(MauMsg::Success,
					qinfo[0], qinfo[1], qinfo[2], qinfo[3]);
		}
		else
		{
			rsp = ::new (msg.addr()) MauMsg::GetQuorumInfomationRspMsg(MauMsg::GetQuorumInfoFailed, -1, -1, 0, 0);
		}
#else
	// This is another quick way of getting information
		// This leads to using destructed objects very quickly
		// It is left here for reference
		if (g_ApMaus && g_ApMaus->isMauCoreRunning())
		{
			using namespace MausDllApi;
			MausCore::MausQuorum qd;

			//if (mAborting)
			//	return 0;

			int rcmc = g_ApMaus->getQuorumdata(qd);
			if (rcmc == ApMaus::SUCCESS)
			{
				rsp = ::new (buffer) MauMsg::GetQuorumInfomationRspMsg(0, qd.connView, qd.quorumView,
						qd.activeMau, qd.activeMask);
			}
			else
			{
				rsp = ::new (buffer) MauMsg::GetQuorumInfomationRspMsg(rcmc == ApMaus::BUSY?
						MauMsg::MauCoreBusy : MauMsg::GetQuorumInfoFailed);
			}
		}
		else
		{
			rsp = ::new (buffer) MauMsg::GetQuorumInfomationRspMsg(MauMsg::MauCoreNotRunning);
		}

#endif

	}

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("GetQuorumInfomationRspMsg [%s]", ss.str().c_str()));
		res = session->send(msg.addr(), BUFFER_SIZE);
		TRACE_DEBUG(("Send result is <%d>", res));
	}
	TRACE_DEBUG(("GMServer::processGetQuorumInfo() returns"));
	return res;
}

ssize_t GMServer::setActiveMask(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t)
{
	TRACE_DEBUG(("GMServer::setActiveMask() with handle <%d>", handle));

	MauMsg msg;
	memcpy(msg.addr(), buffer, BUFFER_SIZE);
	MauMsg::SetActiveMaskMsg* setMaskMsg = reinterpret_cast<MauMsg::SetActiveMaskMsg*>(msg.addr());
	int32_t mask = setMaskMsg->mask();
	TRACE_DEBUG(("GMServer::setActiveMask(), mask to be set <%d>", mask));

	msg.reset();
	MauMsg::SetActiveMasRspMsg* rsp = new (msg.addr()) MauMsg::SetActiveMasRspMsg(MauMsg::MauCoreNotRunning);
	if (m_apMausRunning)
	{

#if 1

		int res = m_connection.setActiveMask(mask);
		if (res == 0)
		{
			rsp = ::new (msg.addr()) MauMsg::SetActiveMasRspMsg(0);
		}
		else
		{
			rsp = ::new (msg.addr()) MauMsg::SetActiveMasRspMsg(MauMsg::SetActiveMaskFailed);
		}
#else
	if (g_ApMaus && g_ApMaus->isMauCoreRunning())
	{
		using namespace MausDllApi;
		int rcmc = g_ApMaus->setActiveMask(mask);
		if (rcmc == ApMaus::SUCCESS)
			rsp = ::new (buffer) MauMsg::SetActiveMasRspMsg(0);
		else
			rsp = ::new (buffer) MauMsg::SetActiveMasRspMsg(rcmc == ApMaus::BUSY?
					MauMsg::MauCoreBusy : MauMsg::SetActiveMaskFailed);
	}
	else
	{
		rsp = ::new (buffer) MauMsg::SetActiveMasRspMsg(MauMsg::MauCoreNotRunning);
	}
#endif
	}

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("SetActiveMasRspMsg [%s]", ss.str().c_str()));
		res = session->send(msg.addr(), BUFFER_SIZE);
		TRACE_DEBUG(("Send result is <%d>", res));
	}
	TRACE_DEBUG(("GMServer::setActiveMask() returns"));
	return res;
}

ssize_t GMServer::performChecksum(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t)
{
	TRACE_DEBUG(("GMServer::performChecksum() with handle <%d>", handle));

	MauMsg msg;
	memcpy(msg.addr(), buffer, BUFFER_SIZE);
	MauMsg::PerformChecksumMsg* checksumMsg = reinterpret_cast<MauMsg::PerformChecksumMsg*>(msg.addr());
	int32_t csInd = checksumMsg->checksumIndicator();
	int32_t confSection = checksumMsg->confSection();
	TRACE_DEBUG(("GMServer::performChecksum(), csInd <%d> confSection <%d>", csInd, confSection));

	MauMsg::PerformChecksumRspMsg* rsp = 0;
	int rescs = m_connection.performChecksum(csInd, confSection);
	TRACE_DEBUG(("GMServer::performChecksum() returns res <%d>", rescs));
	msg.reset();
	if (rescs == 0)
	{
		rsp = ::new (msg.addr()) MauMsg::PerformChecksumRspMsg(0);
	}
	else
	{
		rsp = ::new (msg.addr()) MauMsg::PerformChecksumRspMsg(MauMsg::ChecksumFailed);
	}

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("PerformChecksumRspMsg [%s]", ss.str().c_str()));
		res = session->send(msg.addr(), BUFFER_SIZE);
		TRACE_DEBUG(("Send result is <%d>", res));
	}

	TRACE_DEBUG(("GMServer::PerformChecksumRspMsg() returns"));

	return res;
}

ssize_t GMServer::setFCState(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t)
{
	TRACE_DEBUG(("GMServer::setFCState() with handle <%d>", handle));

	MauMsg msg;
	memcpy(msg.addr(), buffer, 1024);
	MauMsg::SetFCStateMsg* setFCMsg = reinterpret_cast<MauMsg::SetFCStateMsg*>(msg.addr());
	int32_t fcState = setFCMsg->fcState();
	TRACE_DEBUG(("GMServer::setFCState(), fcState to be set <%d>", fcState));

	MauMsg::SetFCStateRspMsg* rsp = new (buffer) MauMsg::SetFCStateRspMsg(MauMsg::MauCoreNotRunning);
	if (m_apMausRunning)
	{
		int res = m_connection.setFCState(fcState);
		if (res == 0)
		{
			rsp = ::new (buffer) MauMsg::SetFCStateRspMsg(0);
		}
		else
		{
			rsp = ::new (buffer) MauMsg::SetFCStateRspMsg(MauMsg::SetFCStateFailed);
		}
	}

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("SetFCStateRspMsg [%s]", ss.str().c_str()));
		res = session->send(buffer, BUFFER_SIZE);
		TRACE_DEBUG(("Send result is <%d>", res));
	}
	TRACE_DEBUG(("GMServer::setFCState() returns"));

	return res;
}

ssize_t GMServer::installMibs(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t)
{
	TRACE_DEBUG(("GMServer::installMibs() with handle <%d>", handle));

	MauMsg msg;
	memcpy(msg.addr(), buffer, BUFFER_SIZE);
	MauMsg::InstallMibsMsg* mibsMsg = reinterpret_cast<MauMsg::InstallMibsMsg*>(msg.addr());
	int32_t opInd = mibsMsg->operationIndicator();
	int32_t confSection = mibsMsg->confSection();
	TRACE_DEBUG(("GMServer::installMibs(), opInd <%d> confSection <%d>", opInd, confSection));

	MauMsg::InstallMibsRspMsg* rsp = 0;

	int rescs = m_connection.installMibs(opInd, confSection);
	TRACE_DEBUG(("GMServer::installMibs() returns res <%d>", rescs));
	msg.reset();
	if (rescs == 0)
	{
		rsp = ::new (msg.addr()) MauMsg::InstallMibsRspMsg(0);
	}
	else
	{
		rsp = ::new (msg.addr()) MauMsg::InstallMibsRspMsg(MauMsg::InstallMibsFailed);
	}

	ssize_t res = -999;
	SessionMapIter it = mSessions.find(handle);
	if (it != mSessions.end() && it->second == session)
	{
		ostringstream ss;
		ss << *rsp;
		TRACE_DEBUG(("InstallMibsRspMsg [%s]", ss.str().c_str()));
		res = session->send(msg.addr(), BUFFER_SIZE);
		TRACE_DEBUG(("Send result is <%d>", res));
	}

	TRACE_DEBUG(("GMServer::installMibs() returns"));

	return res;
}
