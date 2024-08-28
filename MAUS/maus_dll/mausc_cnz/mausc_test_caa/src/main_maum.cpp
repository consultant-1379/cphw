#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <getopt.h>
#include <cstdlib>

#include "CpMauM.h"


using namespace std;

string MAUS1_SRV_NAME = "maus1";
string MAUS2_SRV_NAME = "maus2";

boost::mutex CpMauM::s_cs;

const int sleepTimeInSeconds = 1;
const size_t sleepTimeInNanoSeconds = 500000000L;    // Example 100 ms = 100 000 000 nano seconds


void help();
int parseCmdLine(int argc, char **argv, string& name, int& val, bool&, int&);
int execCmd(const string& cmd, vector<string>& result);
int getSlotId();

int main(int argc, char **argv)
{

	// Parsing command line
	string serviceName;
	int nRepeats;
	bool checkRemote;
	int action = 0;
	int result = parseCmdLine(argc, argv, serviceName, nRepeats, checkRemote, action);

	if (result)
		return result;

	// serviceName can only be MAUS1 or MAUS2
	int instance = serviceName == MAUS1_SRV_NAME? 1 : 2;

	//struct timespec req;
	CpMauM test(instance);
	int count = 0;
	struct timespec req;

	// Process name
	string procName = "cphw_";
	procName += serviceName;
	procName += "d";
	vector<string> vecPid;
	string getId = "pidof ";
	getId += procName;
	int tres = execCmd(getId, vecPid);
	//cout << "Process id of " << procName << " " << oldPid << endl;
	//cout << "tres is " << tres << endl;
	//cout << "second line is <" << vecPid[1] << ">" << endl;

	if (tres <= 0)
	{
		cout << "Cannot get process id of MAUS process; try again" << endl;
		return 0;
	}
	int32_t oldPid = atoi(vecPid[0].c_str());
	vecPid.clear();
	int32_t	curPid = oldPid;

	string getIdRemote = "rsh SC-2-";
	int other = getSlotId() == 1? 2: 1;
	string ostr = dynamic_cast<ostringstream &>(ostringstream() << other).str();
	getIdRemote += ostr;
	getIdRemote += " '";
	getIdRemote += getId;
	getIdRemote += "'";

	tres = execCmd(getIdRemote, vecPid);
	if (tres <= 0)
	{
		cout << "Cannot get process id of MAUS process on remote node; try again" << endl;
		return 0;
	}
	int32_t oldPidRemote = atoi(vecPid[0].c_str());
	vecPid.clear();
	int32_t	curPidRemote = oldPidRemote;


	cout << "\nSimulation of CpMauM operation test starts" << endl;
	cout << "MAUS process id is <" << oldPid << ">" << endl;
	cout << "MAUS process id on remote node is <" << oldPidRemote << ">" << endl;
	cout << endl;

	//cout << "now testing enable" << endl;
	//bool ret = test.enable();
	//cout << "enable returns as " << ret << endl;
	//cout << "now testing disable" << endl;
	//ret = test.disable();
	//cout << "disable returns as " << ret << endl;
	//return 0;

	bool bres = false;
	if (action > 0)
	{
		switch (action)
		{
		case 1:
			bres = test.enable();
			cout << "Enable once, result " << (bres?"true":"false") << endl;
			break;
		case 2:
			cout << "calling disable here" << endl;
			bres = test.disable();
			cout << "Disable once, result " << (bres?"true":"false")<< endl;
			break;
		default:
			break;
		}
		// Waiting for the server to perform tasks
		req.tv_sec = sleepTimeInSeconds;
		req.tv_nsec = sleepTimeInNanoSeconds;
		nanosleep(&req, NULL);

		return 0;
	}


	for (int i = 0; i < nRepeats; ++i)
	{
		if (count % 7 ==0)
		{
			test.disable();

			//req.tv_sec = sleepTimeInSeconds;
			req.tv_sec = 0UL;
			req.tv_nsec = sleepTimeInNanoSeconds;
			nanosleep(&req, NULL);
			//sleep(sleepTimeInSeconds);

			test.disable();
			req.tv_sec = sleepTimeInSeconds;
			//req.tv_sec = 0UL;
			req.tv_nsec = sleepTimeInNanoSeconds;
			nanosleep(&req, NULL);
			//sleep(sleepTimeInSeconds);

			cout << "-" << flush;
		}
		else if (count % 11 == 0)
		{

			test.enable();

			//req.tv_sec = sleepTimeInSeconds;
			req.tv_sec = 0UL;
			req.tv_nsec = sleepTimeInNanoSeconds;
			nanosleep(&req, NULL);
			//sleep(sleepTimeInSeconds);

			test.enable();
			req.tv_sec = sleepTimeInSeconds;
			//req.tv_sec = 0UL;
			req.tv_nsec = sleepTimeInNanoSeconds;
			//sleep(sleepTimeInSeconds);
			cout << "+" << flush;
		}
		else
		{
			test.disable();

			req.tv_sec = sleepTimeInSeconds;
			//req.tv_sec = 0UL;
			req.tv_nsec = sleepTimeInNanoSeconds;
			nanosleep(&req, NULL);
			//sleep(sleepTimeInSeconds);

			test.enable();
			req.tv_sec = 2*sleepTimeInSeconds;
			//req.tv_sec = 0UL;
			req.tv_nsec = sleepTimeInNanoSeconds;
			nanosleep(&req, NULL);
			//sleep(sleepTimeInSeconds);

			cout << "." << flush;
		}

		if (++count % 50 == 0)
		{
			cout << "\trun <" << count << " of " << nRepeats << ", MAUS pid " << oldPid << ">" << endl;
		}

		tres = execCmd(getId, vecPid);
		if (tres == 1)
		{
			curPid = atoi(vecPid[0].c_str());
			vecPid.clear();
			if (oldPid != curPid)
			{
				cout << endl;
				cout << "Pid of MAUS changes; stop test at iteration <" << i << ">" << endl;
				cout << "Old pid <" << oldPid << "> new pid <" << curPid << ">" << endl;
				break;
			}

		}

		if (!checkRemote)
			continue;

		tres = execCmd(getIdRemote, vecPid);
		if (tres == 1)
		{
			curPidRemote = atoi(vecPid[0].c_str());
			vecPid.clear();
			if (oldPidRemote != curPidRemote)
			{
				cout << endl;
				cout << "Pid of MAUS changes on remote node; stop test at iteration <" << i << ">" << endl;
				cout << "Old pid on remote node <" << oldPidRemote << "> new pid <" << curPidRemote << ">" << endl;
				break;
			}
		}
	}

	cout << endl;
	return 0;
}

void help()
{
	cout << "Usage:" << endl;
	cout << endl;
	cout << "maum -s <service_name> -n <number_of_repeats> [-e ] [-d] [-r] " << endl;
	cout << endl;
	cout << "<service_name>:       MAUS1 or MAUS2" << endl;
	cout << "<number_of_repeats>:  number of times the test will repeat" << endl;
	cout << "-r check for remote process id change" << endl;
	cout << endl;
	cout << "-e enable once" << endl;
	cout << "-d disable once" << endl;
}

int parseCmdLine(int argc, char **argv, string& name, int& val, bool& checkRemote, int& action)
{
	name = "";
	val = 0;
	checkRemote = false;
	action = 0;

	opterr = 0;
	int opt = 0;

	bool helpOpt = false;
	bool cremote = false;
	string srvName = "";
	char *nStr = 0;
	while ((opt = getopt(argc, argv,"s:n:hrde")) != -1)
	{
		switch (opt)
		{
		case 's':
			srvName = optarg;
			break;
		case 'n':
			nStr = optarg;
			break;
		case 'h':
			helpOpt = true;
			break;
		case 'r':
			cremote = true;
			break;
		case 'e':
			action = 1;
			break;
		case 'd':
			action = 2;
			break;
		default:
			helpOpt = true;
			break;
		}
	}

	transform(srvName.begin(), srvName.end(), srvName.begin(),(int (*)(int)) ::tolower);
	if (helpOpt || srvName.length() == 0 ||
	   (srvName != MAUS1_SRV_NAME && srvName != MAUS2_SRV_NAME) )
	{
		help();
		return EXIT_FAILURE;
	}

	long int nVal = 0L;
	if (!action)
	{
		char* endptr = 0;
		nVal = strtol(nStr, &endptr, 10);
		if ((endptr[0])) {
			cout << "Inavlid value <" << nStr << "> for option <n>" << endl;
			return EXIT_FAILURE;
		}
	}

    name = srvName;
    val = nVal;
    checkRemote = cremote;

	return 0;

}

int execCmd(const string& cmd, vector<string>& result)
{
	static const int SIZE = 255;

    FILE *pf;
    char data[SIZE];

    pf = popen(cmd.c_str(), "r");
    if (!pf) {
      return -1;
    }

    int count = 0;
    while (!feof(pf)) {
        if (fgets(data, SIZE , pf)) {
        	result.push_back(data);
            ++count;
       }
    }

    // Might fail! What can we do at this point??
    pclose(pf);

	return count;
}

int getSlotId()
{
	static const char* s_slotFileName = "/etc/opensaf/slot_id";
	static int s_slotId = -1;

	if (s_slotId == -1)
	{
		ifstream slotFile(s_slotFileName);

		if (slotFile.good()) {
			int slot = -1;
			slotFile >> slot;
			s_slotId = slotFile.good() ? slot : -1;
		}
	}
	return s_slotId;
}

