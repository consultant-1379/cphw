#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <sstream>
#include <getopt.h>

#include "CPHW_MAUS_API_Libh.h"

using namespace std;

enum OperT
{
	InvalidOper = -1,
	LoadLib = 0,
	UnloadLib = 1,
	CheckOperState = 2,
	ReloadLib = 3,
	Checksum = 4,
	InstallMibs = 5,
	OperTAll
};

string MAUS1_SRV_NAME = "maus1";
string MAUS2_SRV_NAME = "maus2";

void help();
int parseCmdLine(int argc, char **argv, string& name, int& val, int&, int& config);
int loadLib(int instance, int node, int);
int unloadLib(int instance, int node, int);
int reloadLib(int instance, int node, int);
int reloadLibOnOneNode(int instance, int node);
int getOperState(int instance, int node, int);
int checksum(int instance, int node, int);
int installMibs(int instance, int node, int);

typedef int(*LibHandling)(int, int, int);

int main(int argc, char** argv)
{
	string servName;
	int val;
	int oper;
	int config;

	int result = parseCmdLine(argc, argv, servName, val, oper, config);
	if (result)
		return result;

	LibHandling func[OperTAll];
	func[0] = loadLib;
	func[1] = unloadLib;
	func[2] = getOperState;
	func[3] = reloadLib;
	func[4] = checksum;
	func[5] = installMibs;

	// serviceName can only be MAUS1 or MAUS2
	int instance = servName == MAUS1_SRV_NAME? 1 : 2;

	return func[oper](instance, val, config);
}

int installMibs(int instance, int node, int config)
{

	cout << "installMibs for instance " << instance << " and node " << node
			<< " config " << config << endl;

	CPHW_MAUS_API_Libh maus(instance);
	maus.installMibs(node, config);

	return 0;
}

int checksum(int instance, int node, int config)
{

	cout << "Checksum for instance " << instance << " and node " << node
			<< " config " << config << endl;

	CPHW_MAUS_API_Libh maus(instance);
	maus.performChecksum(node, config);

	return 0;
}

int parseCmdLine(int argc, char **argv, string& name, int& val, int& oper, int& config)
{
	name = "";
	val = 0;
	config = 0;
	oper = InvalidOper;

	opterr = 0;
	int opt = 0;

	bool helpOpt = false;
	string srvName = "";
	char *nStr = 0;
	char *cStr = 0;
	int iOper = InvalidOper;
	while ((opt = getopt(argc, argv,"c:s:n:lmour")) != -1)
	{
		switch (opt)
		{
		case 's':
			srvName = optarg;
			break;
		case 'n':
			nStr = optarg;
			break;
		case 'c':
			cStr = optarg;
			iOper = Checksum;
			break;
		case 'h':
			helpOpt = true;
			break;
		case 'r':
			//cremote = true;
			iOper = ReloadLib;
			break;
		case 'l':
			iOper = LoadLib;
			break;
		case 'm':
			iOper = InstallMibs;
			break;
		case 'u':
			iOper = UnloadLib;
			break;
		case 'o':
			iOper = CheckOperState;
			break;
		default:
			helpOpt = true;
			break;
		}
	}

	transform(srvName.begin(), srvName.end(), srvName.begin(),(int (*)(int)) ::tolower);
	if (helpOpt || srvName.length() == 0 ||
	   (srvName != MAUS1_SRV_NAME && srvName != MAUS2_SRV_NAME) ||
	   (nStr == 0) || (iOper == InvalidOper))
	{
		help();
		return EXIT_FAILURE;
	}

    char* endptr = 0;
    long int nVal = strtol(nStr, &endptr, 10);
    if ((endptr[0] || nVal < 0 || nVal > 2)) {
		cout << "Inavlid value <" << nStr << "> for option <n>" << endl;
		return EXIT_FAILURE;
    }

    endptr = 0;
    long int cVal = 0;
    if (iOper == Checksum)
    {
    	cVal = strtol(cStr, &endptr, 10);
    	if (endptr[0] || cVal < 0 || cVal > 2) {
    		cout << "Inavlid value <" << cStr << "> for option <c>" << endl;
    		return EXIT_FAILURE;
    	}
    }

    name = srvName;
    val = nVal;
    config = cVal;
    oper = iOper;
	return 0;

}

void help()
{
	cout << endl;
	cout << "Usage:" << endl;
	cout << "maulib -s <service_name> -n <node> -c <config> {-l -m -u -o -r}" << endl;
	cout << endl;
	cout << "-c for checksum file in <config> section" << endl;
	cout << "-l for loading the lib" << endl;
	cout << "-m for installing mibs" << endl;
	cout << "-u for unloading the lib" << endl;
	cout << "-o for checking operational state" << endl;
	cout << "-r for reload, i.e. unload following by load" << endl;

	cout << endl;
	cout << "<service name> is maus1 or maus2" << endl;
	cout << "<node> is 0, 1, 2 for loading and unloading operation" << endl;
	cout << "<node> is 0, 1 for checking operational state operation" << endl;
	cout << "<node> == 0 is AP-A, <node> == 1 is AP-B, <node> == 2 is both AP-A and AP-B" << endl;
	cout << "<config> == 0 is [new], <config> == 1 is [cur], <config> == 2 is [bak]" << endl;
	cout << endl;
}

int loadLib(int instance, int node, int)
{
	cout << "loadLib(" << instance << ", " << node << ")" << flush;

	CPHW_MAUS_API_Libh maus(instance);
	int res = maus.loadLib(node);

	cout << " returns <" << res << ">" << endl;
	cout << endl;

	return res;
}


int unloadLib(int instance, int node, int)
{

	cout << "unloadLib(" << instance << ", " << node << ")" << flush;

	CPHW_MAUS_API_Libh maus(instance);

	int res = maus.unloadLib(node);

	cout << " returns <" << res << ">" << endl;
	cout << endl;

	return res;

}

int getOperState(int instance, int node, int)
{

	CPHW_MAUS_API_Libh maus(instance);
	int operState;
	int res = maus.getOperationalState(node, operState);

	cout << "operState(" << instance << ", " << node << ")" << flush;
	cout << " returns <" << res << ">" << flush;
	if (res)
		cout << endl;
	else
		cout << " " << "operational state <" << operState << ">" << endl;
	cout << endl;

	return res;
}

int reloadLib(int instance, int node, int)
{
	if (node != 2)
	{
		return reloadLibOnOneNode(instance, node);
	}

	cout << "reload on both nodes testing" << endl;
	CPHW_MAUS_API_Libh maus(instance);
	int res = maus.unloadLib(node);

	if (res == CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED ||
			res == CPHW_MAUS_API_Libh::RC_SUCCESS)
	{
		int32_t msec = 400;
		struct timespec req;
		req.tv_sec = 0;
		req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
		nanosleep(&req, NULL);
		res = maus.loadLib(node);
		if (res != CPHW_MAUS_API_Libh::RC_SUCCESS)
		{
			cout << "load lib failed with code <" << res << ">" << endl;
		}
	}
	else
	{
		cout << "unload lib failed with code <" << res << ">" << endl;
	}

	return res;
}

int reloadLibOnOneNode(int instance, int node)
{
	cout << "reload on one node testing" << endl;
	CPHW_MAUS_API_Libh maus(instance);

	cout << "first unload the lib" << endl;
	int res = maus.unloadLib(node);

	if (res != 0)
	{
		cout << "error in unloading value "<< res << endl;
		return res;
	}
	else {
		cout << "unloading ok" << endl;
	}

	int32_t msec = 250;
	cout << "sleep for " << msec << " ms before loading" << endl;
	struct timespec req;
	req.tv_sec = 0;
	req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
	nanosleep(&req, NULL);

	cout << "wake up and start loading" << endl;
	res = maus.loadLib(node);

	if (res != 0)
	{
		cout << "error in loading value " << res << endl;
		return res;
	}
	else {
		cout << "loading ok" << endl;
	}
	//cout << "loadLib(" << instance << ", " << node << ")" << flush;
	//cout << " returns <" << res << ">" << endl;
	//cout << endl;

	return res;
}


