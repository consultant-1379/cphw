/*
 * syscon_server.cxx
 *
 *  Created on: Oct 22, 2015
 *      Author: xdtthng
 */


#include <sys/poll.h>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <string>

#include "syscon_server.h"
#include "AppTrace.h"

// DSD Server domain is CPCON, Name is SYSCON
const std::string Syscon_Server::DSD_SERVER_NAME[2] = {"CPCON", "SYSCON"};


void Syscon_Server::join()
{
	if (m_appThread.joinable())
		m_appThread.join();
}


Syscon_Server::Syscon_Server():
	m_cmwEndEvent(),
	m_appThread(),
	m_terminalServer()

{
	TRACE(("Syscon_Server::Syscon_Server() ctor"));
}

Syscon_Server::~Syscon_Server()
{
	TRACE(("Syscon_Server::~Syscon_Server() dtor"));
	if (m_appThread.joinable())
	{
		m_appThread.join();
		TRACE(("Syscon_Server::~Syscon_Server() m_appThread.join() returns"));
	}


	TRACE(("Syscon_Server::~Syscon_Server() returns"));
}

void Syscon_Server::run()
{
	TRACE(("Syscon_Server::run()"));

	struct pollfd 	fds[1];

    // Cmw stop event from AMF
	fds[0].fd = m_cmwEndEvent.get();
	fds[0].events = POLLIN;

	// Start Terminal Server
	m_terminalServer.reset(new TerminalServer(DSD_SERVER_NAME[1].c_str(), DSD_SERVER_NAME[0].c_str()));
	m_terminalServer->start();

	bool stop = false;
	int res = 0;
	while (!stop)
	{
		res = poll(fds, 1, -1);

		if (res == 0)
		{
			// Time out
			continue;
		}

		if (res < 0)
		{
			TRACE(("Syscon_Server::run() poll error <%d>", errno));
			continue;
		}

		// terminating event
	    if (fds[0].revents & POLLIN )
	    {
	    	// Stop the Application thread
	    	fds[0].revents = 0;
	    	TRACE(("Syscon_Server::run() detects end event"));
	    	//this->closing();
	    	stop = true;
	    	continue;
	    }

	    // Application code starts here
	}

	TRACE(("Syscon_Server::run() returns"));
}

int Syscon_Server::start()
{
	TRACE(("Syscon_Server::start() from cmw"));

	m_appThread = boost::thread(boost::bind(&Syscon_Server::run, this));
	TRACE(("Syscon_Server::start() returns"));

	return 0;
}

// Only post the end event. The thread will close down its application
int Syscon_Server::stop()
{
	TRACE(("Syscon_Server::stop() from cmw"));

	if (m_terminalServer)
	{
		TRACE(("Syscon_Server::stop() shut down terminal server"));
		m_terminalServer->stop();
		m_terminalServer->join();
		m_terminalServer.reset();
	}
	else
	{
		TRACE(("Syscon_Server::stop() terminal server alredy down"));
	}

	m_cmwEndEvent.set();

	TRACE(("Syscon_Server::stop() returns"));
	return 0;
}

