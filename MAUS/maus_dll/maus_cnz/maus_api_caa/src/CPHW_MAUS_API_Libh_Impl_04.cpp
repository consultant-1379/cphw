/*
 * CPHW_MAUS_Libh_Impl.cpp
 *
 *  Created on: May 5, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include "CPHW_MAUS_API_Libh_Impl.h"
#include "MauMsg.h"

using namespace std;


const char*	CPHW_MAUS_API_Libh_Impl::s_gmServerDomain = "MAUS";
const char* CPHW_MAUS_API_Libh_Impl::s_gmServerName[] = {"Unknown", "CP1MAUM", "CP2MAUM"};

boost::mutex CPHW_MAUS_API_Libh_Impl::s_cs;

CPHW_MAUS_API_Libh_Impl::CPHW_MAUS_API_Libh_Impl(int instance):
	m_instance(instance),
	m_activeConn(false),
	m_passiveConn(false),
	m_gmActive(s_gmServerName[m_instance], s_gmServerDomain, 0),
	m_gmPassive(s_gmServerName[m_instance], s_gmServerDomain, 1)
{

	m_activeConn = m_gmActive.connect();
	m_passiveConn = m_gmPassive.connect();
	m_gmClient[0] = m_gmClient[1] = 0;
	m_conn[0] = m_conn[1] = false;

	// Map node state to node name
	int side = m_gmActive.getSide();
	int oside = side == 1? 0:1;

	cout << "Side of Active node is <" << side << ">" << endl;
	if (side != -1) {
		m_gmClient[side] = &m_gmActive;
		m_gmClient[oside] = &m_gmPassive;
		m_conn[side] = m_activeConn;
		m_conn[oside] = m_passiveConn;
	}

	cout << "Side of Passive node is <" << oside << ">" << endl;
}

CPHW_MAUS_API_Libh_Impl::~CPHW_MAUS_API_Libh_Impl()
{

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::loadLib(int side)
{
	cout << "CPHW_MAUS_API_Libh_Impl::loadLib(" << side << ")" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

	if (side == BothSides)
	{
		do {
			if (!m_conn[SideA] || !m_conn[SideB])
			{
				cout << "loadLib, Either SideA or SideB has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}

			result = load(SideA);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				cout << "loadLib, Failed to load SideA" << endl;
				break;
			}
			result = load(SideB);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				cout << "loadLib, Failed to load SideB" << endl;
				break;
			}
		}
		while (false);
	}
	else
	{
		do {

			if (!m_conn[side])
			{
				cout << "loadLib, Side <" << side << "> has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}
			result = load(side);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				cout << "loadLib, Failed to load Side <" << side << ">" << endl;
				break;
			}
		}
		while (false);
	}

	cout << "CPHW_MAUS_API_Libh_Impl::loadLib() returns <" << result << ">" << endl;

	return result;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::load(int side)
{
	cout << "CPHW_MAUS_API_Libh_Impl::load(" << side << ")" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

#if 0
	int opState;
	result = getOperationalState(side, opState);

	cout << "load(), result of getOperationalState is <" << result << ">" << endl;
	cout << "load(), opState is <" << opState << ">" << endl;

	if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
		return result;

	if (opState == Available)
		return CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED;
#endif

	// Start unloading the lib
	do
	{
		MauMsg msg;
		MauMsg::EnableMsg* cmd = new (msg.addr()) MauMsg::EnableMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;

	    size_t res = m_gmClient[side]->send(msg.addr(), 1024);
		//cout << "CpMauM::disable(), send to active node, result <" << res << ">" << endl;
	    if (res == 0)
	    {
	    	// TRACE
	    	cout << "load(), server does not response" << endl;
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB;
	    	break;
	    }

	    MauMsg::EnableRspMsg* rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	// TRACE
	    	cout << "load(), server response, but cannot load" << endl;
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB;
	    	break;

	    }
	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);


	cout << "CPHW_MAUS_API_Libh_Impl::load() returns result <" << result << ">" << endl;
	return result;
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::unloadLib(int side)
{
	cout << "CPHW_MAUS_API_Libh_Impl::unloadLib()" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

	if (side == BothSides)
	{
		do {
			if (!m_conn[SideA] || !m_conn[SideB])
			{
				cout << "Either SideA or SideB has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}

			result = unload(SideA);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				cout << "Failed to unload SideA" << endl;
				break;
			}
			result = unload(SideB);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				cout << "Failed to unload SideB" << endl;
				break;
			}
		}
		while (false);
	}
	else
	{
		do {

			if (!m_conn[side])
			{
				cout << "Side <" << side << "> has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}
			result = unload(side);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				cout << "Failed to unload Side <" << side << ">" << endl;
				break;
			}
		}
		while (false);
	}

	cout << "CPHW_MAUS_API_Libh_Impl::unloadLib() returns <" << result << ">" << endl;

	return result;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::unload(int side)
{
	cout << "CPHW_MAUS_API_Libh_Impl::unload()" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

#if 0
	int opState;
	result = getOperationalState(side, opState);

	cout << "result of getOperationalState is <" << result << ">" << endl;
	cout << "opState is <" << opState << ">" << endl;

	if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
		return result;

	if (opState == NotAvailable)
		return CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED;
#endif

	// Start unloading the lib
	do
	{
		MauMsg msg;
		MauMsg::DisableMsg* cmd = new (msg.addr()) MauMsg::DisableMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;

	    size_t res = m_gmClient[side]->send(msg.addr(), 1024);
		//cout << "CpMauM::disable(), send to active node, result <" << res << ">" << endl;
	    if (res == 0)
	    {
	    	// TRACE
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB;
	    	break;
	    }

	    MauMsg::DisableRspMsg* rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->error() != 0)
	    {
	    	// TRACE
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB;
	    	break;

	    }
	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);


	cout << "CPHW_MAUS_API_Libh_Impl::unload() returns result <" << result << ">" << endl;
	return result;
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::setMask(int mask)
{
	(void) mask;

	return CPHW_MAUS_API_Libh::RC_SUCCESS;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::getOperationalState(int side, int& opState)
{
	cout << "CPHW_MAUS_API_Libh_Impl::getOperationalState(" << side << ")" << endl;

	boost::mutex::scoped_lock lock(s_cs);
	opState = NotAvailable;
	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_FAILED_TO_GET_OPER_STATE;
	do
	{
		if (!isConnected(side))
		{
			// TRACE()
			cout << "side <" << side << "> is not connected" << endl;
			cout << "m_conn[0] is " << m_conn[0] << endl;
			cout << "m_conn[1] is " << m_conn[1] << endl;
			opState = NotAvailable;
			result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
			break;
		}
		else
		{
			cout << "side <" << side << "> is connected ok" << endl;
		}
		MauMsg msg;
		MauMsg::GetOperationStateMsg* cmd = new (msg.addr()) MauMsg::GetOperationStateMsg(1, 0);
	    //cout << *cmd << endl;
		(void) cmd;
	    size_t res = m_gmClient[side]->send(msg.addr(), 1024);
		//cout << "CpMauM::getOperationalState(), send to active node, result <" << res << ">" << endl;

	    if (res == 0)
	    {
	    	// TRACE
			cout << "Server for side <" << side << "> does not response" << endl;
	    	opState = NotAvailable;
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_GET_OPER_STATE;
	    	break;
	    }

	    MauMsg::GetOperationStateRspMsg* rsp = reinterpret_cast<MauMsg::GetOperationStateRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    if (rsp->state() != 0)
	    {
	    	// TRACE
	    	cout << "Server response with opstate not available" << endl;
	    	opState = NotAvailable;
	    	result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	    	break;

	    }
	    opState = Available;
	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);
	cout << "CPHW_MAUS_API_Libh_Impl::getOperationalState() returns result <" << result << ">" << endl;
	cout << "CPHW_MAUS_API_Libh_Impl::getOperationalState() returns opState <" << opState << ">" << endl;
	return result;
}
