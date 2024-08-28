/*
NAME
*/

#include <iostream>
#include <cstring>

// For signal handling
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

//#include "MausOM.h"
//#include "MausOI.h"

#include "SwMauOI.h"

#include "maus_trace.h"
#include "GMClient.h"
#include "MauMsg.h"
#include "MauscService.h"
#include "CpMauM.h"
#include "CptSwMau.h"
#include "Maus_AehEvent.h"
#include "MausSysInfo.h"
#include "MausCpTableObserver.h"
#include "MAServer.h"

// Only for testing SwMau during early development
// Should be removed

#include "SwMauMom.h"
#include "SwMauOM.h"

const char* s_noservice = "noservice";
const char* s_daemon   = "daemon";
const char* s_traceName = "MAUSC";
static SwMauOI*	s_oi = 0;
boost::mutex CpMauM::s_cs;
boost::mutex CptSwMau::s_cs;
EventReporter EventReporter::s_instance(3);


// For signal handling
void catch_signal();
void install_signal_handler();

// Model functions
void help();
static int run_as_console();
static int run_as_service();
static int run_as_daemon();

// Test drivers.
// Consider to remove them in final production
//
//static void testGMClient();
//static void testGetOpStateMsg();
//static void testDisableMsg();

using namespace std;

void sigHandler(int sig); //TR_HY57051
int setupSignalHandler(const struct sigaction* sigAction); //TR_HY57051


int main(int argc, char** argv)
{

    int retVal = EXIT_SUCCESS;

    // ------------------------------------------------------------------------
    // Parsing command line
    // ------------------------------------------------------------------------

    int opt;
    bool helpOpt = false;
    opterr = 0;
    while((opt = getopt(argc, argv, "h")) != -1) {
        switch(opt) {
        case 'h':
        case '?':
            helpOpt = true;
            break;
        default:;
        }
    }

    if (helpOpt) {
        help();
        return EXIT_FAILURE;
    }

    if (argc == 1) {

    	// Run AMF service
        retVal = run_as_service();
    }
    else if (argc == 2) {
    	string t(argv[1]);
    	transform(t.begin(), t.end(), t.begin(),(int (*)(int)) ::tolower);
    	if (t == s_noservice) {
            retVal = run_as_console();
        }
        else if (t == s_daemon) {
            // Run as Linux daemon service
        	retVal = run_as_daemon();
        }
        else {
            help();
            retVal = EXIT_FAILURE;
        }
    }
    else {
        help();
        retVal = EXIT_FAILURE;
    }

    return retVal;
}

void help()
{
	cout << "Usage: cphw_mauscd [noservice | daemon]" << std::endl;

}

int run_as_service()
{
	int ret = EXIT_SUCCESS;

	try
	{

		MauscService service;
		MausTrace::initialise(s_traceName);


		//TR_HY57051: Setting signal handler for SIGPIPE
		struct sigaction sa;
		sa.sa_handler = sigHandler;
		sa.sa_flags = SA_RESTART;
		int res = sigemptyset(&sa.sa_mask );

		if (res != 0)
		{
			TRACE_WARNING(("sigemptyset returned %d", res));
		}

		int result = setupSignalHandler(&sa);

		if (result != 0)
		{
			TRACE_ERROR(("Signal handler could not be setup"));
			return EXIT_FAILURE;
		}

		// This will loop until receive USR2 signal
		// Or immadm -o3 cmd
		TRACE(("run_as_service() starts"));
		service.run();
		TRACE(("run_as_service() returns"));
	}
	catch (...)
	{
		ret = EXIT_FAILURE;
	}

	MausTrace::finalise();
	return ret;

}

int run_as_daemon()
{
    int ret = EXIT_SUCCESS;
    install_signal_handler();
    try
    {
    	MausTrace::initialise(s_traceName);
    	pid_t pid, sid;

    	//Fork the Parent Process
    	pid = fork();

    	// This is error, just go
    	if (pid < 0) { return EXIT_FAILURE; }

    	//We got a good pid, Close the Parent Process
    	if (pid > 0) { return EXIT_SUCCESS; }

    	//Change File Mask
    	umask(0);

    	//Create a new Signature Id for our child
    	sid = setsid();
    	if (sid < 0) { return EXIT_FAILURE; }

    	//Change Directory
    	//If we cant find the directory we exit with failure
    	//This is where most run stuffs are in the apg
    	if ((chdir("/var/run")) < 0) { return EXIT_FAILURE; }

        //Close Standard File Descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

    	int architecture;
    	int mauType[2];
    	SysInfo::getSysInfo(architecture, mauType);
    	SysInfo::EventT changes;
    	CpTableObserver observer(architecture, mauType, changes);
    	observer.init();

        SwMauOI oi(architecture, mauType, changes);
        oi.start();
        s_oi = &oi;
        oi.join();
    }
    catch (...) {

    	ret =  EXIT_FAILURE;
    }

    MausTrace::finalise();
	return ret;
}


void deleteObject()
{

	cout << "Testing of SwMauM" << endl;
	//string dnName = SwMauMom::getParentInstance();
	SwMauOM swMauOm;
	if (swMauOm.deleteObject(SwMauMom::Cp2))
	{
		cout << "ok to delete swMauId=1 for CP2" << endl;;
	}
	else
	{
		cout << "failed delete swMauId=1 for CP2" << endl;;
	}

	cout << "Testing of SwMauM finish" << endl;
}


void getAdmState()
{
    int admState[2];
    SwMauOM om;
    if (om.readAdminState(admState))
    {
    	cout << "adm state for cp1 " << admState[0] << endl;
    	cout << "adm state for cp2 " << admState[1] << endl;
    }
    else
    {
    	cout << "failed to read adm state" << endl;
    }

}

int run_as_console()
{
    int ret = EXIT_SUCCESS;

    try
    {

#if 1

    	MausTrace::initialise(s_traceName);
		MAServer tcpServer;
		tcpServer.start();
		tcpServer.waitUntilRunning();

    	int architecture;
    	int mauType[2];
    	SysInfo::getSysInfo(architecture, mauType);
    	SysInfo::EventT changes;
    	CpTableObserver observer(architecture, mauType, changes);
    	observer.init();


        SwMauOI oi(architecture, mauType, changes);
		oi.start();

		// Grand closing
		std::cout << "Press Enter key to stop" << std::endl;
		std::cin.get();

		tcpServer.stop();
		oi.stop();


#endif

#if 0
    	{

    		MAServer tcpServer;
    		tcpServer.start();
    		tcpServer.waitUntilRunning();

    		int architecture;
    		int mauType[2];
    		SysInfo::getSysInfo(architecture, mauType);
    		SysInfo::EventT changes;
    		CpTableObserver observer(architecture, mauType, changes);
    		observer.init();

    		MausOI oi(architecture, mauType, changes);
    		oi.start();

    		// Grand closing
    		std::cout << "Press Enter key to stop" << std::endl;
    		std::cin.get();

    		tcpServer.stop();
    		oi.stop();

    	}

//#else
        //testGMClient();
        //testGetOpStateMsg();
        //testDisableMsg();
#endif

    }
    catch (...) {

    	ret =  EXIT_FAILURE;
    }

    MausTrace::finalise();
	return ret;
}

void catch_signal(int signal)
{

    if (signal != SIGTERM)
    {
        return;
    }

    if (s_oi)
    {
        s_oi->stop();
        s_oi = 0;
    }
}

void install_signal_handler()
{
    struct sigaction signalAction;

    signalAction.sa_handler = catch_signal;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;

    // Register to handle SIGTERM
    sigaction(SIGTERM, &signalAction, NULL);
}


//TR_HY57051: Added signal handler for SIGPIPE
//This signal handler will be used when service is running as a amf service.
void sigHandler(int sig)
{
	if(sig == SIGPIPE)
	{
		TRACE_WARNING(("Caught SIGPIPE signal, ignoring it")); //Ignoring SIGPIPE
	}
	else
	{
		TRACE_WARNING(("Unhandled signal caught: %d",sig));
	}
}


int setupSignalHandler(const struct sigaction* sigAction)
{
	if( sigaction(SIGPIPE, sigAction, NULL ) == -1)
	{
		TRACE_ERROR(("Error occurred while setting signal handler for SIGPIPE"));
		return -1;
	}
	return 0;
}



#if 0
void testGetOpStateMsg()
{
	string domain = "MAUS";
	string name = "CP1MAUM";


	cout << "Testing GM Client to Active node" << endl;
	GMClient gm(name.c_str(), domain.c_str(), 0);

	bool conRes = gm.connect();
	cout << "connect result is " << conRes << endl;

	if (!conRes)
		return;

	MauMsg msg;
	new (msg.addr()) MauMsg::GetOperationStateMsg(1, 0);
	const MauMsg::GetOperationStateMsg* cmd = reinterpret_cast<MauMsg::GetOperationStateMsg*>(msg.addr());
	//cout << "send the following msg" << endl;
	cout << *cmd << endl;
	size_t res = gm.send(msg.addr(), 1024);

	const MauMsg::GetOperationStateRspMsg* rsp = reinterpret_cast<MauMsg::GetOperationStateRspMsg*>(msg.addr());
	cout << "response message" << endl;
	cout << *rsp << endl;


}

void testDisableMsg()
{
	string domain = "MAUS";
	string name = "CP1MAUM";

	cout << "Testing GM Client to Active node" << endl;
	GMClient gm(name.c_str(), domain.c_str(), 0);

	bool conRes = gm.connect();
	cout << "connect result is " << conRes << endl;

	if (!conRes)
		return;

	MauMsg msg;
	new (msg.addr()) MauMsg::DisableMsg(1, 0);
	const MauMsg::DisableMsg* cmd = reinterpret_cast<MauMsg::DisableMsg*>(msg.addr());
	//cout << "send the following msg" << endl;
	cout << *cmd << endl;
	size_t res = gm.send(msg.addr(), 1024);

	const MauMsg::DisableRspMsg* rsp = reinterpret_cast<MauMsg::DisableRspMsg*>(msg.addr());
	cout << "response message" << endl;
	cout << *rsp << endl;



}

void testGMClient()
{
	string domain = "MAUS";
	string name = "CP1MAUM";

	{
		cout << "Testing GM Client to Active node" << endl;
		GMClient gm(name.c_str(), domain.c_str(), 0);

		bool conRes = gm.connect();
		cout << "connect result is " << conRes << endl;

		if (!conRes)
			return;

		string buffer = "Just a hello to the other end";
		char resBuffer[1024];
		memset(resBuffer, 0, 1024);
		memcpy(resBuffer, buffer.c_str(), buffer.length());
		size_t res = gm.send(resBuffer, 1024);

		cout << "actual size of sending is " << res << endl;
		cout << "receive from client code is " << gm.getResultCode() << endl;
		if (res)
		{
			cout << "returned message is: <" << resBuffer << ">" << endl;
		}
		cout << endl;
	}

#if 0
	{
		cout << "Testing GM Client to Passive node" << endl;
		GMClient gm(name.c_str(), domain.c_str(), 1);

		bool conRes = gm.connect();
		cout << "connect result is " << conRes << endl;

		if (!conRes)
			return;

		string buffer = "Jsut a hello to the other end";
		char resBuffer[1024];
		memset(resBuffer, 0, 1024);
		memcpy(resBuffer, buffer.c_str(), buffer.length());
		size_t res = gm.send(resBuffer, 1024);

		cout << "actual size of sending is " << res << endl;
		cout << "receive from client code is " << gm.getResultCode() << endl;
		if (res)
		{
			cout << "returned message is: <" << resBuffer << ">" << endl;
		}
	}
#endif
}
#endif
