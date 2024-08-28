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

#include <cstring>
#include <fstream>

#include "CmdError.h"
#include "SysFunx.h"
#include "AppTrace.h"
#include "SysconClient.h"
#include "ScoMsg.h"
//#include "SolSession.h"

using namespace std;

static const char* s_traceName = "SYSLOGLS";

typedef boost::tuple<Cmd_Error, string, string, int> InvalidValueT;

extern int opterr;
static int s_multiCpSystem = -1;

void execute(int cpId = 0xffff, int side = 0, int multiCpSystem = 0);
int help();
void parseCmdLine(int argc, char **argv, string& cpName, int& cpId, int& cpSide, int& logOpt);

int main(int argc, char** argv)
{
	AppTrace::initialise(s_traceName);

	TRACE(("cmd_syscon_main; main() starts as console application"));

	int exitCode = 0;
	int cpId = 0xffff;
	int logOpt = 0;
	int cpSide = -1;
	string cpName;
	try
	{
		s_multiCpSystem = SysFunx::isMultipleCP();
		if (s_multiCpSystem == -1)
			throw eCSUnreachable;

		parseCmdLine(argc, argv, cpName, cpId, cpSide, logOpt);
		execute(cpId, cpSide, s_multiCpSystem);

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
			"syscon -cp <cp_name> -s <side> [-l ]  " :
			"syscon -s <side> [-l ]  ";

	cout << "Usage:" << endl;
	cout << endl;

	cout << cmdline << endl;
	cout << endl;
	cout << "<cp_name>:   CP1, CP2, BC0, BC1, ..., BC63" << endl;
	cout << "<side>:      CPA, CPB" << endl;

	cout << endl;
	cout << "-l syscon log creation for Cluster CP" << endl;

	return 0;
}

void parseCmdLine(int argc, char **argv, string& cpName, int& cpId, int& cpSide, int& logOpt)
{
	cpName = "";
	cpSide = -1;
	logOpt = 0;
	cpId = 0xffff;

	opterr = 0;
	int c = 0;
	bool helpOpt = false;
	try
	{
		string nameStr = "";
		string sideStr = "";
		int side = -1;
		char* sstr = 0;
		int logOptTemp = logOpt;
		while ((c = getopt(argc, argv,"c:s:hl")) != -1)
		{
			switch (c)
			{
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

				if (sideStr != "cpa" && sideStr != "cpb")
					throw InvalidValueT(eInvalidValue, string(optarg), "for option", 's');;

				side = sideStr == "cpa" ? 0 : 1;
				break;

			case 'l':
				logOptTemp = 1;
				break;

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

        // If multiple system option,"cp" is mandatory
        if ((s_multiCpSystem == 1) && nameStr.empty())
            throw eMandatoryOption;

        int cpIdTemp = -1;
        if (s_multiCpSystem == 1)
        {
        	cpIdTemp = SysFunx::cpNameToCpId(nameStr.c_str());
        	if (cpIdTemp == -1)
        		throw eCpNameNotDefined;
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
		logOpt = logOptTemp;
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

void execute(int cpId, int side, int multiCpSystem)
{

	SysconClient terminal(cpId, side, multiCpSystem);
	if (!terminal.connect())
		throw eServerUnreachable;

#if 0
	ScoMsg msg;
	new (msg.addr()) ScoMsg::Sco_ConnectMsg(cpId, side, multiCpSystem, 5678, 1, 0);
	size_t res = terminal.send_blocking(msg.addr(), 2048);

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

#endif

	string name = SysFunx::getLogPath(cpId, side, multiCpSystem);
	ifstream file(name.c_str());

	if (!file.good())
		throw eLogFileDoesNotExist;

	char line[1024];
	while (file.getline(line, 1024))
	{
		cout << line << endl;
	}
	file.close();
}
