/*
 * mauls.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: xdtthng
 */

#include "halPlatformDefinitions.hxx"

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

#include "BMsgQ.h"
#include "MAUApMsgQ.hxx"
#include "Maus_CmdParam.h"
#include "maus_trace.h"
#include "EndpointView.h"
#include "MAUFunx.h"
#include "Exception.h"
#include "CmdErrMsg.h"
#include "Text_Code.h"
#include "CmdCode.h"
#include "AtomicFlag.h"

using namespace std;

extern BoolFlagT s_abortChecksum;
extern BoolFlagT s_applicationAborting;


void execute(int cpId = 0xffff);
void parseCommandLine(int acrg, char** argv);

extern int opterr;

// no_argument

static struct option long_options[] =
{
	{"cp",    	required_argument, 	0, 	'C'},
	{"lock",    no_argument, 		0, 	'l'},
	{"unlock",  no_argument, 		0, 	'u'},
	{0, 		0, 					0, 	0}
};

static bool s_multiCpSystem;
static string s_cpname;
static unsigned int s_lock;
static unsigned int s_unlock;

BoolFlagT s_abortChecksum(false);
BoolFlagT s_applicationAborting(false);

#if 0
typedef boost::tuple<Cmd_Error_Code, string, Text_Code, int> InvalidValue;
typedef pair<string, int> CmdResultError;

EpTableT epTable = boost::assign::map_list_of
	("ap-a", apa)
	("apa", apa)
	("ap-b", apb)
	("apb", apb)
	("cpa", cpa)
	("cp-a", cpa)
	("cp-b", cpb)
	("cpb", cpb);
#endif

int main(int argc, char** argv)
{


	int exitCode = 0;
	int cpId = 0xffff;
	try {
		s_multiCpSystem = MAUFunx::isMultipleCP();
		parseCommandLine(argc, argv);
		if (s_multiCpSystem) {
			transform(s_cpname.begin(), s_cpname.end(), s_cpname.begin(), (int (*)(int)) ::toupper);

			if (s_cpname == "CLUSTER")
				throw eCp1orCp2Supported;

			cpId = MAUFunx::cpNameToCpId(s_cpname.c_str());

			if (cpId != 1001 && cpId != 1002)
				throw eCp1orCp2Supported;

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
				cout << "Usage: mauadm -cp cpName [--lock] [--unlock]" << endl;
			}
			else
			{
				cout << "Usage: mauadm [--lock] [--unlock]" << endl;
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
#if 0
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
#endif
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
	try
	{
		while ((c = getopt_long (argc, argv, "c:", long_options, 0)) != -1)
		{
			switch (c)
			{
			case 'c':

				if (strcmp(argv[optind - 1], "-cp") != 0 || optind == argc || s_lock || s_unlock)
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

			case 'l':
				if (s_lock || s_unlock)
					throw eSyntaxError;

				s_lock = 1;
				break;

			case 'u':
				if (s_lock || s_unlock)
					throw eSyntaxError;

				s_unlock = 1;
				break;

			default:
				throw eSyntaxError;
				break;

			}
		}

        // Have redundant argument
		if (optind < argc || (!s_lock && !s_unlock))
             throw eSyntaxError;

        // If multiple system option,"cp" is mandatory
        if (s_multiCpSystem && s_cpname.empty())
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

	BMsgQ		inQueue;
	MAUApMsgQ	outQueue;
	Maus_CmdParam param;
	pid_t pid = getpid();

	int cmdCode = s_lock? 5 : 7;
	// command code
	param.header(cmdCode, 0, 0, pid);

	string pidstr = dynamic_cast<ostringstream &>(ostringstream() << pid).str();
	inQueue.queueName(pidstr.c_str());
	//cout << "mauls pid is " << pid << endl;
	//cout << "Attempting to create boost message queue" << endl;
	if (!inQueue.create())
	{
		//cout << "Internal error, resources not available, cannot create boost queue" << endl;
		//return 1;
		throw eBMsgQCreate;
	}

	char buffer[MAUApMsgQ::MaxMsgSize];
	int bufferLen = param.publish(buffer, MAUApMsgQ::MaxMsgSize);
	//cout << "bufferLen is " << bufferLen << endl;

	try
	{
		if (cpId == 1002)
			outQueue.setNameTag(MAUFunx::getMsgQNameTag());

		int result = outQueue.attach(MAUApMsgQ::LocalMAUInboundQ);
		//cout << "Queue name for sending to server is " << outQueue.getQueueName() << endl;
		//cout << "result of attach is " << res << endl;
		if (result != MauReturnCode::OK) {
			//cout << "Local MAUS not contactable" << endl;
			//return 1;
			throw eServerUnreachable;
		}

		result = outQueue.send(buffer, bufferLen);
		if (result != MauReturnCode::OK)
		{
			//cout << "Local MAUS not contactable" << endl;
			//return 1;
			throw eServerUnreachable;
		}
		//cout << "resutl from sending message to the server is " << result << endl;
		char msg[BMsgQ::MAX_MSG_SIZE];
		size_t size = 0;
		inQueue.receive(msg, size);
		//cout << "received message with size " << size << endl;

		if (size == 0)
		{
			throw eEmptyMsg;
		}

		Maus_CmdParam response(msg);
		result = response.getResult();
		if (result == CMD_BUSY) {
			throw eCommandBusy;
		}

		if (result) {
			cout << "Set mask return non zero result " << result << endl;
		}



	}
	catch (...)
	{
		inQueue.remove();
		throw;
	}
	inQueue.remove();
}


