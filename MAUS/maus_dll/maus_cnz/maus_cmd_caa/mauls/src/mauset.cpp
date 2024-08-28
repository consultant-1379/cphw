/*
 * mauls.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: xdtthng
 */


#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>

#include "boost/assign/list_of.hpp"
#include "boost/tuple/tuple.hpp"

#include "maus_trace.h"
#include "EndpointView.h"
#include "MAUFunx.h"
#include "Exception.h"
#include "CmdErrMsg.h"
#include "Text_Code.h"
#include "CmdCode.h"
#include "CmdClient.h"
#include "AtomicFlag.h"

using namespace std;

extern BoolFlagT s_abortChecksum;
extern BoolFlagT s_applicationAborting;


void execute(int cpId = 0xffff);
void parseCommandLine(int acrg, char** argv);

extern int opterr;

static struct option long_options[] =
{
	{"cp",    	required_argument, 	0, 	'C'},
	{"name",   required_argument, 	0, 	'N'},
	{0, 		0, 					0, 	0}
};

static bool s_multiCpSystem;
static bool s_upgrade;
static string s_cpname;
static unsigned int s_epMask;
static unsigned int s_rOpt;
static unsigned int s_zOpt;
static string s_serviceName;

BoolFlagT s_abortChecksum(false);
BoolFlagT s_applicationAborting(false);

typedef boost::tuple<Cmd_Error_Code, string, Text_Code, int> InvalidValue;
typedef pair<string, int> CmdResultError;

EpTableT epTable = boost::assign::map_list_of
	("ap-a", apa)
	("apa", apa)
	("ap-b", apb)
	("apb", apb)
	("cps", cps)
	("cp-s", cps);


int main(int argc, char** argv)
{


	int exitCode = 0;
	int cpId = 0xffff;
	try {
		s_multiCpSystem = MAUFunx::isMultipleCP();
		parseCommandLine(argc, argv);
		if (s_serviceName.empty()) {
			if (s_multiCpSystem) {
				transform(s_cpname.begin(), s_cpname.end(), s_cpname.begin(), (int (*)(int)) ::toupper);

				if (s_cpname == "CLUSTER")
					throw eCp1orCp2Supported;

				cpId = MAUFunx::cpNameToCpId(s_cpname.c_str());

				if (cpId != 1001 && cpId != 1002)
					throw eCp1orCp2Supported;

			}
		}
		else {
			cpId = s_serviceName == text_string[cMaus1CompName]? 1001 : 1002;
		}

		execute(cpId);

	}
	catch (Cmd_Error_Code x)
	{
		switch (x)
		{
		case eSyntaxError:
		case eIllegalOption:
		case eMandatoryOption:
			if (s_multiCpSystem)
			{
				cout << "Usage: mauset -cp cpName" << endl;
			}
			else
			{
				cout << "Usage: mauset" << endl;
			}
			break;

		default:
			cout << cmd_error_msg[x] << endl;
			break;
		}

		exitCode = x;
	}
	catch (CodeException& x)
	{
		//cout << "Code exception at main with " << x.errcode() << endl;
		switch (x.errcode())
		{
		case CodeException::CE_CSUNREACHABLE:
			exitCode = eCSUnreachable;
			cout << cmd_error_msg[(Cmd_Error_Code)exitCode] << endl;
			break;
		case CodeException::CE_CS_CPNAME_UNDEFINED:
			exitCode = eCpNameNotDefined;
			cout << cmd_error_msg[(Cmd_Error_Code)exitCode] << endl;
			break;

		default:
			exitCode = eUnknown;
			cout << cmd_error_msg[(Cmd_Error_Code)exitCode] << endl;
			break;

		}
	}
	catch (InvalidValue& e)
	{
		Cmd_Error_Code invalid;
		Text_Code forOption;
		string name;
		int optChar;
		boost::tie(invalid, name, forOption, optChar) = e;
		cout << cmd_error_msg[invalid] << " <" << name << "> "
			 << text_string[forOption] << " <" << char(optChar) << ">" << endl;
		exitCode = invalid;
	}
	catch(...)
	{
		exitCode = eUnknown;
		cout << cmd_error_msg[(Cmd_Error_Code)exitCode] << " at catch all in main" << endl;
	}

	// For Rocco requrements
	return s_upgrade? 0 : exitCode;
}

void parseCommandLine(int argc, char** argv)
{
	int c;
	opterr = 0;
	char* cstr = 0;
	char* mstr = 0;
	char* nStr = 0;
	string epstr;
	EpTableIter epIter;
	bool done = false;
	unsigned int epMask = 0;
	try
	{
		while ((c = getopt_long (argc, argv, "c:m:n:rzu", long_options, 0)) != -1)
		{
			switch (c)
			{
			case 'c':

				if (strcmp(argv[optind - 1], "-cp") != 0 || optind == argc || cstr || mstr || s_rOpt || s_zOpt)
					throw eSyntaxError;

				if (!s_multiCpSystem)
					throw eIllegalOption;

				s_cpname = cstr = argv[optind];
				++optind;
				break;

			case 'C':
				if (cstr)
					throw eSyntaxError;

				if (!s_multiCpSystem)
					throw eIllegalOption;

				s_cpname = cstr = optarg;
				break;

			case 'n':
			case 'N':
				if (cstr || nStr)
					throw eSyntaxError;

				s_serviceName = optarg;
				transform(s_serviceName.begin(), s_serviceName.end(),
						s_serviceName.begin(), (int (*)(int)) ::toupper);

				if (s_serviceName != text_string[cMaus1CompName] && s_serviceName != text_string[cMaus2CompName])
					throw InvalidValue(eInvalidValue, string(optarg), cForOption, 'n');
				break;

			case 'r':
				if (s_rOpt || mstr || s_zOpt)
					throw eSyntaxError;

				s_rOpt = 1;
				break;

			case 'z':
				if (s_rOpt || mstr || s_zOpt)
					throw eSyntaxError;

				s_zOpt = 1;
				break;

			case 'm':
				if (s_rOpt || mstr || s_zOpt)
					throw eSyntaxError;

				epstr = optarg;
				mstr = optarg;
				do
				{
					transform(epstr.begin(), epstr.end(), epstr.begin(), (int (*)(int)) ::tolower);
					epIter = epTable.find(epstr);
					if (epIter == epTable.end())
						throw InvalidValue(eInvalidValue, string(mstr), cForOption, 'm');

					epMask |= epIter->second;
					done = (optind >= argc)  || (argv[optind][0] == '-');
					if (!done) {
						mstr = argv[optind++];
						epstr = mstr;
					}
				}
				while (!done);

				s_epMask = epMask;
				break;

			case 'u':

				s_upgrade = true;
				break;

			default:
				throw eSyntaxError;
				break;

			}
		}

        // Have redundant argument
		if (optind < argc)
             throw eSyntaxError;

        // If multiple system option,"cp" is mandatory
        if (s_multiCpSystem && s_cpname.empty() && s_serviceName.empty())
            throw eMandatoryOption;

	}
	catch (Cmd_Error_Code x)
	{
		switch (x)
		{
		case eSyntaxError:
		case eMandatoryOption:
		case eIllegalOption:
			cout << cmd_error_msg[x] << endl;
			throw;
		default:
			throw;
		}
	}
	catch (...)
	{
		throw;
	}

}

void execute(int cpId)
{
	///////////////////////////////////////////////

	int instance = cpId == 0xffff? 1 : cpId - 1000;

	CmdClient cmd(instance);
	CmdClient::QuorumInfo qd;
	Cmd_Error_Code res = eUnknown;

	// Read current mask only
	if (s_rOpt)
	{
		qd.activeMask = -1;
		res = cmd.getQuorumInfo(qd);

		// Cannot read active mask
		if (res != 0)
		{
			throw res;
		}

		if (qd.activeMask == -1)
		{
			throw eFailToGetEpMask;
		}
		cout << "Mask read from maus is " << qd.activeMask << endl;
		return;
	}

	// Either set or clear mask with s_epMask || s_zOpt is true
	res = cmd.setActiveMask(s_epMask);

	//cout << "return code is " << res << endl;
	if (res != 0)
	{
		throw res;
	}

	if (qd.activeMask == -1)
	{
		throw eFailToGetEpMask;
	}

}


