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
#include <iomanip>
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

BoolFlagT s_abortChecksum(false);
BoolFlagT s_applicationAborting(false);

void execute(string, int);
void parseCommandLine(int acrg, char** argv);

extern int opterr;

typedef pair<const char*, Cmd_Error_Code> InvalidSiError;

static struct option long_options[] =
{
	{"cp",    	required_argument, 	0, 	'C'},
	{"name",   required_argument, 	0, 	'N'},
	{0, 		0, 					0, 	0}
};

typedef pair<string, Cmd_Error_Code> InvalidSi;
typedef boost::tuple<Cmd_Error_Code, string, Text_Code, int> InvalidValue;

static bool s_multiCpSystem;
static bool s_force;
static string s_cpname;
static string s_serviceName;


int main(int argc, char** argv)
{

	int exitCode = 0;
	try {
		s_multiCpSystem = MAUFunx::isMultipleCP();
		parseCommandLine(argc, argv);

		// Locking service using service name
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
		execute(sname, cpId);
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
				cout << "Usage: mauadmls -cp cpName | -n serviceName" << endl;
			}
			else
			{
				cout << "Usage: mauadmls [-n serviceName]" << endl;
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
	catch (InvalidSiError& e)
	{
		cout << cmd_error_msg[e.second] << e.first << endl;
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
	int c;
	opterr = 0;
	char* cstr = 0;
	char* nStr = 0;

	try
	{
		//while ((c = getopt_long (argc, argv, "c:f", long_options, 0)) != -1)
		while ((c = getopt_long (argc, argv, "c:n:", long_options, 0)) != -1)
		{
			switch (c)
			{
			case 'c':

				if (strcmp(argv[optind - 1], "-cp") != 0 || cstr || s_force)
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

#if 0
			case 'f':
			case 'F':
				if (s_force)
					throw eSyntaxError;

				s_force = true;
				break;
#endif
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

	}

}

void execute(string sname, int cpId)
{
	static const string LOCKED = "Locked";
	static const string UNLOCKED = "Unlocked";
	static const char* apNames[] = {"AP-A", "AP-B"};
	static const string DASH = "-";

	using namespace CmdFunx;

	// This command does not contact the local instance of MAUS
	string dn1 = getDn(sname, 1);
	string dn2 = getDn(sname, 2);

	int si1_locked = isLocked(dn1);
	int si2_locked = isLocked(dn2);

	// CP name and ID were already verified with CS
	string cpName = cpId == 0xffff? "" : cpId == 1001 ? "CP1" : "CP2";
	string adm1 = si1_locked ? LOCKED : UNLOCKED;
	string adm2 = si2_locked ? LOCKED : UNLOCKED;

	cout << "MAUS Administration Information " << endl;
	cout << endl;

	if (cpId != 0xffff)
	{
		cout << "CP NAME" << endl;
		cout << cpName << endl;
		cout << endl;
	}
	cout << flush;

	cout << setw(10) << left << (sname + "-1") << (sname + "-2") << endl;
	cout << setw(10) << left << adm1 << adm2 << endl;

	if (si1_locked && si2_locked)
	{
		cout << setw(10) << left << DASH << DASH << endl;
	}
	else
	{
		int si[2];
		getAmfSuSiAssignment(si, sname);
		string siass[2];
		siass[0] = si[0] < 0 ? DASH : apNames[si[0]];
		siass[1] = si[1] < 0 ? DASH : apNames[si[1]];
		cout << setw(10) << left << siass[0] << siass[1] << endl;
	}
	cout << endl;
	cout << "END" << endl;
}
