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



using namespace std;

static const char* s_traceName = "SYSCON";

typedef boost::tuple<Cmd_Error, string, string, int> InvalidValueT;

extern int opterr;
static int s_multiCpSystem = -1;

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

	// Adjusting side information
	if (cpId < 64 && cpId >=0)
	{
		side = 0;
	}

	if (!multiCpSystem)
	{
		cpId = 1001;
	}

	cout << "syscon execution simulation, connect" << endl;
	cout << "cpId:\t" << cpId << endl;
	cout << "side:\t" << side << endl;
	cout << "multiCpSystem:\t" << multiCpSystem << endl;
	//return;

	boost::scoped_ptr<SysconClient> terminal(new SysconClient(cpId, side, multiCpSystem));

	//terminal.reset(new SysconClient(cpId, side, multiCpSystem));
	if (!terminal->connect())
		throw eServerUnreachable;

	cout << "syscon sends Sco_ConnectMsg" << endl;

	ScoMsg msg;
	new (msg.addr()) ScoMsg::Sco_ConnectMsg(cpId, side, multiCpSystem, 5678, 1, 0);
	size_t res = terminal->send_blocking(msg.addr(), 2048);

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

	string ipAddress[2];
	ipAddress[0] = string(ipaddr1, len1);
	ipAddress[1] = string(ipaddr2, len2);


#if 1

	string ipNet1 = ipAddress[0];
	string ipNet2 = ipAddress[1];

#else

	namespace bip = boost::asio::ip;
	bip::address_v4 ipa = bip::address_v4::from_string(ipaddr1);
	uint32_t ipan = ipa.to_ulong();
	ipan &= 0xffffff00;
	ipan |= 1;

	boost::system::error_code ec;
	string ipNet1 = bip::address_v4(ipan).to_string(ec);

	ipa = bip::address_v4::from_string(ipaddr2);
	ipan = ipa.to_ulong();
	ipan &= 0xffffff00;
	ipan |= 1;

	string ipNet2 = bip::address_v4(ipan).to_string(ec);

#endif

	cout << "Server of Ip1 is " << ipNet1 << endl;
	cout << "Server of Ip2 is " << ipNet2 << endl;

	ScoPinger echo(ipNet1, ipNet2);
	if (echo.ping(10) != 0)
	{
		throw eBothNetDown;
	}
	int replyCount = echo.replyCounter();
	if (replyCount != 10 && replyCount < 9)
	{
		throw eNetworkUnstable;
	}
	int index = echo.index();
	cout << "Ping is ok on interface <" << index << ">" << endl;

	// Get a SOL Session and start it here
	terminal->createSol(ipAddress[index]);
	terminal->join();

	cout << endl;
	cout << "sol status is: " << terminal->getStatus() << endl;
	cout << "sol cmd exit is: " << terminal->isCmdExit() << endl;

	if (!terminal->isCmdExit())
	{
		cout << "Not possible to connect to " << ipAddress[index] << endl;
		if (index == 0)
		{
			++index;

			terminal.reset(new SysconClient(cpId, side, multiCpSystem));
			if (!terminal->connect())
				throw eServerUnreachable;

			cout << "attempt to connect to " << ipAddress[index] << endl;
			terminal->createSol(ipAddress[index]);
			terminal->join();

			cout << "sol return from a recovery measure" << endl;
			cout << "sol status is: " << terminal->getStatus() << endl;
			cout << "sol cmd exit is: " << terminal->isCmdExit() << endl;
		}
	}

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

