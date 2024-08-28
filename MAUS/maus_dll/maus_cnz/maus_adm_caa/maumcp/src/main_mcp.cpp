/*
NAME
*/


#include <iostream>
#include <iomanip>

#include "maus_trace.h"
#include "Maus_AehEvent.h"
#include "Maus_Service.h"
#include "Text_Code.h"
#include "AtomicFlag.h"


using namespace std;


extern string s_lockName;
extern BoolFlagT s_apMausRunning;
extern BoolFlagT s_applicationAborting;
extern BoolFlagT s_abortChecksum;
extern Maus_Event s_gShutdown;

const string& NOSERVICE = text_string[cNoService];
const string& LOCK_PATH = text_string[cLockPath];
const string& TRACE_NAME = text_string[cMaus2CompName];
string s_lockName = LOCK_PATH; // + s_procName + ".lock";
static int s_lockfd = -1;

BoolFlagT	s_apMausRunning(false);
BoolFlagT s_applicationAborting(false);
BoolFlagT s_abortChecksum(false);
Maus_Event s_gShutdown;

EventReporter EventReporter::s_instance(2);
ApMaus* g_ApMaus = 0;


void help(const string& name);
void install_signal_handler(int signum);
int lock_application(const string& name);
int run_as_console(const string& name, int instance);
int run_as_amf_service(const string& name, int instance);
void signal_handler(int);
int unlock_application(const string& name);


int main(int argc, char* argv[])
{
	string procPath = argv[0];
	size_t pos = procPath.find_last_of('/');

	string procName = pos == string::npos ? procPath : procPath.substr(pos + 1);
	s_lockName += procName;
	s_lockName += text_string[cLockFileExtention];

    int retVal = EXIT_SUCCESS;
    char *env_comp_name;
    if (argc == 1) {

    	if ((env_comp_name = getenv("SA_AMF_COMPONENT_NAME")) == NULL) {
    		cout << "not started by AMF exiting..." << endl;
    		help(procName);
    		exit(EXIT_FAILURE);
    	}

    	// This is the second instance of MAUS
        retVal = run_as_amf_service(procName, 2);
    }
    else if (argc < 3) {
    	string t(argv[1]);
    	transform(t.begin(), t.end(), t.begin(), (int (*)(int)) ::tolower);

    	if (t == NOSERVICE) {
            retVal = run_as_console(procName, 2);

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
	cout << "Usage: " << name << " [" << NOSERVICE << "]" << endl;
	cout << endl;
	cout << "For example: " << name << " " << NOSERVICE << endl;
	cout << "Running " << name << " as a console service" << endl;
}

void install_signal_handler(int signum)
{
    struct sigaction signalAction;

    signalAction.sa_handler = signal_handler;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;

    sigaction(signum, &signalAction, NULL);
}

int lock_application(const string& name)
{
	TRACE_DEBUG(("lock_application(%s)", name.c_str()));
	int fd;

	fd = open(name.c_str(), O_RDWR | O_CREAT, 0600);
	if (fd < 0)
	{
		TRACE_DEBUG(("lock_application() failed to open file"));
		return -1;
	}
	else
	{
		TRACE_DEBUG(("lock_application() file open ok"));
	}

	s_lockfd = fd;
	struct flock fl;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	if (fcntl(fd, F_SETLK, &fl) < 0) {
		return 1;
	}
	return 0;
}

int run_as_amf_service(const string& name, int instance)
{
	// Daemonization of this application.
	// Tracing instantiation must be after this point
	Maus_Service maus(name, instance);

	MausTrace::initialise(TRACE_NAME.c_str());

	TRACE_DEBUG(("main() %s starts", name.c_str()));
	maus.start();
	TRACE_DEBUG(("main() %s returns", name.c_str()));

	MausTrace::finalise();

	return 0;
}

int run_as_console(const string& name, int instance)
{

	MausTrace::initialise(TRACE_NAME.c_str());

	TRACE_DEBUG(("main() %s starts as console application", name.c_str()));

	int	running = lock_application(s_lockName);
	cout << "Lock application " << s_lockName << " results is " << running << endl;

	switch (running)
	{
	case -1:
		cout << "Cannot create lock file" << endl;
		TRACE_DEBUG(("main() cannot create lock file; %s returns", name.c_str()));
		return EXIT_FAILURE;
	case 1:
		cout << "Already running. Only one instance of " << name << " can run" << endl;
		TRACE_DEBUG(("main() one instance already running; %s returns", name.c_str()));
		return EXIT_FAILURE;
	default:;
	}

	// Deal with Ctrl-C
	install_signal_handler(SIGINT);
	install_signal_handler(SIGTERM);

	Maus_Application maus(instance, s_apMausRunning);
	maus.start();
    cout << "Press Enter key to stop" << endl;
    cin.get();

    maus.stop();
    unlink(s_lockName.c_str());

	TRACE_DEBUG(("main() %s returns", name.c_str()));

	MausTrace::finalise();

	return 0;
}

void signal_handler(int signum)
{
	//cout << "Signum is " << signum << endl;
	(void) signum;
	unlock_application(s_lockName);
}

int unlock_application(const string& name)
{
	TRACE_DEBUG(("unlock_application(%s)", name.c_str()));

	if (s_lockfd != -1) {
		close(s_lockfd);
		s_lockfd = -1;
	}

	int fd = open(name.c_str(), O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		return -1;

	struct flock fl;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fcntl(fd, F_SETLK, &fl);
	close(fd);
	unlink(name.c_str());
	return 0;
}
