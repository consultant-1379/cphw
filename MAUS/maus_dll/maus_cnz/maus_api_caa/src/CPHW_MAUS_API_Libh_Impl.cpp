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
	// This code must always run on the Active side
	int side = m_gmActive.getSide();
	int oside = side == 1? 0:1;

	//cout << "Side of Active node is <" << side << ">" << endl;
	if (side != -1) {
		m_gmClient[side] = &m_gmActive;
		m_gmClient[oside] = &m_gmPassive;
		m_conn[side] = m_activeConn;
		m_conn[oside] = m_passiveConn;
	}
	//cout << "Side of Passive node is <" << oside << ">" << endl;
}

CPHW_MAUS_API_Libh_Impl::~CPHW_MAUS_API_Libh_Impl()
{

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::loadLib(int side)
{
	//cout << "CPHW_MAUS_API_Libh_Impl::loadLib(" << side << ")" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

	if (side < SideA || side > BothSides)
	{
		result = CPHW_MAUS_API_Libh::RC_OPERATION_NOT_IMPLEMENTED;
	}
	else if (side == BothSides)
	{
		do {
			if (!m_conn[SideA] || !m_conn[SideB])
			{
				//cout << "loadLib, Either SideA or SideB has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}

			int operStateA;
			result = getOperationalState(SideA, operStateA);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}

#if 0
			if (operState == Available)
			{
				//cout << "Already loaded" << endl;
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED_NODEA;
				break;
			}
#endif
			int operStateB;
			result = getOperationalState(SideB, operStateB);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}

			if (operStateA == Available && operStateB == Available)
			{
				//cout << "Already loaded" << endl;
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED;
				break;
			}

			if (operStateA == NotAvailable)
			{
				// Checksum calculation
#if 0
				result = performChecksum(SideA);
				if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
				{
					//cout << "Failed to get operation state for Side <" << side << ">" << endl;
					break;
				}
#endif

				result = load(SideA);
				if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
				{
					//cout << "loadLib, Failed to load SideA" << endl;
					result = CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB_NODEA;
					break;
				}
			}

			if (operStateB == NotAvailable)
			{
				int32_t msec = 250;
				//cout << "sleep for " << msec << " ms before loading" << endl;
				struct timespec req;
				req.tv_sec = 0;
				req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
				nanosleep(&req, NULL);

				// Checksum calculation
#if 0
				result = performChecksum(SideB);
				if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
				{
					//cout << "Failed to get operation state for Side <" << side << ">" << endl;
					break;
				}
#endif

				result = load(SideB);
				if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
				{
					//cout << "loadLib, Failed to load SideB" << endl;
					result = CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB_NODEB;
					break;
				}
			}
		}
		while (false);
	}
	else
	{
		do {

			if (!m_conn[side])
			{
				//cout << "loadLib, Side <" << side << "> has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}
			int operState;
			result = getOperationalState(side, operState);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}

			if (operState == Available)
			{
				//cout << "Already loaded" << endl;
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED;
				break;
			}

			// Checksum calculation
#if 0
			result = performChecksum(side);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}
#endif

			result = load(side);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "loadLib, Failed to load Side <" << side << ">" << endl;
				break;
			}
		}
		while (false);
	}

	//cout << "CPHW_MAUS_API_Libh_Impl::loadLib() returns <" << result << ">" << endl;
	return result;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::load(int side)
{
	//cout << "CPHW_MAUS_API_Libh_Impl::load(" << side << ")" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

	// Start unloading the lib
	do
	{
		MauMsg msg;
		new (msg.addr()) MauMsg::EnableMsg(1, 0);

		ssize_t res = m_gmClient[side]->send(msg.addr(), 512);
	    if (res <= 0)
	    {
	    	//cout << "load(), server does not response from side " << side << endl;
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB;
	    	break;
	    }
	    else
	    {
	    	//cout << "load(), send to " << side << " get response with restul " << res << endl;
	    }

	    MauMsg::EnableRspMsg* rsp = reinterpret_cast<MauMsg::EnableRspMsg*>(msg.addr());
	    if (rsp->error() != 0)
	    {
	    	//cout << "load(), server response, but cannot load" << endl;
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB;
	    	break;

	    }
	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);

	//cout << "CPHW_MAUS_API_Libh_Impl::load() returns result <" << result << ">" << endl;
	return result;
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::unloadLib(int side)
{
	//cout << "CPHW_MAUS_API_Libh_Impl::unloadLib()" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

	if (side < SideA || side > BothSides)
	{
		result = CPHW_MAUS_API_Libh::RC_OPERATION_NOT_IMPLEMENTED;

	}
	else if (side == BothSides)
	{
		do {
			if (!m_conn[SideA] || !m_conn[SideB])
			{
				//cout << "Either SideA or SideB has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}

			int operStateA;
			result = getOperationalState(SideA, operStateA);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}

#if 0
			if (operState == NotAvailable)
			{
				//cout << "Already unloaded" << endl;
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED_NODEA;
				break;
			}
#endif
			int operStateB;
			result = getOperationalState(SideB, operStateB);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}

#if 0
			if (operState == NotAvailable)
			{
				//cout << "Already unloaded" << endl;
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED_NODEB;
				break;
			}
#endif
			if (operStateA == NotAvailable && operStateB == NotAvailable)
			{
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED;
				break;
			}

			if (operStateA == Available)
			{
				result = unload(SideA);
				if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
				{
					//cout << "Failed to unload SideA" << endl;
					result = CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB_NODEA;
					break;
				}
			}

			if (operStateB == Available)
			{
				int32_t msec = 200;
				//cout << "sleep for " << msec << " ms before unloading" << endl;
				struct timespec req;
				req.tv_sec = 0;
				req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
				nanosleep(&req, NULL);

				result = unload(SideB);
				if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
				{
					//cout << "Failed to unload SideB" << endl;
					result = CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB_NODEB;
					break;
				}
			}
		}
		while (false);
	}
	else
	{
		do {

			if (!m_conn[side])
			{
				//cout << "Side <" << side << "> has no contact to MAUS" << endl;
				result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
				break;
			}
			int operState;
			result = getOperationalState(side, operState);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to get operation state for Side <" << side << ">" << endl;
				break;
			}

			if (operState == NotAvailable)
			{
				//cout << "Already unloaded" << endl;
				result = CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED;
				break;
			}

			result = unload(side);
			if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
			{
				//cout << "Failed to unload Side <" << side << ">" << endl;
				break;
			}
		}
		while (false);
	}

	//cout << "CPHW_MAUS_API_Libh_Impl::unloadLib() returns <" << result << ">" << endl;
	return result;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::unload(int side)
{
	//cout << "CPHW_MAUS_API_Libh_Impl::unload()" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;

	// Start unloading the lib
	do
	{
		MauMsg msg;
		new (msg.addr()) MauMsg::DisableMsg(1, 0);

	    ssize_t res = m_gmClient[side]->send(msg.addr(), 512);
	    if (res <= 0)
	    {
	    	// TRACE
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB;
	    	break;
	    }

	    MauMsg::DisableRspMsg* rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
	    if (rsp->error() != 0)
	    {
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB;
	    	break;

	    }
	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);

	//cout << "CPHW_MAUS_API_Libh_Impl::unload() returns result <" << result << ">" << endl;
	return result;
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::setMask(int mask)
{
	(void) mask;

	return CPHW_MAUS_API_Libh::RC_OPERATION_NOT_IMPLEMENTED;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::getOperationalState(int side, int& opState)
{
	//cout << "CPHW_MAUS_API_Libh_Impl::getOperationalState(" << side << ")" << endl;

	boost::mutex::scoped_lock lock(s_cs);
	opState = NotAvailable;
	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_FAILED_TO_GET_OPER_STATE;
	do
	{
		if (side < SideA || side > SideB)
		{
			opState = NotAvailable;
			result = CPHW_MAUS_API_Libh::RC_OPERATION_NOT_IMPLEMENTED;
			break;
		}

		if (!isConnected(side))
		{
			opState = NotAvailable;
			result = CPHW_MAUS_API_Libh::RC_NOCONTACT;
			break;
		}
		//cout << "side <" << side << "> is connected ok" << endl;
		MauMsg msg;
		new (msg.addr()) MauMsg::GetOperationStateMsg(1, 0);
	    ssize_t res = m_gmClient[side]->send(msg.addr(), 512);

	    if (res <= 0)
	    {
			//cout << "Server for side <" << side << "> does not response" << endl;
	    	opState = NotAvailable;
	    	result = CPHW_MAUS_API_Libh::RC_FAILED_TO_GET_OPER_STATE;
	    	break;
	    }

	    MauMsg::GetOperationStateRspMsg* rsp = reinterpret_cast<MauMsg::GetOperationStateRspMsg*>(msg.addr());
	    if (rsp->state() == 0)
	    {
	    	//cout << "Server response with opstate not available" << endl;
	    	opState = NotAvailable;
	    	result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	    	break;

	    }
	    opState = Available;
	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);
	return result;
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::performChecksum(int side)
{
	//cout << "CPHW_MAUS_API_Libh_Impl::performChecksum(" << side << ")" << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;
	do
	{
		MauMsg msg;
		new (msg.addr()) MauMsg::PerformChecksumMsg(1, 1, 1, 0);
	    ssize_t rescs = m_gmClient[side]->send(msg.addr(), 512);
		//cout << "perform checksum rescs " << rescs << endl;
	    if (rescs <= 0)
	    {
	    	//result = side == SideA ? CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEA :
	    	//		CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEB;

	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEA :
	    			CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEB;
	    	break;
	    }
	    MauMsg::PerformChecksumRspMsg* rspcs = reinterpret_cast<MauMsg::PerformChecksumRspMsg*>(msg.addr());
	    if (rspcs->error() != 0)
	    {
	    	//cout << "load(), server response, check sum error" << endl;
	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEA :
	    			CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEB;
	    	break;
	    }

	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);

	//cout << "CPHW_MAUS_API_Libh_Impl::load() returns result <" << result << ">" << endl;
	return result;
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::performChecksum(int side, int config)
{
	//cout << "CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::performChecksum(int side, int config)" << endl;
	//cout << "side " << side << " config " << config << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;
	do
	{
		MauMsg msg;
		new (msg.addr()) MauMsg::PerformChecksumMsg(1, config, 1, 0);

		//MauMsg::PerformChecksumMsg* reqMsg = reinterpret_cast<MauMsg::PerformChecksumMsg*>(msg.addr());
		//cout << "PerformChecksumMsg " << *reqMsg << endl;

		if (!m_conn[side])
		{
	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEA :
	    			CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEB;
	    	break;
		}

	    ssize_t rescs = m_gmClient[side]->send(msg.addr(), 512);

	    if (rescs <= 0)
	    {
	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEA :
	    			CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEB;
	    	break;
	    }

	    MauMsg::PerformChecksumRspMsg* rspcs = reinterpret_cast<MauMsg::PerformChecksumRspMsg*>(msg.addr());
		//cout << "PerformChecksumRspMsg " << *rspcs << endl;

	    if (rspcs->error() != 0)
	    {
	    	//cout << "performChecksum(), server response, check sum error " << rspcs->error() << endl;
	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEA :
	    			CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEB;
	    	break;
	    }

	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);

	//cout << "CPHW_MAUS_API_Libh_Impl::performChecksum() returns result <" << result << ">" << endl;
	return result;

}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::installMibs(int side, int config)
{
	//cout << "CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_Impl::installMibs(int side, int config)" << endl;
	//cout << "side " << side << " config " << config << endl;

	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_INTERNAL_ERROR;
	do
	{
		MauMsg msg;
		//cout << "create and send MauMsg::InstallMibsMsg" << endl;
		new (msg.addr()) MauMsg::InstallMibsMsg(1, config, 1, 0);

		//MauMsg::InstallMibsMsg* reqMsg = reinterpret_cast<MauMsg::InstallMibsMsg*>(msg.addr());
		//cout << "InstallMibsMsg " << *reqMsg << endl;

	    ssize_t rescs = m_gmClient[side]->send(msg.addr(), 512);

	    //cout << "result of sending msg is rescs<" << rescs << ">" << endl;

	    if (rescs <= 0)
	    {
	    	//cout << "installMibs(), server does not response from side " << side << endl;
	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_INSTALL_MIBS_FAILED_NODEA :
	    			CPHW_MAUS_API_Libh::RC_INSTALL_MIBS_FAILED_NODEB;
	    	break;
	    }
	    else {
	    	//cout << "installMibs(), server does response from side" << endl;
	    }

	    MauMsg::InstallMibsRspMsg* rspcs = reinterpret_cast<MauMsg::InstallMibsRspMsg*>(msg.addr());
		//cout << "InstallMibsRspMsg " << *rspcs << endl;

	    if (rspcs->error() != 0)
	    {
	    	//cout << "performChecksum(), server response, check sum error " << rspcs->error() << endl;
	    	result = side == SideA ? CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEA :
	    			CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEB;
	    	break;
	    }

	    result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	}
	while (false);


	return result;
}
