/*
 * TerminalSession.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: xdtthng
 */

#include <sys/poll.h>

#include "TerminalServer.h"
#include "TerminalSession.h"
#include "AppTrace.h"
#include "SolSession.h"
#include "ScoMsg.h"

using namespace std;

int TerminalSession::s_termSessionId = 0;

TerminalSession::TerminalSession(int cpid, int side, int handle, SessionPtr session,
		const string& ipAddr, TerminalServer* termServer):
	m_cpId(cpid),
	m_side(side),
	m_handle(handle),
	m_session(session),
	m_ipAddr(ipAddr),
	m_termServer(termServer),
	m_endEvent(),
	m_termDataAvailable(),
	m_solDataAvailable(),
	m_thread(),
	m_termQueue(),
	m_solQueue(),
	m_solSession(),
	m_id(s_termSessionId++)
{
	TRACE(("TerminalSession::TerminalSession() ctor"));
	TRACE(("TerminalSession::TerminalSession() ctor returns"));
}

TerminalSession::~TerminalSession()
{
	TRACE(("TerminalSession::TerminalSession() dtor"));

	if (m_thread.joinable())
	{
		m_thread.join();
	}

	TRACE(("TerminalSession::TerminalSession() dtor returns"));
}


void TerminalSession::run()
{
	//TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
	//TRACE(("----------------------------"));
	TRACE(("TerminalSession::run() m_id <%d>", m_id));
	//TRACE(("----------------------------"));
	//TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>"));

	struct pollfd 	fds[2];

	// Stop syscon client thread
	fds[0].fd = m_endEvent.get();
	fds[0].events = POLLIN;

	// Keyboard input available
	//fds[2].fd = m_termDataAvailable.get();
	//fds[2].events = POLLIN;

	// SOL data available
	fds[1].fd = m_solDataAvailable.get();
	fds[1].events = POLLIN;

	bool stop = false;
	int res = 0;
	while (!stop)
	{
		res = poll(fds, 2, -1);

		if (res == 0)
		{
			// Time out
			continue;
		}

		if (res < 0)
		{
			TRACE(("TerminalSession::run() poll error <%d>", errno));
			continue;
		}

		// terminating event
		if (fds[0].revents & POLLIN )
		{
			// Stop the Application thread
			fds[0].revents = 0;
			TRACE(("TerminalSession::run() detects end event"));
			stop = true;
			continue;
		}


		// SOL data from CPUB has arrived
		if (fds[1].revents & POLLIN )
		{
			//TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
			//TRACE(("TerminalSession::run() SOL data arrived"));
			//TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));

			// Input ready
			fds[1].revents = 0;
			m_solDataAvailable.reset();

			string buffer;
			bool res = m_solQueue.try_pop(buffer);

			//TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
			//TRACE(("TerminalSession::run() SOL data <%s>", buffer.c_str()));
			//TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
			//if (buffer.length() < 15)
			{
				//TRACE(("======================================"));
				//TRACE(("SOL data <%s>", buffer.c_str()));
				//TRACE(("======================================"));
			}

			if (!res)
			{
				// This is not possible; there should be no action here
				TRACE(("TerminalSession::run() detects fatal error in CQueue::try_pop()"));
			}

			// Make this data available to syscon client
			ScoMsg msg;
			::new (msg.addr()) ScoMsg::Sco_DataMsg(buffer.length(), buffer.c_str());
			size_t ressize = m_session->send(msg.addr(), 2048);

			TRACE(("TerminalSession::run() send data to client size <%d>", ressize));

			if (ressize <= 0)
			{

			}

			continue;
		}

		// Client data from keyboard has arrived

#if 0

		if (fds[2].revents & POLLIN )
		{
			TRACE(("------------------------------------------"));
			TRACE(("TerminalSession::run() client data arrived"));
			TRACE(("-------------------------------------------"));

			fds[2].revents = 0;
			m_termDataAvailable.reset();

			string buffer;
			bool res = m_termQueue.try_pop(buffer);
			if (res)
			{
				TRACE(("TerminalSession::run() client data <%s>", buffer.c_str()));
			}
			else {
				TRACE(("TerminalSession::run() client data recieiving error"));
			}

			continue;
		}
#endif

	}

	TRACE(("TerminalSession::run() m_id <%d> returns", m_id));
}

int TerminalSession::start()
{
	TRACE(("TerminalSession::start()"));

	m_thread = boost::thread(boost::bind(&TerminalSession::run, this));

	TRACE(("TerminalSession::start() returns"));

	return 0;
}

int TerminalSession::startSolSession()
{
	TRACE(("TerminalSession::startSolSession()"));

	// To be touch up later
	// --------------------
	// Create SOL Session towards CPUB
	m_solSession.reset(new SolSession(m_ipAddr, this));
	m_solSession->init();
	m_solSession->start();
	//
	// --------------------
	TRACE(("TerminalSession::startSolSession() creates solSession id <%d>", m_solSession->getId()));
	TRACE(("TerminalSession::startSolSession() returns"));
	return 0;
}

int TerminalSession::stop()
{
	TRACE(("TerminalSession::stop()"));

	if (m_solSession)
	{
		m_solSession->stop();
	}

	m_endEvent.set();

	TRACE(("TerminalSession::stop() returns"));
	return 0;
}


void TerminalSession::terminalIncomingData(const string& data)
{
	//TRACE(("TerminalSession::terminalIncomingData()"));

	m_termQueue.push(data);
	m_termDataAvailable.set();

	//TRACE(("TerminalSession::terminalIncomingData() term session id <%d> returns", m_id));
}

void TerminalSession::solIncomingData(const string& data)
{
	//TRACE(("TerminalSession::solIncomingData()"));

	m_solQueue.push(data);
	m_solDataAvailable.set();

	//TRACE(("TerminalSession::solIncomingData() returns"));
}
