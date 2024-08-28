/*
 * CmdFunx.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: xdtthng
 */

#include <iostream>
#include <sstream>
#include <ACS_APGCC_CommonLib.h>
#include <cstdio>
#include "CmdFunx.h"

using namespace std;

string CmdFunx::getDn(const string& sname, int si)
{
	static const string SAFSI = "safSi=";
	static const string SAFAPP = ",safApp=ERIC-APG";

    //string dn = "safSi=MAUS-1,safApp=ERIC-MAUS";
	string dn = SAFSI;
	dn += sname;
	dn += "-";
	dn += dynamic_cast<ostringstream &>(ostringstream() << si).str();
	dn += SAFAPP;
	return dn;
}

// isLock() returns
// -1 when error in OM Handler
// 0 when it is not locked
// 1 when it is locked

int CmdFunx::isLocked(const string& dn)
{
	static const string SI_ADM_STATE = "saAmfSIAdminState";

    OmHandler omHandler;
    ACS_CC_ReturnType result = omHandler.Init();

    if (result != ACS_CC_SUCCESS)
        return -1;

    ACS_APGCC_ImmAttribute admin;
    admin.attrName = SI_ADM_STATE;

    vector<ACS_APGCC_ImmAttribute *> attributes;
    attributes.push_back(&admin);

    result = omHandler.getAttribute(dn.c_str(), attributes);
    if (result != ACS_CC_SUCCESS)
        return -1;

    int adminstate = *((int*)admin.attrValues[0]);

    // saf admin state 2 == locked; 1 == unlocked
    return adminstate == 2;
}

// execCmd() returns
// -1, when error occurs
// others, value returns by system()
// 0, system() returns 0 when there is no fault

int CmdFunx::execCmd(const string& cmd, const string& dn)
{
	static const string SILENT = " >/dev/null 2>/dev/null";

	string command = cmd;
	command += dn;
	command += SILENT;
	int res = system(command.c_str());
	if (WIFEXITED(res))
		return WEXITSTATUS(res) ;
	return -1;
}

int CmdFunx::execCmd(const string& cmd, vector<string>& result)
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
        }
        ++count;
    }

    // Might fail! What can we do at this point??
    pclose(pf);

	return count;
}

int CmdFunx::lockSi(const string& dn)
{
	static const string LOCK_CMD = "amf-adm lock ";
	return execCmd(LOCK_CMD, dn);
}

int CmdFunx::unLockSi(const string& dn)
{
	static const string LOCK_CMD = "amf-adm unlock ";
	return execCmd(LOCK_CMD, dn);
}

bool CmdFunx::confirm(const string& prompt)
{
	static const string SUGGESTION = " [y(es)/n(o)]: ";

	string ans;
	cout << prompt << SUGGESTION << flush;
	getline(cin, ans);
	transform(ans.begin(), ans.end(), ans.begin(), (int (*)(int)) ::tolower);
	return (ans == "y" || ans == "yes");
}

int CmdFunx::getAmfSuSiAssignment(int siAss[2], const string& siName)
{
	static const string SAFSU = "safSu=";
	static const string SAFSI = "safSi=";
	static const string AMF_SUSI_CMD = "amf-state siass | grep -i ";

	siAss[0] = -1; siAss[1] = -1;
	vector<string> amfOut;
	string cmd = AMF_SUSI_CMD;
	cmd += siName;
	int count = execCmd(cmd, amfOut);

	if (count <= 0)
		return count;

	size_t suPos = 0;
	size_t siPos = 0;
	string safsi = SAFSI;
	safsi += siName;
	safsi += "-";
	count = 0;
	for (vector<string>::iterator it = amfOut.begin(); it != amfOut.end(); ++ it)
	{
		suPos = it->find(SAFSU);
		if (suPos == string::npos)
			continue;

		string suNum = it->substr(suPos + SAFSU.length(), 1);
		int suInt = 0;
		istringstream (suNum) >> suInt;
		if (suInt > 2 || suInt < 1)
			continue;

		siPos = it->find(safsi, suPos + 1);
		if (siPos == string::npos)
			continue;

		int siInt = 0;
		string siNum = it->substr(siPos + safsi.length(), 1);
		istringstream (siNum) >> siInt;
		if (siInt > 2 || siInt < 1)
			continue;
		siAss[--siInt] = --suInt;
		++count;
	}
	return count;
}
