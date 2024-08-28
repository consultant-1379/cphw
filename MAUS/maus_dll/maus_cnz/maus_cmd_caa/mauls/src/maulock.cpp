/*
 * maulock.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: xdtthng
 */

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>

#include "MAUFunx.h"
#include "Exception.h"
#include "CmdErrMsg.h"
#include "CmdFunx.h"
#include "Text_Code.h"
#include "AtomicFlag.h"

using namespace std;

extern BoolFlagT s_abortChecksum;
extern BoolFlagT s_applicationAborting;


void lock_maus(const string& sname, int si);
void lock_maus(const string& sname);
void parseCommandLine(int acrg, char** argv);

extern int opterr;

static struct option long_options[] =
{
	{"cp",    	required_argument, 	0, 	'C'},
	{"force",   0, 	0, 					'F'},
	{"name",   required_argument, 	0, 	'N'},
	{0, 		0, 					0, 	0}
};

typedef pair<string, Cmd_Error_Code> InvalidSi;
typedef boost::tuple<Cmd_Error_Code, string, Text_Code, int> InvalidValue;


static bool s_multiCpSystem;
static bool s_force;
static string s_cpname;
static int	s_si;
static string s_serviceName;

BoolFlagT s_abortChecksum(false);
BoolFlagT s_applicationAborting(false);


int main(int argc, char** argv)
{

	int exitCode = 0;
	try {
		s_multiCpSystem = MAUFunx::isMultipleCP();
		parseCommandLine(argc, argv);

		// Locking service using cp name
		int cpId = 0xffff;
		string sname;
		if (s_serviceName.empty()) {
			if (s_multiCpSystem) {
				if (s_cpname == "CLUSTER")
					throw eCp1orCp2Supported;

				cpId = MAUFunx::cpNameToCpId(s_cpname.c_str());

				if (cpId != 1001 && cpId != 1002)
					throw eCp1orCp2Supported;

				sname = cpId == 1002 ? text_string[cMaus2CompName] : text_string[cMaus1CompName];
			}
			else {
				sname = text_string[cMaus1CompName];
			}
		}
		else {	// Locking service using service name
			sname = s_serviceName;
		}

		// All parsing errors are found at this point in time
		// Present a dialog if necessary
		string prompt = "Really want to lock ";
		prompt += sname;
		prompt += " service?";
		if (!s_force && !CmdFunx::confirm(prompt))
			throw eUserAbort;

		if (s_si)
			lock_maus(sname, s_si);
		else
			lock_maus(sname);

	}
	catch (Cmd_Error_Code x)
	{
		switch (x)
		{
		case eSyntaxError:
		case eIllegalOption:
		case eMandatoryOption:
			if (s_multiCpSystem)
				cout << "Usage: maulock -cp cpName [-f]" << endl;
			else
				cout << "Usage: maulock [-f]" << endl;
			cout << "Or:    maulock -n sname [-f]" << endl;
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
	catch (InvalidSi& e)
	{
		cout << cmd_error_msg[e.second] << " " << e.first << endl;
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

	}

	return exitCode;
}

void parseCommandLine(int argc, char** argv)
{
	static const string s_releaseOptions = "c:fn:";
	string cmdOpt = s_releaseOptions;

	if (MAUFunx::isSiOptionUsed())
		cmdOpt += "s:";

	int c;
	opterr = 0;
	char* cstr = 0;
	char* siStr = 0;
	char* nStr = 0;
	try
	{
		while ((c = getopt_long (argc, argv, cmdOpt.c_str(), long_options, 0)) != -1)
		{
			switch (c)
			{
			case 'c':

				if (strcmp(argv[optind - 1], "-cp") != 0 || optind == argc ||
						cstr || siStr || s_force || nStr)
					throw eSyntaxError;

				if (!s_multiCpSystem)
					throw eIllegalOption;

				s_cpname = cstr = argv[optind];
				++optind;
				transform(s_cpname.begin(), s_cpname.end(), s_cpname.begin(), (int (*)(int)) ::toupper);
				break;

			case 'C':
				if (cstr)
					throw eSyntaxError;

				if (!s_multiCpSystem)
					throw eIllegalOption;

				s_cpname = cstr = optarg;
				break;

			case 'f':
			case 'F':
				if (s_force)
					throw eSyntaxError;

				s_force = true;
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

			case 's':
				if (siStr)
					throw eSyntaxError;

				siStr = optarg;
				{
					char *endptr;
					long int optval = strtol(siStr, &endptr, 10);
					if((endptr[0]) || (optval < 1) || (optval > 2))
						throw InvalidValue(eInvalidValue, string(optarg), cForOption, 's');;
					s_si = optval;
				}
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

void lock_maus(const string& sname, int si)
{
	using namespace CmdFunx;

	string dn = getDn(sname, si);
	int si_locked = isLocked(dn);
	if (si_locked < 0)
		throw eOmHandlerFault;
	string siStr = sname;
	siStr += "-";
	siStr += dynamic_cast<ostringstream &>(ostringstream() << si).str();
	if (si_locked)
		throw InvalidSi(siStr, eInstanceLocked);
	if (lockSi(dn))
		throw InvalidSi(siStr, eLockingFailed);
}

void lock_maus(const string& sname)
{
	using namespace CmdFunx;

	string dn1 = getDn(sname, 1);
	string dn2 = getDn(sname, 2);

	int si1_locked = isLocked(dn1);
	int si2_locked = isLocked(dn2);

	if (si1_locked < 0 || si2_locked < 0 )
		throw eOmHandlerFault;

	if (si1_locked && si2_locked)
		throw eAllInstancesLocked;

	if (!si1_locked && lockSi(dn1))
		throw eLockingFailed;

	if (!si2_locked && lockSi(dn2))
		throw eLockingOneOK;

}
