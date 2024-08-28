/*
 * CmdClient.cpp
 *
 *  Created on: Nov 3, 2015
 *      Author: xdtthng
 */

#include <sys/poll.h>
#include <cctype>
#include <iostream>

#include "SysconClient.h"
#include "SolSession.h"
#include "AppTrace.h"
#include "ParseIni.h"


using namespace std;


void SysconClient::join()
{
	m_solSession->join();
}

bool SysconClient::isCmdExit() const
{
	return m_solSession->isCmdExit();
}

int SysconClient::getStatus() const
{
	return m_solSession->getStatus();
}


// getConnCount
int SysconClient::getConnCount() const
{
	return m_solSession->getConnCount();
}

SysconClient::SysconClient(int cpId, int side, int multiCpSystem):
		m_cpId(cpId),
		m_side(side),
		m_multiCpSystem(multiCpSystem),
		m_keyboard(),
		m_solSession()

{
	TRACE(("SysconClient::SysconClient() ctor"));

	int savedErrno;
	if (m_keyboard.ttyRaw(savedErrno) == -1)
	{
		//EVENT(EventReporter::SubSystemError,
		//		PROBLEM_DATA("ttyRaw() failed, fd:%d, errno:%d",
		//				m_keyboard.handle(), savedErrno),
		//				PROBLEM_TEXT(""));
		//return;
	}

	TRACE(("SysconClient::SysconClient() ctor returns"));
}


SysconClient::~SysconClient()
{
	TRACE(("SysconClient::~SysconClient() dtor"));

	int savedErrno;
	if (m_keyboard.ttyReset(savedErrno) == -1)
	{
	}

	if (m_solSession)
	{
		m_solSession->stop();
		TRACE(("SysconClient::~SysconClient() stop SolSession; waiting for SolSession::join()"));
		m_solSession->join();
		TRACE(("SysconClient::~SysconClient() SolSession::join() returns"));
		m_solSession.reset();
	}

	// Shutdown SolSession
	SolSession::shutdownSolSession();

	TRACE(("SysconClient::~SysconClient() dtor returns"));
}

void SysconClient::createSol(const std::string& solIpAddr)
{
	TRACE(("SysconClient::createSol()"));

	m_solSession.reset(new SolSession(solIpAddr, m_cpId, m_side, m_multiCpSystem));
	m_solSession->init();
	m_solSession->start();

	TRACE(("SysconClient::createSol() returns"));
}



