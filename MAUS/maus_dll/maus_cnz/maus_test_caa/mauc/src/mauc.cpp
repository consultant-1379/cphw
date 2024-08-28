#include <iostream>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <iterator>

#include "CmdErrMsg.h"
#include "CmdClient.h"
#include "EndpointView.h"
#include "MAUFunx.h"
#include <getopt.h>
#include "AtomicFlag.h"

//#include "boost/shared_ptr.hpp"
#include <boost/scoped_ptr.hpp>

using namespace std;

extern BoolFlagT s_abortChecksum;
extern BoolFlagT s_applicationAborting;

BoolFlagT s_abortChecksum(false);
BoolFlagT s_applicationAborting(false);

string MAUS1_SRV_NAME = "maus1";
string MAUS2_SRV_NAME = "maus2";

int execCmd(const string& cmd, vector<string>& result);
void help();
int parseCmdLine(int argc, char **argv, string& name, int& val, bool&);


class MausCommand
{
public:
	MausCommand(int instance, bool lock = true);
	~MausCommand();

	void mauls();
	void maumaskset(int mask = 0);
	void maumaskread();
	void maulock();
	void mauunlock();
	void exec();

	void getClient()
	{
		m_cmdClient.reset(new CmdClient(m_instance));
		m_error = 0;
	}

	int error(bool clr = false)
	{
		int error = m_error;
		if (clr)
			m_error = 0;
		return error;
	}

private:

	int								m_instance;
	//CmdClient						m_cmdClient;
	boost::scoped_ptr<CmdClient> 	m_cmdClient;
	bool							m_testLock;
	int								m_error;

	static string s_lockCmd[3];
	static string s_unlockCmd[3];

	void testsleep(uint32_t sleepTimeInMilliSeconds, uint32_t sleepTimeSeconds = 0);
};

string MausCommand::s_lockCmd[3] = { "No cmd", "maulock -cp cp1 -f", "maulock -cp cp2 -f" };
string MausCommand::s_unlockCmd[3] = { "No cmd", "mauunlock -cp cp1", "mauunlock -cp cp2" };


MausCommand::MausCommand(int instance, bool testLock): m_instance(instance),
		m_cmdClient(),
		m_testLock(testLock),
		m_error(0)
{

}

MausCommand::~MausCommand()
{

}

void MausCommand::testsleep(uint32_t sleepTimeInMilliSeconds, uint32_t sleepTimeSeconds)
{
	int32_t sec = sleepTimeInMilliSeconds / 1000;
	int32_t msec = sleepTimeInMilliSeconds % 1000;
	sec += sleepTimeSeconds;
	struct timespec req;
	req.tv_sec = sec;
	req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
	nanosleep(&req, NULL);

}

void MausCommand::exec()
{
	static const uint32_t mseconds = 800;
	static const uint32_t seconds = 0;

	do
	{
#if 0
		// should never run. Just keep it here for reference
		if (m_testLock)
		{
			this->testsleep(mseconds, seconds);
			cout << s_lockCmd[m_instance] << endl;
			this->maulock();
			cout << endl;

			this->testsleep(700, 1);
			cout << s_unlockCmd[m_instance] << endl;
			this->mauunlock();
			this->testsleep(500, 3);
			cout << endl;
		}
#endif

		this->testsleep(mseconds, seconds);
		cout << "mauls -cp cp1" << endl;
		this->mauls();
		if (this->error() == eDSDFatalError)
			break;
		cout << endl;

		this->testsleep(mseconds, seconds);
		cout << "mauset -cp cp1 -m apa cps" << endl;
		this->maumaskset(5);
		if (this->error() == eDSDFatalError)
			break;
		cout << endl;

		this->testsleep(mseconds, seconds);
		cout << "mauset -cp cp1 -r" << endl;
		this->maumaskread();
		if (this->error() == eDSDFatalError)
			break;
		cout << endl;

		this->testsleep(mseconds, seconds);
		cout << "mauset -cp cp1 -z" << endl;
		this->maumaskset(0);
		if (this->error() == eDSDFatalError)
			break;
		cout << endl;

		this->testsleep(mseconds, seconds);
		cout << "mauset -cp cp1 -r" << endl;
		this->maumaskread();
		if (this->error() == eDSDFatalError)
			break;
		cout << endl;
	}
	while (false);

}

void MausCommand::maulock()
{
	try
	{
		vector<string> result;

		int tres = execCmd(s_lockCmd[m_instance], result);

		if (tres < 0)
			cout << "maulock failed" << endl;

		for (int i = 0; i < tres; ++i)
			cout << result[i];

	}
	catch (Cmd_Error_Code x)
	{
		cout << cmd_error_msg[x] << endl;
	}
	catch (...)
	{
		cout << "mauc; maulock; Unknown error" << endl;
	}
}

void MausCommand::mauunlock()
{
	try
	{
		vector<string> result;

		int tres = execCmd(s_unlockCmd[m_instance], result);

		if (tres < 0)
			cout << "mauunlock failed" << endl;

		for (int i = 0; i < tres; ++i)
			cout << result[i];

	}
	catch (Cmd_Error_Code x)
	{
		cout << cmd_error_msg[x] << endl;
	}
	catch (...)
	{
		cout << "mauc; mauunlock; Unknown error" << endl;
	}
}

void MausCommand::maumaskread()
{
	try
	{
		CmdClient::QuorumInfo qd;
		Cmd_Error_Code res = m_cmdClient->getQuorumInfo(qd);

		// Cannot read active mask
		if (res != 0)
		{
			m_error = res;
			throw res;
		}

		if (qd.activeMask == -1)
		{
			throw eFailToGetEpMask;
		}
		cout << "Mask read from maus is " << qd.activeMask << endl;
	}
	catch (Cmd_Error_Code x)
	{
		cout << cmd_error_msg[x] << endl;
	}
	catch (...)
	{
		cout << "mauc; maumaskread; Unknown error" << endl;
	}
}


void MausCommand::maumaskset(int mask)
{
	try
	{
		Cmd_Error_Code res = eUnknown;

		res = m_cmdClient->setActiveMask(mask);

		//cout << "return code is " << res << endl;
		if (res != 0)
		{
			m_error = res;
			throw res;
		}

	}
	catch (Cmd_Error_Code x)
	{
		cout << cmd_error_msg[x] << endl;
	}
	catch (...)
	{
		cout << "mauc; maumaskset; Unknown error" << endl;
	}
}

void MausCommand::mauls()
{
	try
	{
		CmdClient::QuorumInfo qd;
		Cmd_Error_Code res = m_cmdClient->getQuorumInfo(qd);

		if (res != 0)
		{
			m_error = res;
			throw res;
		}

		// It is either 0 or -1
		if (qd.connView <= 0 || qd.quorumView <= 0)
		{
			throw eMausDataNotAvaliable;
		}

		cout << "MAUS MEMBERSHIP INFORMATION" << endl << endl;
		cout << "END POINTS IN CONNECTIVITY VIEW" << endl;

		EndpointView epv = qd.connView;
		cout << epsep(',') << epv << endl;
		cout << endl;

		cout << "END POINTS IN MAUS VIEW" << endl;
		epv.view(qd.quorumView);
		cout << epv << endl;
		cout << endl;

		cout << "ACTIVE MAU HOSTED ON" << endl;
		cout << EndpointView::getName(qd.activeMau) << endl;
		cout << endl;
		cout << "END" << endl;


	}
	catch (Cmd_Error_Code x)
	{
		cout << cmd_error_msg[x] << endl;
	}
	catch (...)
	{
		cout << "mauc; mauls; Unknown error" << endl;
	}

}

int parseCmdLine(int argc, char **argv, string& name, int& val, bool& testLock)
{
	name = "";
	val = 0;
	testLock = false;

	opterr = 0;
	int opt = 0;

	bool helpOpt = false;
	string srvName = "";
	char *nStr = 0;
	bool tlock = false;
	while ((opt = getopt(argc, argv,"s:n:l")) != -1)
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
		case 'l':
			tlock = true;
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

	long int nVal = 0;
    char* endptr = 0;
    nVal = strtol(nStr, &endptr, 10);
    if ((endptr[0])) {
		cout << "Inavlid value <" << nStr << "> for option <n>" << endl;
		return EXIT_FAILURE;
    }

    name = srvName;
    val = nVal;
    testLock = tlock;

	return 0;

}

void help()
{
	cout << "mausc -s service -n repeats" << endl;
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

///////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------

int main(int argc, char **argv)

{

	string serviceName;
	int nRepeats;
	bool testLock;
	int result = parseCmdLine(argc, argv, serviceName, nRepeats, testLock);

	if (result)
		return result;

	// serviceName can only be MAUS1 or MAUS2
	int instance = serviceName == MAUS1_SRV_NAME? 1 : 2;

	vector<string> dateTime;
	string startTime;
	string curTime;
	const string getDate = "date";

	int tres = execCmd(getDate, dateTime);
	if (tres <= 0)
	{
		cout << "Cannot get start date and time, try again" << endl;
		return EXIT_FAILURE;
	}
	startTime = dateTime[0];

	// Get process id
	string procName = "cphw_";
	procName += serviceName;
	procName += "d";
	vector<string> vecPid;
	string getId = "pidof ";
	getId += procName;
	tres = execCmd(getId, vecPid);

	if (tres <= 0)
	{
		cout << "Cannot get process id of MAUS process; try again" << endl;
		return 0;
	}
	int32_t oldPid = atoi(vecPid[0].c_str());
	vecPid.clear();
	int32_t	curPid = oldPid;

	string getIdRemote = "rsh SC-2-";
	int other = MAUFunx::getSlotId() == 1? 2: 1;
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

	// Start test loop
	MausCommand cmd(instance, testLock);
	cmd.getClient();
	for (int i = 0; i < nRepeats; ++i)
	{
		cout << "--------   Run " << i << " of " << nRepeats << " -----------" << endl;

		dateTime.clear();
		tres = execCmd(getDate, dateTime);
		if (tres > 0)
			curTime = dateTime[0];

		cout << curTime;
		cout << "Test started at " << startTime << endl;

		cmd.exec();
		if (cmd.error(true) == eDSDFatalError)
			cmd.getClient();

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

    return 0;
}


