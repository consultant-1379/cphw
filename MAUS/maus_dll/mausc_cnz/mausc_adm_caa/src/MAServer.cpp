/*
 * MAServer.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: xdtthng
 */

#include <string>
#include "boost/bind.hpp"
#include <boost/thread/thread.hpp>

#include "MAServer.h"
#include "maus_trace.h"
#include "Maus_AehEvent.h"
#include "CpMauMsg.h"
#include "acs_prc_api.h"
#include "CpMauData.h"

#define Every_Ntimes	if (s_traceCount%150 == 0)
#define Step_Trace_Count (++s_traceCount)

// #include "MausOM.h"

using namespace std;

static unsigned int s_traceCount = 0;

MAServer::MAServer(void): TCPServer(),
	m_mutex(),
	m_condition(),
	m_serverThread()
{

}

MAServer::~MAServer(void)
{
	TRACE(("MAServer::~MAServer()"));

	if (m_serverThread.joinable())
	{
		TRACE(("MAServer::~MAServer() joining main thread"));
		m_serverThread.join();
	}
	TRACE(("MAServer::~MAServer() m_serverThread joins; dtor returns "));
}

void MAServer::start(void)
{
    TRACE(("MAServer::start()"));

    m_serverThread = boost::thread(boost::bind(&MAServer::run, this));
	TRACE(("MAServer::start() returns"));

}

void MAServer::stop(void)
{
    TRACE(("MAServer::stop()"));
    TCPServer::stop();
}

void MAServer::waitUntilRunning(void)
{
    TRACE(("BUServer::waitUntilRunning()"));

    boost::unique_lock<boost::mutex> lock(m_mutex);

    while (m_running == false)
    {
        TRACE(("Wait until MAServer Thread signal"));
        m_condition.wait(lock);
        TRACE(("MAServer Thread signaled"));
    }
}

void MAServer::run(void)
{
    TRACE(("MAServer::run(void)"));

    // Use condition variable to notify other thread
    // that server is started
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        m_running = true;
        //m_fatal_error = 0;
    }
    m_condition.notify_one();


    //if (isTRACEABLE()) {
    {
        std::ostringstream ss;
        ss << "MAServer::run() thread id "<< boost::this_thread::get_id();
        TRACE(("%s", ss.str().c_str()));
    }

	TCPServer::run();

    m_running = false;
    TRACE(("MAServer::run(void) returns", 0));
}

void MAServer::incoming_data_handler(TCPConnection_ptr p, const char* data, std::size_t num)
{
	Step_Trace_Count;

	Every_Ntimes
    TRACE(("MAServer::incoming_data_handler with size %u", num));

    const CpMauMsg::MsgHeader* header = reinterpret_cast<const CpMauMsg::MsgHeader*>(data);
	Every_Ntimes
    TRACE(("MAServer::incomingData(), Primitive is <%d>", header->primitive()));

    switch(header->primitive())
    {
    case CpMauMsg::GetFexRq:
    {
    	Every_Ntimes
    	TRACE(("MAServer::incomingData() process get fex req"));
    	processGetFexReqMsg(p, data);
    	break;
    }
    case CpMauMsg::SetFexRq:
    {
    	Every_Ntimes
    	TRACE(("MAServer::incomingData() process set fex req"));
    	processSetFexReqMsg(p, data);
    	break;
    }

    // Removed but kept
    // ----------------
    //case CpMauMsg::GetAdminState:
    //{
    //	TRACE(("MAServer::incomingData() process GetAdminStateReqMsg"));
    //	processGetAdminStateReqMsg(p, data);
    //	break;
    //}

    case CpMauMsg::GetApgStateRq:
    {
    	Every_Ntimes
    	TRACE(("MAServer::incomingData() process GetAdminStateReqMsg"));
    	processGetApgStateReqMsg(p, data);
    	break;
    }

    default:
    	break;
    }
    Every_Ntimes
    TRACE(("MAServer::incoming_data_handler() returns"));
}

void MAServer::processGetFexReqMsg(TCPConnection_ptr p, const char* data)
{
	//Step_Trace_Count;

	Every_Ntimes
	TRACE(("MAServer::processGetFexReqMsg()"));

    // Read information from incoming message
    CpMauMsg msg;
    memcpy(msg.addr(), data, CpMauMsg::MSG_SIZE_IN_BYTES);
    const CpMauMsg::GetFexReqMsg* reqMsg = reinterpret_cast<const CpMauMsg::GetFexReqMsg*>(msg.addr());
    ostringstream ss;
    ss << *reqMsg;
    Every_Ntimes
    TRACE(("%s", ss.str().c_str()));

    int seqno = reqMsg->seqNo();
    int serviceIdCode = reqMsg->serviceIdCode();
    Every_Ntimes
    TRACE(("MAServer::processGetFexReqMsg(), seqno <%d> serviceIdCode <%d>", seqno, serviceIdCode));
    int verMaj = reqMsg->version().major();
    int verMin = reqMsg->version().minor();
    Every_Ntimes
	TRACE(("MAServer::processGetFexReqMsg(), verMaj <%d> verMin <%d>", verMaj, verMin));

    // Set data for response message.
    // Default values are for error cases
    int fex = -1;
    int error = 1;
    CpMauData::CpTypeT cpType = CpMauData::CpInvalid;
    CpMauMsg::GetFexRspMsg* rsp = 0;
    msg.reset();

    if (verMaj != 1 || verMin != 0 ||
    		(serviceIdCode != 1 && serviceIdCode != 2))
    {
    	TRACE(("Error encountered, either in version or serviceIdCode"));
    	// Will set proper error code later
        fex = -1;
        error = 255;
        if (verMaj != 1 || verMin != 0 )
        {
        	error = 1;
        }
        else if (serviceIdCode != 1 && serviceIdCode != 2)
        {
        	error = 2;
        }
        rsp =  ::new (msg.addr()) CpMauMsg::GetFexRspMsg(error, fex, seqno);
    }
    else
    {
        cpType =  serviceIdCode == 1? CpMauData::Cp1 : CpMauData::Cp2;
        bool res = CpMauData::readFexRequest(fex, cpType);
        Every_Ntimes
        TRACE(("MAServer::processGetFexReqMsg(), fex req from ini file <%d>", fex));
        if (res)
        {
        	rsp =  ::new (msg.addr()) CpMauMsg::GetFexRspMsg(0, fex, seqno);
        }
        else
        {
        	rsp =  ::new (msg.addr()) CpMauMsg::GetFexRspMsg(3, -1, seqno);
        }
    }

    ss.str("");
    ss.clear();
    ss << *rsp;
    Every_Ntimes
    TRACE(("%s", ss.str().c_str()));
    p->send(msg.addr(), 512);

    Every_Ntimes
    TRACE(("MAServer::processGetFexReqMsg() returns"));
}

void MAServer::processSetFexReqMsg(TCPConnection_ptr p, const char* data)
{
	//Step_Trace_Count;

	Every_Ntimes
	TRACE(("MAServer::processSetFexReqMsg()"));

    CpMauMsg msg;
    memcpy(msg.addr(), data, CpMauMsg::MSG_SIZE_IN_BYTES);
    const CpMauMsg::SetFexReqMsg* reqMsg = reinterpret_cast<const CpMauMsg::SetFexReqMsg*>(msg.addr());
    ostringstream ss;
    ss << *reqMsg;
    Every_Ntimes
    TRACE(("%s", ss.str().c_str()));

    int seqno = reqMsg->seqNo();
    int verMaj = reqMsg->version().major();
    int verMin = reqMsg->version().minor();
    int serviceIdCode = reqMsg->serviceIdCode();
	int fex = reqMsg->fexReqState();
	Every_Ntimes
    TRACE(("MAServer::processGetFexReqMsg(), seqno <%d> serviceIdCode <%d> fex <%d>", seqno, serviceIdCode, fex));
	Every_Ntimes
	TRACE(("MAServer::processGetFexReqMsg(), verMaj <%d> verMin <%d>", verMaj, verMin));

    // Set data for response message.
    // Default values are for error cases
    int error = 1;
    CpMauData::CpTypeT cpType = CpMauData::CpInvalid;
    CpMauMsg::SetFexRspMsg* rsp = 0;
    msg.reset();

    if (verMaj != 1 || verMin != 0 ||
    		(fex < 0 || fex > 2) ||
    		(serviceIdCode != 1 && serviceIdCode != 2))
    {
    	TRACE(("Error encountered, either in version or serviceIdCode"));
    	// Will set proper error code later
        error = 255;
        if (verMaj != 1 || verMin != 0 )
        {
        	error = 1;
        }
        else if (serviceIdCode != 1 && serviceIdCode != 2)
        {
        	error = 2;
        }
        else if (fex < 0 || fex > 2)
        {
        	error = 3;
        }
        rsp =  ::new (msg.addr()) CpMauMsg::SetFexRspMsg(error, seqno);
    }
    else
    {
        cpType =  serviceIdCode == 1? CpMauData::Cp1 : CpMauData::Cp2;
        bool res = CpMauData::setFexRequest(fex, cpType);
        Every_Ntimes
        TRACE(("MAServer::processSetFexReqMsg(), setFexRequest() returns res <%d>", res));
        if (res)
        {
        	rsp =  ::new (msg.addr()) CpMauMsg::SetFexRspMsg(0, seqno);
        }
        else
        {
        	rsp =  ::new (msg.addr()) CpMauMsg::SetFexRspMsg(4, seqno);
        }
    }

    ss.str("");
    ss.clear();
    ss << *rsp;
    Every_Ntimes
    TRACE(("%s", ss.str().c_str()));
    p->send(msg.addr(), 512);

    Every_Ntimes
    TRACE(("MAServer::processSetFexReqMsg() returns"));
}

void MAServer::processGetApgStateReqMsg(TCPConnection_ptr p, const char* data)
{
	//Step_Trace_Count;

    // Read information from incoming message
    CpMauMsg msg;
    memcpy(msg.addr(), data, CpMauMsg::MSG_SIZE_IN_BYTES);
    const CpMauMsg::GetApgStateReqMsg* reqMsg = reinterpret_cast<const CpMauMsg::GetApgStateReqMsg*>(msg.addr());
    ostringstream ss;
    ss << *reqMsg;
    Every_Ntimes
    TRACE(("%s", ss.str().c_str()));

    int seqno = reqMsg->seqNo();
    Every_Ntimes
    TRACE(("MAServer::processGetApgStateReqMsg(), seqno <%d>", seqno));
    int verMaj = reqMsg->version().major();
    int verMin = reqMsg->version().minor();
    Every_Ntimes
	TRACE(("MAServer::processGetApgStateReqMsg(), verMaj <%d> verMin <%d>", verMaj, verMin));

    // Set data for response message.
    // Default values are for error cases
    int apaState = -1;
    int apbState = -1;
    int error = 1;
    CpMauMsg::GetApgStateRspMsg* rsp = 0;
    msg.reset();

    if (verMaj != 1 || verMin != 0)
    {
    	TRACE(("Error encountered, either in version or serviceIdCode"));
    	// Will set proper error code later
    	apaState = apbState = -1;
        error = 255;
        if (verMaj != 1 || verMin != 0 )
        {
        	error = 1;
        }
        rsp =  ::new (msg.addr()) CpMauMsg::GetApgStateRspMsg(error, apaState, apbState, seqno);
    }
    else
    {
    	ACS_PRC_API prcApi;
		apaState = prcApi.askForNodeState("SC-2-1");
		if (apaState == -1)
		{
			TRACE(("askForNodeState(\"SC-2-1\") error <%s>", prcApi.getLastErrorText()));
		}
		apbState = prcApi.askForNodeState("SC-2-2");
		if (apbState == -1)
		{
			TRACE(("askForNodeState(\"SC-2-2\") error <%s>", prcApi.getLastErrorText()));
		}

		Every_Ntimes
        TRACE(("MAServer::processGetApgStateReqMsg(), apa state <%d>", apaState));
		Every_Ntimes
        TRACE(("MAServer::processGetApgStateReqMsg(), apb state <%d>", apbState));
        rsp =  ::new (msg.addr()) CpMauMsg::GetApgStateRspMsg(0, apaState,  apbState, seqno);

    }

    ss.str("");
    ss.clear();
    ss << *rsp;
    Every_Ntimes
    TRACE(("%s", ss.str().c_str()));
    p->send(msg.addr(), 512);

    Every_Ntimes
    TRACE(("MAServer::processGetApgStateReqMsg() returns"));

}

