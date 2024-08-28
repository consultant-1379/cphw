/*
 * CptSwMau.cpp
 *
 *  Created on: May 9, 2016
 *      Author: xdtthng
 */

#include "CptSwMau.h"
#include "CptSwMauMsg.h"
#include "maus_trace.h"
#include <iostream>

using namespace std;

const char*	CptSwMau::s_gmServerDomain = "CPT";
const char* CptSwMau::s_gmServerName = "SwMau";

CptSwMau::CptSwMau(int instance): m_instance(instance),
		m_conn(false),
		m_client(s_gmServerName, s_gmServerDomain, 0)
{
	TRACE(("CptSwMau::CptSwMau() ctor"));
	m_conn = m_client.connect();
	TRACE(("CptSwMau::CptSwMau() return, m_conn bool <%d>", m_conn));
}

bool CptSwMau::ptcoiExist()
{
	TRACE(("CptSwMau::ptcoiExist()"));
	boost::mutex::scoped_lock lock(s_cs);
	int32_t ptcoiStatus = NoPtcoiSession;
	do
	{
		if (!isConnected())
		{
			TRACE(("CptSwMau::ptcoiExist(), no connection to DSD Server in CPT"));
			ptcoiStatus = NoPtcoiSession;
			break;
		}

		CptSwMauMsg msg;
		CptSwMauMsg::GetPtcoiReqMsg* cmd = new (msg.addr()) CptSwMauMsg::GetPtcoiReqMsg(m_instance, 0);
	    //cout << *cmd << endl;
		(void) cmd;
	    ssize_t res = m_client.send(msg.addr(), CptSwMauMsg::MSG_SIZE_IN_BYTES);
		//cout << "CptSwMauMsg::GetPtcoiReqMsg, send result <" << res << ">" << endl;
	    TRACE(("CptSwMau::ptcoiExist(), m_client.send() returns res <%d>", res));

	    if (res <= 0)
	    {
	    	TRACE(("CptSwMau::ptcoiExist(), error when sending message to DSD Server in CPT"));
	    	ptcoiStatus = NoPtcoiSession;
	    	break;
	    }

	    CptSwMauMsg::GetPtcoiRspMsg* rsp = reinterpret_cast<CptSwMauMsg::GetPtcoiRspMsg*>(msg.addr());
	    //cout << "response message" << endl;
	    //cout << *rsp << endl;
	    TRACE(("CptSwMau::ptcoiExist(), GetPtcoiRspMsg, error <%d>, ptcoiStatus", rsp->error(), rsp->ptcoiStatus()));
	    if (rsp->error())
	    {
	    	TRACE(("CptSwMau::ptcoiExist(), ptcoi sessin return error code <%d>", rsp->error()));
	    	ptcoiStatus = NoPtcoiSession;
	    	break;

	    }

	    ptcoiStatus = rsp->ptcoiStatus();
	}
	while (false);

	TRACE(("CptSwMau::ptcoiExist() returns ptcoiStatus<%d>", ptcoiStatus));
	return ptcoiStatus == PtcoiSessionExists;
}
