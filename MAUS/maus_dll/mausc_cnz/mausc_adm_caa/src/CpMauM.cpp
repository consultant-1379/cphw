/*
 * CpMauM.cpp
 *
 *  Created on: Feb 23, 2015
 *      Author: xdtthng
 */

#include "CpMauM.h"
#include "MauMsg.h"
#include "maus_trace.h"
#include <iostream>

using namespace std;

const char*	CpMauM::s_gmServerDomain = "MAUS";
const char* CpMauM::s_gmServerName[] = {"Unknown", "CP1MAUM", "CP2MAUM"};

CpMauM::CpMauM(int instance): m_instance(instance),
		m_activeConn(false),
		m_passiveConn(false),
		m_gmActive(s_gmServerName[m_instance], s_gmServerDomain, 0),
		m_gmPassive(s_gmServerName[m_instance], s_gmServerDomain, 1)
{
	TRACE(("CpMauM::CpMauM() ctor"));
	m_activeConn = m_gmActive.connect();
	m_passiveConn = m_gmPassive.connect();
	TRACE(("CpMauM::CpMauM() m_activeConn bool <%d>, m_passiveConn bool <%d>", m_activeConn, m_passiveConn));
}

CpMauM::OperationalStateT CpMauM::getOperationalState()
{
	TRACE(("CpMauM::getOperationalState()"));
	boost::mutex::scoped_lock lock(s_cs);
	OperationalStateT opState = NotAvailable;
	do
	{
		if (!isConnected())
		{
			// TRACE()
			opState = NotAvailable;
			break;
		}

		//cout << "CpMauM::getOperationalState(), all connected" << endl;

		MauMsg msg;
		MauMsg::GetOperationStateMsg* cmd = new (msg.addr()) MauMsg::GetOperationStateMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;
	    ssize_t res = m_gmActive.send(msg.addr(), 512);
		//cout << "CpMauM::getOperationalState(), send to active node, result <" << res << ">" << endl;

	    if (res <= 0)
	    {
	    	// TRACE
	    	opState = NotAvailable;
	    	break;
	    }

	    MauMsg::GetOperationStateRspMsg* rsp = reinterpret_cast<MauMsg::GetOperationStateRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->state() != 1)
	    {
	    	// TRACE
	    	opState = NotAvailable;
	    	break;

	    }

	    msg.reset();
	    new (msg.addr()) MauMsg::GetOperationStateMsg(1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
	    if (res <= 0)
	    {
	    	// TRACE
	    	opState = NotAvailable;
	    	break;
	    }
	    rsp = reinterpret_cast<MauMsg::GetOperationStateRspMsg*>(msg.addr());
	    //cout << "final response is" << endl;
	    //cout << *rsp << endl;
	    opState = OperationalStateT(rsp->state());
	}
	while (false);

	TRACE(("CpMauM::getOperationalState() returns opState<%d>", opState));
	return opState;
}

bool CpMauM::disable()
{
	//cout << "CpMauM::disable()" << endl;
	TRACE(("CpMauM::disable()"));
	boost::mutex::scoped_lock lock(s_cs);
	bool result = false;
	do
	{
		if (!isAnyConnected())
		{
			TRACE(("CpMauM::enable() both DSD Server on Active and Passive nodes are down"));
			result = false;
			break;
		}

		MauMsg msg;
		MauMsg::DisableMsg* cmd = 0;
		MauMsg::DisableRspMsg* rsp = 0;
		ssize_t res = 0;

		do
		{
		    if (!m_activeConn)
		    {
		    	break;
		    }
			cmd = new (msg.addr()) MauMsg::DisableMsg(1, 0);
			//cout << *cmd << endl;
			(void) cmd;

			res = m_gmActive.send(msg.addr(), 512);
			//cout << "CpMauM::disable(), send to active node, result <" << res << ">" << endl;
	    	TRACE(("CpMauM::disable() send disable msg to Active node with res <%d>", res));
			if (res <= 0)
			{
				break;
			}

			rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
			//cout << "response message" << endl;
			//cout << *rsp << endl;
			TRACE(("CpMauM::disable() disable rsp msg error codepoint <%d>", rsp->error()));
		}
		while (false);

	    if (!m_passiveConn)
	    {
	    	result = false;
	    	break;
	    }

	    msg.reset();
	    cmd = new (msg.addr()) MauMsg::DisableMsg(1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
		//cout << "CpMauM::disable(), send to passive node, result <" << res << ">" << endl;
    	TRACE(("CpMauM::disable() send disable msg to Passive node with res <%d>", res));
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }
	    rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
		TRACE(("CpMauM::disable() disable rsp msg error codepoint <%d>", rsp->error()));
	    result = rsp->error() == 0;
	}
	while (false);

	TRACE(("CpMauM::disable() returns result <%d>", result));
	return result;
}

bool CpMauM::enable()
{
	//cout << "CpMauM::enable()" << endl;
	TRACE(("CpMauM::enable()"));
	boost::mutex::scoped_lock lock(s_cs);
	bool result = false;
	do
	{
		if (!isAnyConnected())
		{
			TRACE(("CpMauM::enable() both DSD Server on Active and Passive nodes are down"));
			result = false;
			break;
		}

		ssize_t res = 0;
		MauMsg::PerformChecksumRspMsg* rspcs = 0;
		MauMsg::EnableMsg* cmd = 0;
		MauMsg::EnableRspMsg* rsp = 0;
		MauMsg msg;

		// This is a Bug 508
		// Services are brought up only when MAUS on both AP nodes are operational
		// It should be broght up when MAUS is operational on any AP node.
#if 0
		new (msg.addr()) MauMsg::PerformChecksumMsg(1, 0);
	    res = m_gmActive.send(msg.addr(), 512);
	    TRACE(("CpMauM::enable() send checksum msg to Active node with res <%d>", res));

	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }

	    rspcs = reinterpret_cast<MauMsg::PerformChecksumRspMsg*>(msg.addr());
	    if (rspcs->error() != 0)
	    {
	    	result = false;
	    	break;
	    }

		msg.reset();
		cmd = new (msg.addr()) MauMsg::EnableMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;

	    res = m_gmActive.send(msg.addr(), 512);
	    TRACE(("CpMauM::enable() send Enable msg to Active node with res <%d>", res));
		//cout << "CpMauM::enable(), send to active node, result <" << res << ">" << endl;
	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }

	    MauMsg::EnableRspMsg* rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	result = false;
	    	break;
	    }

#endif


	    // Service should be brought up on any node which MAUS is operational
	    // Attempt to bring up the service if MAUS is operational on Active node
	    do
	    {
		    if (!m_activeConn)
		    {
		    	break;
		    }
	    	new (msg.addr()) MauMsg::PerformChecksumMsg(1, 1, 1, 0);
	    	res = m_gmActive.send(msg.addr(), 512);
	    	TRACE(("CpMauM::enable() send checksum msg to Active node with res <%d>", res));
		    if (res <= 0)
		    {
		    	break;
		    }
    		rspcs = reinterpret_cast<MauMsg::PerformChecksumRspMsg*>(msg.addr());
    		TRACE(("CpMauM::enable() checksum res msg error codepoint <%d>", rspcs->error()));
    	    if (rspcs->error() != 0)
    	    {
    	    	break;
    	    }
			msg.reset();
			cmd = new (msg.addr()) MauMsg::EnableMsg(1, 0);
			//cout << *cmd << endl;
			res = m_gmActive.send(msg.addr(), 512);
			TRACE(("CpMauM::enable() send Enable msg to Active node with res <%d>", res));
		    if (res <= 0)
		    {
		    	break;
		    }
			MauMsg::EnableRspMsg* rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
			TRACE(("CpMauM::enable() Enable rsp msg error codepoint <%d>", rsp->error()));
	    }
	    while (false);

	    if (!m_passiveConn)
	    {
	    	result = false;
	    	break;
	    }

	    msg.reset();
	    new (msg.addr()) MauMsg::PerformChecksumMsg(1, 1, 1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
	    TRACE(("CpMauM::enable() send checksum msg to Passive node with res <%d>", res));
	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }
	    rspcs = reinterpret_cast<MauMsg::PerformChecksumRspMsg*>(msg.addr());
	    TRACE(("CpMauM::enable() checksum res msg error codepoint <%d>", rspcs->error()));
	    if (rspcs->error() != 0)
	    {
	    	result = false;
	    	break;
	    }

	    msg.reset();
	    cmd = new (msg.addr()) MauMsg::EnableMsg(1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
		//cout << "CpMauM::disable(), send to passive node, result <" << res << ">" << endl;
	    TRACE(("CpMauM::enable() send Enable msg to Passive node with res <%d>", res));
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }
	    rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    TRACE(("CpMauM::enable() Enable rsp msg error codepoint <%d>", rsp->error()));
	    result = rsp->error() == 0;
	}
	while (false);
	TRACE(("CpMauM::enable() returns result <%d>", result));

	return result;
}


#if 0

bool CpMauM::setFCState(int state)
{
	boost::mutex::scoped_lock lock(s_cs);
	bool result = false;
	do
	{
		if (!isAnyConnected())
		{
			TRACE(("CpMauM::setFCState() both DSD Server on Active and Passive nodes are down"));
			result = false;
			break;
		}

		ssize_t res = 0;
		MauMsg msg;
		MauMsg::SetFCStateRspMsg* rspfc = 0;

		// Try to send message on Active node
		// If failed, then try Passive node
		do
		{
		    if (!m_activeConn)
		    {
		    	break;
		    }
			new (msg.addr()) MauMsg::SetFCStateMsg(state, 1, 0);
			res = m_gmActive.send(msg.addr(), 512);
	    	TRACE(("CpMauM::setFCState() send fexReq msg to Active node with res <%d>", res));
			if (res <= 0)
			{
				break;
			}
			rspfc = reinterpret_cast<MauMsg::SetFCStateRspMsg*>(msg.addr());
			TRACE(("CpMauM::setFCState() fexRsp msg error codepoint <%d>", rspfc->error()));
		}
		while (false);

	    if (!m_passiveConn)
	    {
	    	result = false;
	    	break;
	    }
		new (msg.addr()) MauMsg::SetFCStateMsg(state, 1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
    	TRACE(("CpMauM::setFCState() send fexReq msg to Passive node with res <%d>", res));
	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }
	    rspfc = reinterpret_cast<MauMsg::SetFCStateRspMsg*>(msg.addr());
		TRACE(("CpMauM::setFCState() fexRsp msg error codepoint <%d>", rspfc->error()));
	    result = rspfc->error() == 0;
	}
	while (false);
	return result;
}

#endif
