/*
 * Maus_ApConn.cpp
 *
 *  Created on: Sep 17, 2014
 *      Author: xdtthng
 */


#include <sys/poll.h>
#include <cstring>
#include <new>

#include "Maus_ApConn.h"
#include "maus_trace.h"
#include "MAUMsgQNames.h"
#include "maus_event.h"

extern Maus_Event s_gShutdown;

using namespace std;

ApConn::ApConn(int n, int term):
		//m_conn(n + 2),
		m_conn(n, MSGQ_NAME),
		m_term(term)

{
	TRACE_DEBUG(("ApConn::ApConn()"));
}

ApConn::~ApConn()
{
	TRACE_DEBUG(("ApConn::~ApConn()"));

}

MauReturnCode::Return ApConn::connect()
{
	TRACE_DEBUG(("ApConn::connect()"));

	MauReturnCode::Return ret = MauReturnCode::FAIL;

	ret = m_conn.connect();
	TRACE_DEBUG(("ApConn::connect() connect result <%d>", ret));
	return ret;
}

int ApConn::getOperationalState(int32_t& opState)
{
	TRACE_DEBUG(("ApConn::getOperationalState()"));

	size_t size = m_conn.getMaxCptSigSize();
	memset(m_buffer, 0, size);

	int* info = ::new (m_buffer) int[4];

	info[0] = 2;	// length of info == 2
	info[1] = 1;	// message id == 1, get operational state
	opState = 1;	// not available
	if (this->send(m_buffer, size) == MauReturnCode::OK)
	{
		if (info[2] == 0)
		{
			opState = info[3];
			TRACE_DEBUG(("ApConn::getOperationalState() returns view <%d>", opState));
		}
		else
		{
			TRACE_DEBUG(("ApConn::getOperationalState() cannot get operational state from mau core"));
		}

	}
	else
	{
		TRACE_DEBUG(("ApConn::getOperationalState() fails"));
		info[2] = 1;
	}

	TRACE_DEBUG(("ApConn::getOperationalState() returns"));
	return info[2];
}

int ApConn::getQuorumInfo(int32_t* qinfo, int32_t qsize)
{
	TRACE_DEBUG(("ApConn::getQuorumInfo()"));

	memset(qinfo, 0, qsize*sizeof(int32_t));

	int* info = new (m_buffer) int[8];
	//int* info = new int[8];

	info[0] = 2;	// length of info == 2
	info[1] = 3;	// message id == 3, get quorum information

	size_t size = m_conn.getMaxCptSigSize();
	if (this->send(m_buffer, size) == MauReturnCode::OK)
	{
		if (info[2] == 0)
		{
			memcpy(qinfo, &info[3], qsize*sizeof(int32_t));
			TRACE_DEBUG(("ApConn::getQuorumInfo() collect data ok"));
			TRACE_DEBUG(("ApConn::getQuorumInfo() info <%d %d %d %d>", info[3], info[4], info[5], info[6]));
			TRACE_DEBUG(("ApConn::getQuorumInfo() qinfo <%d %d %d %d>", qinfo[0], qinfo[1], qinfo[2], qinfo[3]));
		}
		else
		{
			TRACE_DEBUG(("ApConn::getQuorumInfo() cannot get operational state from mau core"));
		}

	}
	else
	{
		TRACE_DEBUG(("ApConn::getQuorumInfo() fails"));
		info[2] = 1;
	}

	TRACE_DEBUG(("ApConn::getQuorumInfo() returns"));
	return info[2];
}

int ApConn::setActiveMask(int32_t mask)
{
	TRACE_DEBUG(("ApConn::setActiveMask()"));

	int* info = new (m_buffer) int[8];
	info[0] = 3;			// length of info == 3
	info[1] = 5;			// message id == 5, set active mask
	info[2] = mask;			// mask to be set

	size_t size = m_conn.getMaxCptSigSize();
	if (this->send(m_buffer, size) == MauReturnCode::OK)
	{
		if (info[2] == 0)
		{
			TRACE_DEBUG(("ApConn::setActiveMask() set mask <%d> ok", mask));
		}
		else
		{
			TRACE_DEBUG(("ApConn::setActiveMask() set mask <%d> failed", mask));
		}

	}
	else
	{
		TRACE_DEBUG(("ApConn::setActiveMask() fails"));
		info[2] = 1;
	}

	TRACE_DEBUG(("ApConn::setActiveMask() returns"));
	return info[2];
}

int ApConn::performChecksum(int32_t indicator, int32_t config)
{
	TRACE_DEBUG(("ApConn::performChecksum()"));

	int* info = new (m_buffer) int[8];
	info[0] = 2;			// length of info == 3
	info[1] = 7;			// message id == 7, perform checksum message
	info[2] = 0;			// checksum indicator
	info[3] = 0;

	if (indicator == 1)
	{
		info[2] = indicator;			// checksum indicator
		info[3] = config;
	}

	size_t size = m_conn.getMaxCptSigSize();
	if (this->send(m_buffer, size) == MauReturnCode::OK)
	{
		if (info[2] == 0)
		{
			TRACE_DEBUG(("ApConn::performChecksum()  ok"));
		}
		else
		{
			TRACE_DEBUG(("ApConn::performChecksum() failed"));
		}

	}
	else
	{
		TRACE_DEBUG(("ApConn::performChecksum() failed"));
		info[2] = 1;
	}
	TRACE_DEBUG(("ApConn::performChecksum() returns"));
	return info[2];
}

MauReturnCode::Return ApConn::send(char* buffer, size_t& size)
{
	TRACE_DEBUG(("ApConn::send()"));
	MauReturnCode::Return ret = MauReturnCode::FAIL;
	ret = m_conn.send(buffer, size);

	if (ret != MauReturnCode::OK)
	{
		TRACE_DEBUG(("ApConn::send() send results with error <%d>", ret));
		return ret;
	}

	struct pollfd fds[2];
	//fds[0].fd = m_term;
	fds[0].fd = s_gShutdown.get();
	fds[0].events = POLLIN;
	mqd_t fd;
	ret = m_conn.getDescriptor(MAUMsgQ::LocalMAUInboundQ, fd);
	if (ret != MauReturnCode::OK)
	{
		TRACE_DEBUG(("ApConn::send() failed to get inboud file descriptor with error <%d>", ret));
		return ret;
	}

	fds[1].fd = fd;
	fds[1].events = POLLIN;

	int res = poll(fds, 2, -1);

	if (res <= 0)
	{
		TRACE_DEBUG(("ApConn::send() poll error <%d>", errno));
		return MauReturnCode::FAIL;
	}

	// terminating event
    if (fds[0].revents & POLLIN )
    {
    	//cout << "ready to call msg queue received, msg len set to " << *msg_len << endl;
    	fds[0].revents = 0;
    	TRACE_DEBUG(("Maus_CommandServer::connect() detects end event, returns <1>"));
    	return MauReturnCode::OK;
    }


	size_t recSize = MAUMsgQ::MaxMsgSize;
	memset(buffer, 0, recSize);
	ret = m_conn.poll(buffer, &recSize);
	if (ret != MauReturnCode::OK)
	{
		TRACE_DEBUG(("ApConn::send() failed to receive the returned message with error <%d>", ret));
		return ret;
	}

	TRACE_DEBUG(("ApConn::send() returned message size is <%d>", recSize));

	int* info = new (buffer) int[7];
	TRACE_DEBUG(("info[0] <%d>", info[0]));
	TRACE_DEBUG(("info[1] <%d>", info[1]));
	TRACE_DEBUG(("info[2] <%d>", info[2]));
	TRACE_DEBUG(("info[3] <%d>", info[3]));
	TRACE_DEBUG(("info[4] <%d>", info[4]));
	TRACE_DEBUG(("info[5] <%d>", info[5]));
	TRACE_DEBUG(("info[6] <%d>", info[6]));

	TRACE_DEBUG(("ApConn::send() returns ok"));
	return MauReturnCode::OK;
}

int ApConn::setFCState(int32_t fcState)
{
	TRACE_DEBUG(("ApConn::setFCState()"));

	int* info = new (m_buffer) int[8];
	info[0] = 3;			// length of info == 3
	info[1] = 9;			// message id == 5, set active mask
	info[2] = fcState;		// mask to be set

	size_t size = m_conn.getMaxCptSigSize();
	if (this->send(m_buffer, size) == MauReturnCode::OK)
	{
		if (info[2] == 0)
		{
			TRACE_DEBUG(("ApConn::setFCState() set FC State <%d> ok", fcState));
		}
		else
		{
			TRACE_DEBUG(("ApConn::setFCState() set FC State <%d> failed", fcState));
		}

	}
	else
	{
		TRACE_DEBUG(("ApConn::setFCState() fails"));
		info[2] = 1;
	}

	TRACE_DEBUG(("ApConn::setFCState() returns"));
	return info[2];
}


int ApConn::installMibs(int32_t indicator, int32_t config)
{
	TRACE_DEBUG(("ApConn::installMibs()"));

	int* info = new (m_buffer) int[8];
	info[0] = 2;			// length of info == 3
	info[1] = 11;			// message id == 7, perform checksum message
	info[2] = 0;			// operation indicator
	info[3] = 0;			// config

	if (indicator == 1)
	{
		info[2] = indicator;			// checksum indicator
		info[3] = config;
	}

	size_t size = m_conn.getMaxCptSigSize();
	if (this->send(m_buffer, size) == MauReturnCode::OK)
	{
		if (info[2] == 0)
		{
			TRACE_DEBUG(("ApConn::installMibs()  ok"));
		}
		else
		{
			TRACE_DEBUG(("ApConn::installMibs() failed"));
		}

	}
	else
	{
		TRACE_DEBUG(("ApConn::installMibs() failed"));
		info[2] = 1;
	}
	TRACE_DEBUG(("ApConn::installMibs() returns"));
	return info[2];
}


//////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------------
// Keep for reference as why they might not work
#if 0
void ApConn::flush()
{
	TRACE_DEBUG(("ApConn::flush()"));

	char buffer[512];
	size_t recSize = MAUMsgQ::MaxMsgSize;
	MauReturnCode::Return res = m_conn.poll(buffer, &recSize);

	TRACE_DEBUG(("ApConn::flush() discarding msg content returns <%d>", res));

	while (res == MauReturnCode::OK)
	{
		recSize = MAUMsgQ::MaxMsgSize;
		res = m_conn.poll(buffer, &recSize);
		TRACE_DEBUG(("ApConn::flush() discarding msg content returns <%d>", res));
	}

	TRACE_DEBUG(("ApConn::flush() returns"));
}
#endif
