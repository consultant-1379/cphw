/*
 * CmdClient.cpp
 *
 *  Created on: May 17, 2015
 *      Author: xdtthng
 */

#include "CmdClient.h"
#include "MauMsg.h"
#include "maus_trace.h"
#include "MAUFunx.h"
#include <iostream>

using namespace std;

const char*	CmdClient::s_gmServerDomain = "MAUS";
const char* CmdClient::s_gmServerName[] = {"Unknown", "CP1MAUM", "CP2MAUM"};

CmdClient::CmdClient(int instance): m_instance(instance),
		m_thisSide(-1),
		m_otherSide(-1),
		m_gmActive(s_gmServerName[m_instance], s_gmServerDomain, 0),
		m_gmPassive(s_gmServerName[m_instance], s_gmServerDomain, 1)
{

	m_thisSide = MAUFunx::getSlotId() - 1;
	m_otherSide = m_thisSide == 0?1:0;

	int nodeState = MAUFunx::getNodeState();

	// Cannot get node state, or this node is active node
	if (nodeState == -1 || nodeState == 1)
	{
		m_gmClient[m_thisSide] = &m_gmActive;
		m_gmClient[m_otherSide] = &m_gmPassive;
	}
	else // This node is passive node
	{
		m_gmClient[m_otherSide] = &m_gmActive;
		m_gmClient[m_thisSide] = &m_gmPassive;
	}


	//m_conn[m_thisSide] = m_gmClient[m_thisSide]->connect();
	//m_conn[m_otherSide] = m_gmClient[m_otherSide]->connect();

	m_conn[m_thisSide] = m_conn[m_otherSide] = -1;

	//cout << "command client ctor, this side " << m_thisSide << " other side " << m_otherSide << endl;
}

CmdClient::~CmdClient()
{

}

Cmd_Error_Code CmdClient::getQuorumInfo(QuorumInfo& qd)
{
	//cout << "CmdClient::getQuorumInfo()" << endl;

	Cmd_Error_Code result = eUnknown;
	ssize_t res = 0;
	do
	{
		//cout << "m_aConn " << m_aConn << " and m_pConn " << m_pConn << endl;

		MauMsg msg;
		//if (m_conn[m_thisSide] == -1)
		if (m_conn[m_thisSide] != 1)
		{
			// Client connect returns bool, true if connected
			m_conn[m_thisSide] = m_gmClient[m_thisSide]->connect();
		}

		if (m_conn[m_thisSide] == 1)
		{
			new (msg.addr()) MauMsg::GetQuorumInfomationMsg(1, 0);
			res = m_gmClient[m_thisSide]->send(msg.addr(), 512);
			// if can fetch data from Active node
			if (res > 0)
			{
				MauMsg::GetQuorumInfomationRspMsg* rsp = reinterpret_cast<MauMsg::GetQuorumInfomationRspMsg*>(msg.addr());
				int errorCode = rsp->error();
				if (errorCode)
				{
					result = errorCode == MauMsg::MauCoreBusy ? eCommandBusy : eMausDataNotAvaliable;
					break;
				}
				//cout << "Result from Active connection" << endl;

				qd.activeMask = rsp->activeMask();
				qd.activeMau = rsp->activeMau();
				qd.connView = rsp->connView();
				qd.quorumView = rsp->quorumView();
				result = eNoError;
				break;
			}
		}


		if (m_conn[m_otherSide] != 1)
		//if (m_conn[m_otherSide] == -1)
		{
			m_conn[m_otherSide] = m_gmClient[m_otherSide]->connect();
		}

		if (m_conn[m_otherSide] == 1)
		{
			// Cannot get data from Active node, try passive node
			new (msg.addr()) MauMsg::GetQuorumInfomationMsg(1, 0);
			res = m_gmClient[m_otherSide]->send(msg.addr(), 512);

			if (res == 0)
			{
				result = eMausDataNotAvaliable;
				break;
			}

			if (res < 0)
			{
				result = eDSDFatalError;
				break;
			}

			MauMsg::GetQuorumInfomationRspMsg* rsp = reinterpret_cast<MauMsg::GetQuorumInfomationRspMsg*>(msg.addr());

			int errorCode = rsp->error();
			if (errorCode)
			{
				result = errorCode == MauMsg::MauCoreBusy ? eCommandBusy : eMausDataNotAvaliable;
				break;
			}
			//cout << "Result from Passive connection" << endl;

			qd.activeMask = rsp->activeMask();
			qd.activeMau = rsp->activeMau();
			qd.connView = rsp->connView();
			qd.quorumView = rsp->quorumView();
			result = eNoError;
			break;
		}

		if (!m_conn[m_thisSide] && !m_conn[m_otherSide])
		{
			//cout << "passive and active nodes are down" << endl;
			result = eServerUnreachable;
			break;
		}

	}
	while (false);
	return result;
}

Cmd_Error_Code CmdClient::setActiveMask(int32_t mask)
{
	Cmd_Error_Code result = eUnknown;
	ssize_t res = 0;
	do
	{

		MauMsg msg;
		if (m_conn[m_thisSide] != 1)
		//if (m_conn[m_thisSide] == -1)
		{
			// Client connect returns bool, true if connected
			m_conn[m_thisSide] = m_gmClient[m_thisSide]->connect();
		}

		if (m_conn[m_thisSide] == 1)
		{
			new (msg.addr()) MauMsg::SetActiveMaskMsg(mask, 1, 0);
			res = m_gmClient[m_thisSide]->send(msg.addr(), 512);
			// if can fetch data from Active node
			if (res > 0)
			{
				MauMsg::SetActiveMasRspMsg* rsp = reinterpret_cast<MauMsg::SetActiveMasRspMsg*>(msg.addr());
				if (!rsp->error())
				{
					//cout << "Result from Active connection " << rsp->error() << endl;

					result = eNoError;
					break;
				}
			}
		}

		//if (m_conn[m_otherSide] == -1)
		if (m_conn[m_otherSide] != 1)
		{
			m_conn[m_otherSide] = m_gmClient[m_otherSide]->connect();
		}

		if (m_conn[m_otherSide] == 1)
		{
			// Cannot get data from Active node, try passive node
			new (msg.addr()) MauMsg::SetActiveMaskMsg(mask, 1, 0);
			res = m_gmClient[m_otherSide]->send(msg.addr(), 512);

			if (res == 0)
			{
				result = eFailToSetEpMask;
				break;
			}

			if (res < 0)
			{
				result = eDSDFatalError;
				break;
			}

			MauMsg::SetActiveMasRspMsg* rsp = reinterpret_cast<MauMsg::SetActiveMasRspMsg*>(msg.addr());

			int errorCode = rsp->error();

			//cout << "Command client errorCode is " << errorCode << endl;
			if (errorCode)
			{
				result = errorCode == MauMsg::MauCoreBusy ? eCommandBusy : eFailToSetEpMask;
				break;
			}
			//cout << "Result from Passive connection" << endl;

			result = eNoError;
			break;
		}

		if (!m_conn[m_thisSide] && !m_conn[m_otherSide])
		{
			//cout << "passive and active nodes are down" << endl;
			result = eServerUnreachable;
			break;
		}

	}
	while (false);
	return result;

}
