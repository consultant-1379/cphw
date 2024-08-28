/*
 * SolConnect.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: xdtthng
 */


//#include <string>
#include <iostream>

#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include "SolConnect.h"
#include "AppTrace.h"
#include "SolSession.h"
#include "ParseIni.h"
#include "GepFunx.h"
#include "SysFunx.h"
#include "CmdFunx.h"

using namespace std;

string SolConnect::s_ipNetName[] = {"169", "170"};
string SolConnect::s_connIniPath = "/data/cphw/data/syscon/conndata.ini";

SolConnect::SolConnect(int cpId, int side, int multiCpSystem): m_echo(), m_terminal(),
		m_cpId(cpId),
		m_side(side),
		m_multiCpSystem(multiCpSystem),
		//m_keyboard(),
		m_solConn(false),
		m_ipAddrTable()
{
	TRACE(("SolConnect::SolConnect()"));
	m_ipAddress[0] = "";
	m_ipAddress[1] = "";
	m_ipError[0] = m_ipError[1] = -255;
	m_solError[0] = m_solError[1] = -255;

	TRACE(("SolConnect::SolConnect() returns"));
}

SolConnect::~SolConnect()
{
	TRACE(("SolConnect::~SolConnect()"));

	if (m_solConn)
	{
		m_solConn = false;
	}

	TRACE(("SolConnect::~SolConnect() returns"));
}


void SolConnect::init()
{

	// Adjusting side information
	if (m_cpId < 64 && m_cpId >=0)
	{
		m_side = 0;
	}

	if (!m_multiCpSystem)
	{
		m_cpId = 1001;
	}
	m_terminal.reset(new SysconClient(m_cpId, m_side, m_multiCpSystem));
	int error = 0;
	do
	{
		// Check if IP address is defined
		if (!m_ipAddrTable.find(m_cpId, m_side, m_ipAddress[0], m_ipAddress[1]))
		{
			error = 1;
			break;
		}

		if (isSolConnect())
		{
			error = 2;
			break;
		}
	}
	while (false);


	if (error)
	{
		switch (error)
		{
		case 1:
			throw eSolIPAddressNotDefined;

		case 2:
			throw eTermConnectionExsits;

		default:
			throw eUnknown;
			break;
		}
	}

	m_echo.setIpAddr(m_ipAddress[0].c_str(), m_ipAddress[1].c_str());
}

// This implementation might change
inline
void SolConnect::ipCheck()
{
	m_ipError[0] = m_echo.ping(0, ECHO_REPEAT);
	m_ipError[1] = m_echo.ping(1, ECHO_REPEAT);

	return;
}

// Return
// 100 	exit
// 1	both subnets are down
// 2	Sol cannot be conatacted via both network, initially
// 3	Sol cannot be conatacted via both network after one succesfull connection
//
int SolConnect::connect(int index)
{
	int result = -255;

	m_ipError[0] = m_ipError[1] = -255;
	m_solError[0] = m_solError[1] = -255;
	int count = 0;
	do
	{

		TRACE(("SolConnect::connect() loop for index <%d>", index));
		for (int i = 0; i < 2; ++i)
		{
			this->ipCheck();
			// If both networks down, exit
			if (m_ipError[0] != 0 && m_ipError[1] != 0)
			{
				if (i == 0)
				{
					m_ipError[0] = m_ipError[1] = -255;
					continue;
				}
				result = 1;
				break;
			}
			else
			{
				break;
			}
		}
		if (result ==1)
			break;

		// If only one network is up, select the up network
		if ((m_ipError[0] == 0 && m_ipError[1] != 0) ||
				(m_ipError[0] != 0 && m_ipError[1] == 0))
			index = m_ipError[0] == 0 ? 0 : 1;

		TRACE(("SolConnect::connect() create sol <%s>", m_ipAddress[index].c_str()));

		cout << "Attempting to connect to SOL on " << s_ipNetName[index] << " network. Please wait ...." << endl;
		m_terminal->createSol(m_ipAddress[index]);
		m_terminal->join();

		m_solError[index] = m_terminal->getStatus();

		TRACE(("SolConnect::connect() sol status <%d> sol cmd exit <%d>", m_solError[index], m_terminal->isCmdExit()));
		if (m_terminal->isCmdExit())
		{
			result = 100;
			break;
		}

		// Sol connection is not possible on both 169 and 170 network
		if (m_solError[0] == SolSession::INIT_ERROR && m_solError[1] == SolSession::INIT_ERROR)
		{
			TRACE(("SolConnect::connect() init error on both subnets"));
			result = 2;
			break;
		}

		// Sol connection is not possible on both 169 and 170 network
		if (m_solError[0] > 0 && m_solError[1]> 0)
		{
			TRACE(("SolConnect::connect() sol errors via both subnets"));
			result = 3;
			break;
		}

		TRACE(("SolConnect::connect() sleeps before another try"));
		index ^= 1;
		++count;
		//sleep(32);
		sleep(1);
		TRACE(("SolConnect::connect() wakes up"));
	}
	while (count < 2);

	return result;
}

int SolConnect::getCpId(const string& aline)
{
	string line = aline;
	line.erase(0, 2);
	boost::trim(line);
	int cpId = -1;
	cpId = SysFunx::cpNameToCpId(line.c_str());

	return cpId;
}


int SolConnect::getSide(const string& aline)
{
	string line = aline;
	int side = -1;
	line.erase(0, 1);
	boost::trim(line);

	if (line == "a" || line == "cpa")
	{
		side = 0;
	}
	else if (line == "b" || line == "cpb")
	{
		side = 1;
	}

	return side;
}

bool SolConnect::isSolConnect()
{
	pid_t pid = getpid();
	string cmd = "ps -ef | grep syscon | grep -v grep | grep -v bash | grep -v sudo | grep -v ";
	cmd += dynamic_cast<ostringstream &>(ostringstream() << pid).str();
	vector<string> lines;
	CmdFunx::execCmd(cmd, lines);
	//copy(lines.begin(), lines.end(), ostream_iterator<string>(cout, "\n"));

	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> sep("-");

	bool result = false;
	for(size_t idx = 0; idx < lines.size(); ++idx)
	{
		Tokenizer tok(lines[idx], sep);
		int cpId = -1;
		int side = -1;
		int add = -1;
		size_t pos;
		for (Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
		{
			pos = it->find("cp");
			if (pos != string::npos && pos == 0)
			{
				cpId = getCpId(*it);
				continue;
			}
			pos = it->find("s");
			if (pos != string::npos && pos == 0)
			{
				side = getSide(*it);
				continue;
			}

			pos = it->find("a");
			if (pos != string::npos && pos == 0)
			{
				add = 0;
				continue;
			}
		}
		if (cpId <= 63 && cpId >=0)
		{
			side = 0;
		}
		if (!m_multiCpSystem)
		{
			cpId = 1001;
		}

		result = (add == 0) && (side == m_side) && (cpId == m_cpId);
		if (result)
			break;
	}

	return result;
}

