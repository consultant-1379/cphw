

#include <string>
#include <new>
#include "TerminalServer.h"
#include "AppTrace.h"
#include "AppEvent.h"
#include "ScoMsg.h"

using namespace std;

TerminalServer::TerminalServer(const char* name, const char* domain) :
		DSDServer(name, domain),
		m_aborting(false),
		m_ipAddrTable(),
		m_termConn(),
		m_termSession()
{
	TRACE_DEBUG(("TerminalServer::TerminalServer() ctor"));
	TRACE_DEBUG(("TerminalServer::TerminalServer() ctor"));
}

TerminalServer::~TerminalServer()
{
	TRACE_DEBUG(("TerminalServer::~TerminalServer() dtor"));

	TRACE_DEBUG(("TerminalServer::~TerminalServer() returns"));
}

ssize_t TerminalServer::incomingData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes)
{
	(void) handle;
	(void) session;
	(void) buffer;
	(void) noOfBytes;

	TRACE_DEBUG(("TerminalServer::incomingData()"));
	ScoMsg::MsgHeader* header = reinterpret_cast<ScoMsg::MsgHeader*>(buffer);
	TRACE_DEBUG(("TerminalServer::incomingData(), Primitive is <%d>", header->primitive()));

	ssize_t result = 0;
	switch(header->primitive())
	{
	case ScoMsg::Sco_Connect_prim:
		result = processScoConnectMsg(handle, session, buffer, noOfBytes);
		break;

	default:;
		break;
	}
	return result;
}


void TerminalServer::start()
{
	TRACE_DEBUG(("TerminalServer::start()"));
	DSDServer::start();
	TRACE_DEBUG(("TerminalServer::start() returns"));
}

void TerminalServer::stop()
{
	TRACE_DEBUG(("TerminalServer::stop()"));

	// Input from client side is rejected from this point onward
	m_aborting(true);

	DSDServer::stop();

	TRACE_DEBUG(("TerminalServer::stop() returns"));
}

ssize_t TerminalServer::processScoConnectMsg(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes)
{
	TRACE_DEBUG(("TerminalServer::processScoConnectMsg() with handle <%d> noOfBytes <%d>", handle, noOfBytes));

	// Server is shutting down, stop all messages
	if (m_aborting)
	{
		return 1;
	}

	ScoMsg msg;
	memcpy(msg.addr(), buffer, 2048);

	ScoMsg::Sco_ConnectMsg* connectMsg = reinterpret_cast<ScoMsg::Sco_ConnectMsg*>(msg.addr());
    ostringstream ss;
    ss << *connectMsg;
    TRACE(("%s", ss.str().c_str()));

	int32_t seqNo = connectMsg->seqNumber();
	int32_t cpId = connectMsg->getCpId();
	int32_t side = connectMsg->getCpSide();
	int32_t multiCp = connectMsg->getMultiCpSystem();

	// Adjusting side information
	if (cpId < 64 && cpId >=0)
	{
		side = 0;
	}

	if (!multiCp)
	{
		cpId = 1001;
	}

	TRACE_DEBUG(("TerminalServer::processScoConnectMsg(), seqNo <%d>", seqNo));
	TRACE_DEBUG(("TerminalServer::processScoConnectMsg(), cpId <%d>", cpId));
	TRACE_DEBUG(("TerminalServer::processScoConnectMsg(), side <%d>", side));
	TRACE_DEBUG(("TerminalServer::processScoConnectMsg(), multiCp <%d>", multiCp));

	msg.reset();
	string ipAddr1, ipAddr2;
	int32_t	error = 0;
	do {

		// check if SOL IP address is defined for this CP
		if (!m_ipAddrTable.find(cpId, side, ipAddr1, ipAddr2))
		{
			//res = ::new (msg.addr()) ScoMsg::Sco_ConnectRspMsg(ScoMsg::SolIPAddressNotDefined, seqno, 1, 0);
			error = ScoMsg::SolIPAddressNotDefined;
			break;
		}

		bool temp = m_termConn.insert(cpId, side, handle);
		TRACE(("insert new cp to connection result is <%d>", temp));

		if (!temp)
		{
			error = ScoMsg::ConnectionExsits;
			break;
		}
	}
	while (false);

	TRACE(("ipAddr1 len <%d> <%s>", ipAddr1.length(), ipAddr1.c_str()));
	TRACE(("ipAddr2 len <%d> <%s>", ipAddr2.length(), ipAddr2.c_str()));

	ScoMsg::Sco_ConnectRspMsg* rspMsg = ::new (msg.addr()) ScoMsg::Sco_ConnectRspMsg(error, seqNo,
			ipAddr1.length(), ipAddr1.c_str(), ipAddr2.length(), ipAddr2.c_str(),1, 0);
    ss.str("");
    ss.clear();
    ss << *rspMsg;
    TRACE(("%s", ss.str().c_str()));
    size_t res = session->send(msg.addr(), 2048);

	return res;
}

void TerminalServer::closeConnection(acs_dsd::HANDLE handle)
{
	TRACE_DEBUG(("TerminalServer::closeConnection() for handle <%d>", handle));
	m_termConn.remove(handle);
	TRACE_DEBUG(("TerminalServer::closeConnection() at return, size of m_termConn <%d>", m_termConn.size()));
}
