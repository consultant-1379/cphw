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
//#include "boost/regex.hpp"
//#include <cstring>

//#include <vector>
#include "CmdError.h"
#include "SysFunx.h"
#include "AppTrace.h"

//#include "ACS_CS_API.h"
#include "acs_prc_api.h"

#include "SolConnect.h"
#include "GepFunx.h"

using namespace std;

enum {SCB = 0, SCX = 1, DMX = 2, VIRTUALIZED = 3};

static const char* s_traceName = "SYSCON";

typedef boost::tuple<Cmd_Error, string, string, int> InvalidValueT;

extern int opterr;
static int s_multiCpSystem = -1;


void execute(int cpId, int side, int multiCpSystem, int);
void listSol(int cpId, int side, int multiCpSystem, int log);
void resetTty();

int help();
void parseCmdLine(int argc, char **argv, string& cpName, int& cpId, int& cpSide, int& logOpt, int& action);

void typedef (*Cmd)(int, int, int, int);


int main(int argc, char** argv)
{
	AppTrace::initialise(s_traceName);

	TRACE(("syscon; main() starts"));

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
		// Check for system configuration
		// syscon does not run on Virtual
		int infra = SysFunx::getNodeArchitecture();
		if (infra == VIRTUALIZED)
			throw eIllcmdInSystemConfig;

#if 1

		if (argc == 2 && string(argv[1]) == "9999")

#else

		char const* pEnv = getenv("SYSCON_COMPLETE");
		if (pEnv && string(pEnv) == "88")

#endif

		{
			resetTty();
			return 0;
		}

	    ACS_PRC_API prcApi;
	    // return code from ACS_PRC_API::askForNodeState()
	    // -1 Error detected
	    // 1  Active
	    // 2  Passive
	    if (prcApi.askForNodeState() != 1)
	    {
	        throw eServerUnreachable;
	    }

		s_multiCpSystem = SysFunx::isMultipleCP();
		if (s_multiCpSystem == -1)
			throw eCSUnreachable;

		parseCmdLine(argc, argv, cpName, cpId, cpSide, logOpt, action);

		if (action == 255)
		{
			//this is help
			return 0;
		}

		//execute(cpId, cpSide, s_multiCpSystem, 0);
		cmds[action](cpId, cpSide, s_multiCpSystem, logOpt);

	}
	catch (Cmd_Error x)
	{
		switch (x)
		{

		case eSyntaxError:
		//case eIllegalOption:
		//case eMandatoryOption:
		//case eSideMandatoryOption:
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

int help_ncd()
{
	cout << "Usage:" << endl;
	cout << endl;

	cout << "syscon -a -s side" << endl;
	cout << "syscon -h" << endl;
	cout << endl;

	cout << "side:     A or CPA, B or CPB" << endl;
	cout << endl;
	return 0;
}

int help_mcp()
{
	cout << "Usage:" << endl;
	cout << endl;

	cout << "syscon -a -cp cpname [-s side]" << endl;
	cout << "syscon -h" << endl;
	cout << endl;
	cout << "cpname:   CP1, CP2, BC0, BC1, ..., BC63" << endl;
	cout << "side:     A or CPA, B or CPB" << endl;
	cout << endl;

	return 0;
}

int help()
{
	return s_multiCpSystem == 1 ? help_mcp() : help_ncd();
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

	try
	{
		string nameStr = "";
		string sideStr = "";
		int side = -1;
		char* sstr = 0;
		int actOptTemp = action;
		int lsOptTemp = action;

		while ((c = getopt(argc, argv,"ac:s:h")) != -1)
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

			case 'l':
				if (actOptTemp != -1 || lsOptTemp != -1)
					throw eSyntaxError;
				lsOptTemp = 0;
				break;

			case 'h':
				help();
				action = 255;
				return;

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
        if ((cpIdTemp == 0xffff || cpIdTemp == 1001 || cpIdTemp == 1002) && sideStr.empty())
        	throw eSideMandatoryOption;

		cpName = nameStr;
		cpSide = side;
		cpId = cpIdTemp;

		if (actOptTemp == 0)
		{
			action = 0;
		}
		else if (lsOptTemp == 0)
		{
			//action = 2;
			action = 1;
			logOpt = cpName.length()? 2: 3;
		}
	}
	catch (Cmd_Error x)
	{
		switch (x)
		{
		case eSyntaxError:
		//case eMandatoryOption:
		//case eIllegalOption:
		//case eSideMandatoryOption:
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
	TRACE(("execute()"));

	// Adjusting side information
	if (cpId < 64 && cpId >=0)
	{
		side = 0;
	}

	if (!multiCpSystem)
	{
		cpId = 1001;
	}

	// Attempt to get GEP version
	int gepVer;
	cout << "Query GEP Version, please wait ..." << endl;

	int res = GepFunx::getGepVersion(gepVer, cpId, side);
	TRACE(("Return from GepFunx::getGepVersion() res <%d> gepVer <%d>", res, gepVer));
	if (gepVer < 0)

	{
		throw eFailedToGetGepVersion;
	}

	if (gepVer < 5)
	{
		throw eGepVersionNotSupported;
	}

	cout << "Enter ^] to exit" << endl;

	int index = 0;
	int result;
	string ipaddr[2];
	bool done = false;
	SolConnect sol(cpId, side, multiCpSystem);
	sol.init();
	while (!done)
	{
		ipaddr[0] = sol.getIpAddr(0);
		ipaddr[1] = sol.getIpAddr(1);

		TRACE(("SolSession at index <%d> <%s>", index, ipaddr[index].c_str()));
		result = sol.connect(index);
		TRACE(("execute() sol.connect() returns result <%d>", result));
		switch (result)
		{
		case 100:
			// Command exit
			done = true;
			break;

		case 1:
			throw eBothNetDown;
			break;

		case 2:
			//throw eSolServerNotContactable;
			break;

		case 3:
			//throw eLostContactToSolServer;
			break;

		default:;
		}

		usleep(35000);
		//sleep(62);
	}

	TRACE(("execute( returns)"));
}

// perCp == 2; print for cp specified
//
void listSol(int cpId, int side, int multiCpSystem, int perCp)
{

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
	cout << "perCp:\t" << perCp << endl;
	cout << endl;

	cout << "\nFuntion is under developed\n" << endl;

}

void resetTty()
{

	Keyboard keyboard;
	int savedErrno;
	keyboard.ttyRaw(savedErrno);
	keyboard.ttyReset(savedErrno);
}
