/*
 * syscon_main.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include <signal.h>

#include "syscon_service.h"
#include "AppTrace.h"

using namespace std;

static const char* s_traceName = "SCOSRV";
static const string NOSERVICE = "noservice";

Syscon_Server* s_server = 0;

void help(const string& name);
void catch_signal();
void install_signal_handler(int signum);
int run_as_console(const string& name);
int run_as_amf_service(const string& name);


int main(int argc, char* argv[])
{

	string procPath = argv[0];
	size_t pos = procPath.find_last_of('/');
	string procName = pos == string::npos ? procPath : procPath.substr(pos + 1);

    int retVal = EXIT_SUCCESS;
    char *env_comp_name;
    if (argc == 1) {

    	if ((env_comp_name = getenv("SA_AMF_COMPONENT_NAME")) == NULL) {
    		cout << "not started by AMF exiting..." << endl;
    		help(procName);
    		exit(EXIT_FAILURE);
    	}

        retVal = run_as_amf_service(procName);
    }
    else if (argc < 3) {
    	string t(argv[1]);
    	transform(t.begin(), t.end(), t.begin(), (int (*)(int)) ::tolower);

    	if (t == NOSERVICE) {
            retVal = run_as_console(procName);
        }
        else {
            help(procName);
            retVal = EXIT_FAILURE;
        }
    }
    else {
        help(procName);
        retVal = EXIT_FAILURE;
    }

	return retVal;

}

void help(const string& name)
{
	cout << endl;
	cout << "Usage: " << name << " [" << NOSERVICE << "]"  << endl;
	cout << endl;
	cout << "For example: " << name << " " << NOSERVICE << endl;
	cout << "Running " << name << " as a console service" << endl;
}

void catch_signal(int signal)
{

    if (signal != SIGTERM)
    {
        return;
    }

    if (s_server)
    {
    	s_server->stop();
        s_server = 0;
    }
}

void install_signal_handler(int signum)
{
    struct sigaction signalAction;

    signalAction.sa_handler = catch_signal;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;

    sigaction(signum, &signalAction, NULL);
}

int run_as_amf_service(const string& name)
{
	(void) name;
	int ret = EXIT_SUCCESS;
	try
    {
    	Syscon_Service service;
    	AppTrace::initialise(s_traceName);

        // This will loop until receive USR2 signal
        // Or immadm -o3 cmd
        service.run();
    }
    catch (...)
    {
    	ret =  EXIT_FAILURE;
    }

    AppTrace::finalise();
    return ret;
}

int run_as_console(const string& name)
{
	int ret = EXIT_SUCCESS;
	try
	{
		AppTrace::initialise(s_traceName);

		TRACE(("main() %s starts as console application", name.c_str()));

		// Deal with Ctrl-C
		install_signal_handler(SIGINT);
		install_signal_handler(SIGTERM);

		Syscon_Server server;
		server.start();
		s_server = &server;

		cout << "Press Enter key to stop" << endl;
		cin.get();

		server.stop();

		TRACE(("main() %s returns", name.c_str()));

	}
	catch (...)
	{
		ret =  EXIT_FAILURE;
	}
	AppTrace::finalise();
	return ret;
}
