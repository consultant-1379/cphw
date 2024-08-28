/*
 * CmdClient.h
 *
 *  Created on: Nov 3, 2015
 *      Author: xdtthng
 */

#ifndef SYSCON_TERMINAL_CLIENT_H_
#define SYSCON_TERMINAL_CLIENT_H_

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <bitset>

#include "CmdError.h"
#include "AppEvent.h"
#include "Keyboard.h"


class SolSession;

// Syscon Terminal Client
class SysconClient
{
public:

	enum StatusT
	{
		OK,
		NotOK,
		Disconnected,
		NoReadableInput
	};

	SysconClient(int cpId, int side, int multiCpSystem);
	~SysconClient();

	void createSol(const std::string& solIpAddr);

	bool isCmdExit() const;
	int getStatus() const;
	int getConnCount() const;
	void join();
	bool getSolConn() const;


private:

	// Description of CP the terminal is connected to
	int		m_cpId;
	int		m_side;
	int		m_multiCpSystem;

	// Handle to the Keyboard associated with STDIN.
	Keyboard 	m_keyboard;

	// Connection to SOL
	boost::shared_ptr<SolSession>	m_solSession;

};

#endif /* SYSCON_TERMINAL_CLIENT_H_ */
