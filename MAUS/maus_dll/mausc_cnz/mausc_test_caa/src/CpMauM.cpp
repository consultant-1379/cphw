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

//#undef PERFORM_CHECKSUM_ON_CPMAUM_LOADING
#define PERFORM_CHECKSUM_ON_CPMAUM_LOADING


using namespace std;

const char*	CpMauM::s_gmServerDomain = "MAUS";
const char* CpMauM::s_gmServerName[] = {"Unknown", "CP1MAUM", "CP2MAUM"};

CpMauM::CpMauM(int instance): m_instance(instance),
		m_activeConn(false),
		m_passiveConn(false),
		m_gmActive(s_gmServerName[m_instance], s_gmServerDomain, 0),
		m_gmPassive(s_gmServerName[m_instance], s_gmServerDomain, 1)
{
	m_activeConn = m_gmActive.connect();
	m_passiveConn = m_gmPassive.connect();
}

CpMauM::OperationalStateT CpMauM::getOperationalState()
{
	//TRACE(("CpMauM::getOperationalState()"));
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
	    if (rsp->state() != 0)
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

	//TRACE(("CpMauM::getOperationalState() returns opState<%d>", opState));
	return opState;
}

bool CpMauM::disable()
{
	//cout << "CpMauM::disable()" << endl;
	//TRACE(("CpMauM::disable()"));
	boost::mutex::scoped_lock lock(s_cs);
	bool result = false;
	do
	{
		if (!isConnected())
		{
			// TRACE()
			result = false;
			break;
		}

		MauMsg msg;
		MauMsg::DisableMsg* cmd = new (msg.addr()) MauMsg::DisableMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;

	    ssize_t res = m_gmActive.send(msg.addr(), 512);
		//cout << "CpMauM::disable(), send to active node, result <" << res << ">" << endl;
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }

	    MauMsg::DisableRspMsg* rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;

	    }
	    msg.reset();
	    cmd = new (msg.addr()) MauMsg::DisableMsg(1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
		//cout << "CpMauM::disable(), send to passive node, result <" << res << ">" << endl;
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }
	    rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;

	    }
	    result = true;
	}
	while (false);

	//TRACE(("CpMauM::disable() returns result <%d>", result));
	return result;
}

bool CpMauM::enable()
{
	//cout << "CpMauM::enable()" << endl;
	//TRACE(("CpMauM::enable()"));
	boost::mutex::scoped_lock lock(s_cs);
	bool result = false;
	do
	{
		if (!isConnected())
		{
			// TRACE()
			result = false;
			break;
		}

		ssize_t res = 0;
		MauMsg msg;

#ifdef PERFORM_CHECKSUM_ON_CPMAUM_LOADING
		new (msg.addr()) MauMsg::PerformChecksumMsg(1, 0);
	    res = m_gmActive.send(msg.addr(), 512);
	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }
	    MauMsg::PerformChecksumRspMsg* rspcs = reinterpret_cast<MauMsg::PerformChecksumRspMsg*>(msg.addr());
	    if (rspcs->error() != 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }
#endif
		msg.reset();
		MauMsg::EnableMsg* cmd = new (msg.addr()) MauMsg::EnableMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;

	    res = m_gmActive.send(msg.addr(), 512);
		//cout << "CpMauM::enable(), send to active node, result <" << res << ">" << endl;
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }

	    MauMsg::EnableRspMsg* rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }

#ifdef PERFORM_CHECKSUM_ON_CPMAUM_LOADING
	    msg.reset();
	    new (msg.addr()) MauMsg::PerformChecksumMsg(1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
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
#endif
	    msg.reset();
	    cmd = new (msg.addr()) MauMsg::EnableMsg(1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
		//cout << "CpMauM::enable(), send to passive node, result <" << res << ">" << endl;
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;
	    }
	    rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	// TRACE
	    	result = false;
	    	break;

	    }
	    result = true;
	}
	while (false);
	//TRACE(("CpMauM::enable() returns result <%d>", result));

	return result;
}

bool CpMauM::setFCState(int state)
{
	boost::mutex::scoped_lock lock(s_cs);
	bool result = false;
	do
	{
		if (!m_activeConn)
			m_activeConn = m_gmActive.connect();
		if (!m_passiveConn)
			m_passiveConn = m_gmPassive.connect();

		if (!isConnected())
		{
			result = false;
			break;
		}

		ssize_t res = 0;
		MauMsg msg;

		// Try to send message on Active node
		// If failed, then try Passive node
		new (msg.addr()) MauMsg::SetFCStateMsg(state, 1, 0);
	    res = m_gmActive.send(msg.addr(), 512);
	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }
	    MauMsg::SetFCStateRspMsg* rspfc = reinterpret_cast<MauMsg::SetFCStateRspMsg*>(msg.addr());
	    if (rspfc->error() != 0)
	    {
	    	result = false;
	    	break;
	    }

		new (msg.addr()) MauMsg::SetFCStateMsg(state, 1, 0);
	    res = m_gmPassive.send(msg.addr(), 512);
	    if (res <= 0)
	    {
	    	result = false;
	    	break;
	    }
	    rspfc = reinterpret_cast<MauMsg::SetFCStateRspMsg*>(msg.addr());
	    if (rspfc->error() != 0)
	    {
	    	result = false;
	    	break;
	    }
	    result = true;
	}
	while (false);
	return result;
}

