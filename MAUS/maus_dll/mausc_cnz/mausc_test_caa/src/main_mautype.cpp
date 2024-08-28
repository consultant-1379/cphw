/*
 * main_mautype.cpp
 *
 *  Created on: Mar 22, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include <fstream>
#include <vector>

#include "ACS_CS_API.h"
#include <ACS_APGCC_CommonLib.h>

using namespace std;

string MAUS1_SRV_NAME = "maus1";
string MAUS2_SRV_NAME = "maus2";

// Use 6,4 or 3, 2
const int sleepTimeInSecondsLong = 3;
const int sleepTimeInSecondsShort = 2;

void help();
int parseCmdLine(int argc, char **argv, string& name, int& val, bool&, int&);
int setMauType(int instance, int value);
int getMauType(int instance);
int execCmd(const string& cmd, vector<string>& result);
int getSlotId();

enum {
	MAUB = 1,
	MAUS = 2
};

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

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


	cout << "\nChanging of mauType test starts" << endl;
	cout << "MAUS process id is <" << oldPid << ">" << endl;
	cout << "MAUS process id on remote node is <" << oldPidRemote << ">" << endl;
	cout << endl;

	int bres = 0;
	if (action > 0)
	{
		switch (action)
		{
		case 1:
			bres = setMauType(instance, MAUB);
			cout << "Set mauType=MAUB once, result <" << bres << ">" << endl;
			break;
		case 2:
			bres = setMauType(instance, MAUS);
			cout << "Set mauType=MAUS once, result <" << bres << ">" << endl;
			break;
		default:
			break;
		}
		return 0;
	}


	//struct timespec req;
	int count = 0;
	for (int i = 0; i < nRepeats; ++i)
	{
		if (i % 7 ==0)
		{
			setMauType(instance, MAUB);

			sleep(sleepTimeInSecondsShort);

			setMauType(instance, MAUB);
			sleep(sleepTimeInSecondsShort);

			cout << "-" << flush;

		}
		else if (i % 11 == 0)
		{

			setMauType(instance, MAUS);

			sleep(sleepTimeInSecondsShort);

			setMauType(instance, MAUS);
			sleep(sleepTimeInSecondsShort);
			cout << "+" << flush;
		}
		else
		{
			setMauType(instance, MAUB);

			sleep(sleepTimeInSecondsLong);

			setMauType(instance, MAUS);

			sleep(sleepTimeInSecondsLong);
			cout << "." << flush;

		}
		if (++count % 50 == 0)
		{
			//cout << "\trun <" << count << ">" << endl;
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
				cout << "old pid <" << oldPid << "> new pid <" << curPid << ">" << endl;
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

	// Just set this to maus before the end
	setMauType(instance, MAUS);
	cout << endl;
	return 0;
}

int getMauType(int instance)
{
	// This is reading off imm data base
	int mautype = -1;
	OmHandler omHandler;

	try
	{
		string cpName = instance == 1 ? "CP1" : "CP2";

		ACS_CC_ReturnType result = omHandler.Init();

		if (result != ACS_CC_SUCCESS) {
			//cout << "cannot start object handler" << endl;
			return -1;
		}

		ACS_APGCC_ImmAttribute mauTypeAttr;
		mauTypeAttr.attrName = "mauType";
		vector<ACS_APGCC_ImmAttribute *> attributes;
		attributes.push_back(&mauTypeAttr);

		// "dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1"
		string dn = "dualSidedCpId=";
		dn += cpName;
		dn += ",logicalMgmtId=1,AxeEquipmentequipmentMId=1";

		result = omHandler.getAttribute(dn.c_str(), attributes);
		if (result != ACS_CC_SUCCESS)
		{
			//cout << "there is something wrong" << endl;
			omHandler.Finalize();
			return -1;
		}

		mautype = *((int*)mauTypeAttr.attrValues[0]);
		//cout << "mau type is " << mautype << endl;
	}
	catch (...) {
		// Do nothing here
		mautype = -1;
	}
	omHandler.Finalize();
	return mautype;
}

int setMauType(int instance, int value)
{
	// This is reading off imm data base
	OmHandler omHandler;

	try
	{
		string cpName = instance == 1 ? "CP1" : "CP2";

		ACS_CC_ReturnType result = omHandler.Init();

		if (result != ACS_CC_SUCCESS) {
			//cout << "cannot start object handler" << endl;
			return -1;
		}

	    // Modify
	    ACS_CC_ImmParameter attrs;
	    /*Fill the rdn Attribute */
	    string tname = "mauType";
	    attrs.attrName = (char*)tname.c_str();
        attrs.attrType = ATTR_INT32T;

        attrs.attrValuesNum = 1;
        void* valueAttrs[1]={reinterpret_cast<void*>(&value)};
        attrs.attrValues = valueAttrs;

		// "dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1"
		string dn = "dualSidedCpId=";
		dn += cpName;
		dn += ",logicalMgmtId=1,AxeEquipmentequipmentMId=1";

		//string transName = "GetMauTypeHere";
		//cout << "dn is " << dn << endl;
		// Using transaction style need to apply request
		// omHandler.resetRequest(transactionName) != ACS_CC_SUCCESS
		// This is to be done as quickly as possible

		result = omHandler.modifyAttribute(dn.c_str(), &attrs);
		//cout << "result is " << result << endl;


		if (result != ACS_CC_SUCCESS)
		{
			cout << "there is something wrong, cannot set attribute" << endl;
			omHandler.Finalize();
			return -1;
		}

	}
	catch (...) {
		// Do nothing here
		cout << "Exception, cannot set attribute" << endl;
		return -1;
	}
	omHandler.Finalize();
	return 0;
}

void help()
{
	cout << "Usage:" << endl;
	cout << endl;
	cout << "mautype -s <service_name> -n <number_of_repeats> [-r] [-b] [-v]" << endl;
	cout << endl;
	cout << "<service_name>:       MAUS1 or MAUS2" << endl;
	cout << "<number_of_repeats>:  number of times the test will repeat" << endl;
	cout << "-r check for remote process id change" << endl;
	cout << endl;
	cout << "-b set MAUB once" << endl;
	cout << "-v set MAUS once (v for virtual)" << endl;
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
	string srvName = "";
	bool cremote = false;
	char *nStr = 0;
	while ((opt = getopt(argc, argv,"s:n:hrbv")) != -1)
	{
		switch (opt)
		{
		case 's':
			srvName = optarg;
			break;
		case 'n':
			nStr = optarg;
			break;
		case 'b':
			action = 1;
			break;
		case 'v':
			action = 2;
			break;
		case 'h':
			helpOpt = true;
			break;
		case 'r':
			cremote = true;
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

    char* endptr = 0;
    long int nVal = 0L;
	if (!action)
	{
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

