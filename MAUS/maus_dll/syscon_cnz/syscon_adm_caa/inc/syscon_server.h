/*
 * syscon_server.h
 *
 *  Created on: Oct 22, 2015
 *      Author: xdtthng
 */

#ifndef SYSCON_SERVER_H_
#define SYSCON_SERVER_H_

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "boost/shared_ptr.hpp"

#include "AppEvent.h"
#include "TerminalServer.h"

class Syscon_Server
{
public:

	Syscon_Server();
	~Syscon_Server();

	// Start all applications
	int start();

	// Stop all applications
	int stop();

	void join();

private:

	// CMW event to stop all application threads
	AppEvent		m_cmwEndEvent;

	// Application thread
	boost::thread	m_appThread;

	// Listens to message from terminal client
	boost::scoped_ptr<TerminalServer> m_terminalServer;

	// Static data memebers
	static const std::string DSD_SERVER_NAME[2];

	void run();
};

#endif /* SYSCON_SERVER_H_ */
