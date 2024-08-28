/*
 * cmd_syscon_main.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include "boost/asio/ip/address_v4.hpp"
#include <boost/shared_ptr.hpp>
#include <cstring>

#include "CmdError.h"
#include "SysFunx.h"
#include "SysconClient.h"
#include "AppTrace.h"
#include "ScoMsg.h"
#include "Funx.h"
#include "ScoPinger.h"
#include "DmxGepInfo.h"

#include "ACS_CS_API.h"
#include "boost/regex.hpp"
#include "SolSession.h"


using namespace std;

static const char* s_traceName = "SYSCON";

typedef boost::tuple<Cmd_Error, string, string, int> InvalidValueT;

extern int opterr;
static int s_multiCpSystem = -1;


class SolConnect
{
public:
	enum N100
	{
		ECHO_REPEAT = 10
	};

	SolConnect(int cpId, int side, int multiCpSystem);
	~SolConnect();
	void init();

	void ipCheck();
	int connect(int index);
	void createTerminal();

	int getIpErrCode(int index)
	{
		return m_ipError[index];
	}

	string getIpAddr(int index)
	{
		return m_ipAddress[index];
	}


private:

	ScoPinger 						m_echo;
	boost::scoped_ptr<SysconClient> m_terminal;

	int			m_cpId;
	int 		m_side;
	int			m_multiCpSystem;
	string 		m_ipAddress[2];
	int 		m_ipError[2];
	int 		m_solError[2];
};

SolConnect::SolConnect(int cpId, int side, int multiCpSystem): m_echo(), m_terminal(),
		m_cpId(cpId),
		m_side(side),
		m_multiCpSystem(multiCpSystem)
{
	m_ipAddress[0] = "";
	m_ipAddress[1] = "";
	m_ipError[0] = m_ipError[1] = -255;
	m_solError[0] = m_solError[1] = -255;
}

SolConnect::~SolConnect()
{

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

	cout << "syscon execution simulation, connect" << endl;
	cout << "cpId:\t" << m_cpId << endl;
	cout << "side:\t" << m_side << endl;
	cout << "multiCpSystem:\t" << m_multiCpSystem << endl;
	//return;

	/*
	m_terminal.reset(new SysconClient(m_cpId, m_side, m_multiCpSystem));

	//terminal.reset(new SysconClient(cpId, side, multiCpSystem));
	if (!m_terminal->connect())
		throw eServerUnreachable;
	*/

	this->createTerminal();
	cout << "syscon sends Sco_ConnectMsg" << endl;

	ScoMsg msg;
	new (msg.addr()) ScoMsg::Sco_ConnectMsg(m_cpId, m_side, m_multiCpSystem, 5678, 1, 0);
	size_t res = m_terminal->send_blocking(msg.addr(), 2048);

	cout << "syscon sends Sco_ConnectMsg returns with result <" << res << ">" << endl;

	if (res <= 0)
	{
		throw eServerUnreachable;
	}
	ScoMsg::Sco_ConnectRspMsg* rsp = reinterpret_cast<ScoMsg::Sco_ConnectRspMsg*>(msg.addr());
	int error = rsp->error();

	if (error)
	{
		switch (error)
		{
		case 1:
			throw eServerConfigError;
			break;
		case 2:
			throw eTermConnectionExsits;
			break;
		case 3:
			throw eSolIPAddressNotDefined;
		default:
			throw eUnknown;
			break;
		}
	}

	const char* ipaddr1 = rsp->getIpAddr();
	uint32_t len1 = rsp->getIpAddrLen();

	cout << "ip addr len1: " << len1 << endl;
	cout << "ip adrr1: " << ipaddr1 << endl;

	const char* ipaddr2 = rsp->getIpAddr(1);
	uint32_t len2 = rsp->getIpAddrLen(1);

	cout << "ip addr len2: " << len2 << endl;
	if (len2 <= 16)
		cout << "ip adrr2: " << ipaddr2 << endl;

	m_ipAddress[0] = ipaddr1;
	m_ipAddress[1] = ipaddr2;
	m_echo.setIpAddr(ipaddr1, ipaddr2);
}

void SolConnect::ipCheck()
{
	//res[0] = m_ipError[0] == -255 ? m_echo.ping(0, ECHO_REPEAT) : m_ipError[0];
	//res[1] = m_ipError[1] == -255 ? m_echo.ping(1, ECHO_REPEAT) : m_ipError[1];
	//m_ipError[0] = res[0];
	//m_ipError[1] = res[1];

	m_ipError[0] = m_echo.ping(0, ECHO_REPEAT);
	m_ipError[1] = m_echo.ping(1, ECHO_REPEAT);

	return;
}

// Return
// 100 	exit
// 1	both subnets are down
// 2	Sol cannot be conatacted via both network
//
int SolConnect::connect(int index)
{
	int result = -255;

	m_ipError[0] = m_ipError[1] = -255;
	m_solError[0] = m_solError[1] = -255;
	int count = 0;
	int connCount = 0;
	do
	{
		TRACE(("SolConnect::connect() loop for index <%d>", index));
		this->ipCheck();
		if (m_ipError[0] != 0 && m_ipError[1] != 0)
		{
			result = 1;
			break;
		}

		TRACE(("SolConnect::connect() create sol <%s>", m_ipAddress[index].c_str()));

		m_terminal->createSol(m_ipAddress[index]);
		m_terminal->join();

		m_solError[index] = m_terminal->getStatus();
		cout << "sol status is: " << m_solError[index] << endl;
		cout << "sol cmd exit is: " << m_terminal->isCmdExit() << endl;

		if (m_terminal->isCmdExit())
		{
			result = 100;
			break;
		}

		/*
		connCount = m_terminal->getConnCount();
		if (connCount)
		{
			TRACE(("SolConnect::connect() connCount <%d>", connCount));
			m_solError[index] = 0;
		}
		*/
		m_solError[index] = m_solError[index] == SolSession::INIT_ERROR ? 0 : m_solError[index];

		// Sol connection is not possible on both 169 and 170 network
		if (m_solError[0] == SolSession::INIT_ERROR && m_solError[1] == SolSession::INIT_ERROR)
		{
			result = 2;
			break;
		}
		index ^= 1;
		++count;

		/*
		m_terminal.reset(new SysconClient(m_cpId, m_side, m_multiCpSystem));
		if (!m_terminal->connect())
			throw eServerUnreachable;
		*/
		this->createTerminal();

	}
	while (count < 2);

	return result;
}

void SolConnect::createTerminal()
{
	m_terminal.reset(new SysconClient(m_cpId, m_side, m_multiCpSystem));
	if (!m_terminal->connect())
		throw eServerUnreachable;

}

//void execute(int cpId = 0xffff, int side = 0, int multiCpSystem = 0);
//void logParam(int cpId = 0xffff, int side = 0, int multiCpSystem = 0);

void execute(int cpId, int side, int multiCpSystem, int);

void listSol(int cpId, int side, int multiCpSystem, int log);

void getDmx(int cpId, int side, int multiCpSystem);

int help();
void parseCmdLine(int argc, char **argv, string& cpName, int& cpId, int& cpSide, int& logOpt, int& action);

void typedef (*Cmd)(int, int, int, int);


int main(int argc, char** argv)
{
	AppTrace::initialise(s_traceName);

	TRACE(("cmd_syscon_main; main() starts as console application"));
	//Cmd cmds[4] = {execute, logParam, listSol, removeSol};
	Cmd cmds[2] = {execute, listSol};

	int exitCode = 0;
	int cpId = 0xffff;
	int logOpt = 0;
	int cpSide = -1;
	int action = -1;
	string cpName;
	try
	{
		s_multiCpSystem = SysFunx::isMultipleCP();
		if (s_multiCpSystem == -1)
			throw eCSUnreachable;

		parseCmdLine(argc, argv, cpName, cpId, cpSide, logOpt, action);
		if (action > 3)
		{
			cout << "Function not yet supported" << endl;
			return -1;
		}

		//execute(cpId, cpSide, s_multiCpSystem, 0);
		cmds[action](cpId, cpSide, s_multiCpSystem, logOpt);

	}
	catch (Cmd_Error x)
	{
		switch (x)
		{
		case eSyntaxError:
		case eIllegalOption:
		case eMandatoryOption:
		case eSideMandatoryOption:
			help();
			break;

		default:
			cout << cmd_error_msg[x] << endl;
			break;
		}

		exitCode = x;
	}
	catch (InvalidValueT& e)
	{
		Cmd_Error invalid;
		string forOption;
		string name;
		int optChar;
		boost::tie(invalid, name, forOption, optChar) = e;
		cout << cmd_error_msg[invalid] << " <" << name << "> "
			 << forOption << " <" << char(optChar) << ">" << endl;
		exitCode = invalid;
	}
	catch(...)
	{
		exitCode = eUnknown;
		cout << cmd_error_msg[(Cmd_Error)exitCode] << " at catch all in main" << endl;
	}

	AppTrace::finalise();
	return exitCode;

}


int help()
{
	static string cmdline = (s_multiCpSystem == 1)?
			"syscon -a -cp <cp_name> [-s <side>]"
			"\n-l [ -cp <cp_name> [-s <side>]]"
			:
			"syscon -a [-s <side>]"
			"\n-l [-s <side>]";

	cout << "Usage:" << endl;
	cout << endl;

	cout << cmdline << endl;
	cout << endl;
	cout << "<cp_name>:   CP1, CP2, BC0, BC1, ..., BC63" << endl;
	cout << "<side>:      CPA, CPB" << endl;
	cout << endl;


	return 0;
}

void parseCmdLine(int argc, char **argv, string& cpName, int& cpId, int& cpSide, int& logOpt, int& action)
{
	cpName = "";
	cpSide = -1;
	logOpt = -1;
	cpId = 0xffff;
	action = -1;

	opterr = 0;
	int c = 0;
	bool helpOpt = false;
	try
	{
		string nameStr = "";
		string sideStr = "";
		//string logStr = "";
		int side = -1;
		char* sstr = 0;
		//int logOptTemp = logOpt;
		//int logOptVal = 0;
		int actOptTemp = action;
		int lsOptTemp = action;
		//int rmOptTemp = action;
		while ((c = getopt(argc, argv,"ac:s:hd:lr")) != -1)
		{
			switch (c)
			{
			case 'a':
			{
				if (actOptTemp != -1 || lsOptTemp != -1)
					throw eSyntaxError;

				actOptTemp = 0;
				break;
			}
			case 'c':
			{
#if 0
				if (strcmp(argv[optind - 1], "-cp") != 0 || optind == argc)
					throw eSyntaxError;

				if (!s_multiCpSystem)
					throw eIllegalOption;

				nameStr = argv[optind];
				++optind;
#endif
				string argvcp = argv[optind - 1];
				if (argvcp == "-cp")
				{
					if (optind == argc)
						throw eSyntaxError;

					nameStr  = argv[optind];
					++optind;

				}
				else
				{
					size_t pos = argvcp.find("-cp");
					if (pos == string::npos)
						throw eSyntaxError;

					nameStr = argvcp.substr(3);
				}

				if (!s_multiCpSystem)
					throw eIllegalOption;

				transform(nameStr.begin(), nameStr.end(), nameStr.begin(), (int (*)(int)) ::toupper);

				break;
			}
			case 's':
				sideStr = sstr = optarg;
				transform(sideStr.begin(), sideStr.end(), sideStr.begin(), (int (*)(int)) ::tolower);

				if (sideStr != "cpa" && sideStr != "cpb" && sideStr != "a" && sideStr != "b")
					throw InvalidValueT(eInvalidValue, string(optarg), "for option", 's');;

				side = (sideStr == "cpa" || sideStr == "a") ? 0 : 1;
				break;

#if 0

			case 'd':
				if (actOptTemp != -1 || logOptTemp != -1 || lsOptTemp != -1 || rmOptTemp != -1)
					throw eSyntaxError;

				logStr = optarg;
				transform(logStr.begin(), logStr.end(), logStr.begin(), (int (*)(int)) ::tolower);

				if (logStr != "on" && logStr != "off")
					throw InvalidValueT(eInvalidValue, string(optarg), "for option", 'd');;

				logOptVal = logStr == "on" ? 1 : 0;
				logOptTemp = 0;
				break;

#endif

			case 'l':
				if (actOptTemp != -1 || lsOptTemp != -1)
					throw eSyntaxError;
				lsOptTemp = 0;
				break;

#if 0

			case 'r':
				if (actOptTemp != -1 || logOptTemp != -1 || lsOptTemp != -1 || rmOptTemp != -1)
					throw eSyntaxError;
				rmOptTemp = 0;

				break;

#endif

			case 'h':
				helpOpt = true;
				break;

			default:
				throw eSyntaxError;
			}
		}

        // Have redundant argument
		if (optind < argc)
             throw eSyntaxError;

		if (actOptTemp == -1 && lsOptTemp == -1)
			throw eSyntaxError;

		// If multiple system option,"cp" is mandatory
        if ((s_multiCpSystem == 1) && nameStr.empty() && lsOptTemp == -1)
            throw eMandatoryOption;

        int cpIdTemp = -1;
        if (s_multiCpSystem == 1)
        {
        	if (nameStr.size())
        	{
        		cpIdTemp = SysFunx::cpNameToCpId(nameStr.c_str());
        		if (cpIdTemp == -1)
        			throw eCpNameNotDefined;
        	}
        }
        else
        {
        	cpIdTemp = 0xffff;
        }

        // -s option is mandatory
        if ((cpIdTemp == 1001 || cpIdTemp == 1002) && sideStr.empty())
        	throw eSideMandatoryOption;

		cpName = nameStr;
		cpSide = side;
		cpId = cpIdTemp;

		if (actOptTemp == 0)
		{
			action = 0;
		}
		//else if (logOptTemp == 0)
		//{
		//	action = 1;
		//	logOpt = logOptVal;
		//}
		else if (lsOptTemp == 0)
		{
			//action = 2;
			action = 1;
			logOpt = cpName.length()? 2: 3;
		}
		//else if (rmOptTemp == 0)
		//{
		//	action = 3;
		//}
	}
	catch (Cmd_Error x)
	{
		switch (x)
		{
		case eSyntaxError:
		case eMandatoryOption:
		case eIllegalOption:
		case eSideMandatoryOption:
			cout << cmd_error_msg[x] << endl;
			// Fall through
		default:
			throw;
		}
	}
	catch (...)
	{
		throw;
	}
}

void execute(int cpId, int side, int multiCpSystem, int)
{
	//cout << "Stub syscon execution" << endl;
	//cout << "cpId:\t" << cpId << endl;
	//cout << "side:\t" << side << endl;
	//cout << "multiCpSystem:\t" << multiCpSystem << endl;

	SolConnect sol(cpId, side, multiCpSystem);
	sol.init();
	int ipStatus[2];
	ipStatus[0] = ipStatus[1] = -1;
	//sol.ipCheck(ipStatus);
	string ipaddr1 = sol.getIpAddr(0);
	string ipaddr2 = sol.getIpAddr(1);
	cout << "Status of " << ipaddr1 << " " << ipStatus[0] << endl;
	cout << "Status of " << ipaddr2 << " " << ipStatus[1] << endl;

	int index = 0;
	int result;

	while (true)
	{
		// Check IP Status on both subnets
		result = sol.connect(index);
		cout << "result is " << result << endl;
		if (result == 100)
		{
			// Command exit
			break;
		}

		if (result == 1)
			throw eBothNetDown;

		if (result == 2)
			throw eBothSolServerDown;

		sol.createTerminal();
	}

#if 0

	int ipStatus[2];
	SolConnect sol(ipaddr1, ipaddr2);
	sol.ipCheck(ipStatus);

	cout << "Status of " << ipaddr1 << " " << ipStatus[0] << endl;
	cout << "Status of " << ipaddr2 << " " << ipStatus[1] << endl;

	int index = 0;
	bool done = false;
	while (!done)
	{
		TRACE(("Start of while loop, ping both networks"));

		// Check IP Status on both subnets
		if (ipStatus[index] > 0 && ipStatus[index ^ 1] > 0)
		{
			throw eBothNetDown;
		}

		if (ipStatus[index] == 0)
		{
			// SOL connect to subnet referred to by index
			sol.connect(index, terminal.get());
			cout << endl;
			cout << "sol status is: " << terminal->getStatus() << endl;
			cout << "sol cmd exit is: " << terminal->isCmdExit() << endl;

			if (terminal->isCmdExit())
			{
				break;
			}

		}
		else
		{

		}

		// Remove the following later
		break;
	}

#endif

}


// perCp == 2; print for cp specified
//
void listSol(int cpId, int side, int multiCpSystem, int perCp)
{
	namespace bip = boost::asio::ip;
	typedef ACS_CS_API_CommonBasedArchitecture Acs_ArchT;
	typedef Acs_ArchT::ArchitectureValue ArchValueT;


	if (cpId < 64 && cpId >=0)
	{
		side = 0;
	}


	cout << "syscon execution simulation, connect" << endl;
	cout << "cpId:\t" << cpId << endl;
	cout << "side:\t" << side << endl;
	cout << "multiCpSystem:\t" << multiCpSystem << endl;
	cout << "perCp:\t" << perCp << endl;
	cout << endl;

	cout << "\nFuntion is under developed\n" << endl;


	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue infra;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);

	if (res != ACS_CS_API_NS::Result_Success)
	{
		cout << "Cannot get node architecture" << endl;
		return;
	}

	cout << "Node architecture is " << infra << endl;

	switch (infra)
	{
	case GepInfo::SCB:
		break;
	case GepInfo::SCX:
		cout << "This is SCX node" << endl;
		//testScx(sysid, side);
		break;
	case GepInfo::DMX:
		cout << "This is DMX node" << endl;
		getDmx(cpId, side, multiCpSystem);
		break;
	default:
		break;

	}

}

void getDmx(int sysId, int side, int)
{
	DmxGepInfo aGep;
	int gepNo = aGep.getGepVersion(sysId, side);
	cout << "Gep number is " << gepNo << endl;
}

