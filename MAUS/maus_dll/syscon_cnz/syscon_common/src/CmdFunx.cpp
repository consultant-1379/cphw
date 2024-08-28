/*
 * CmdFunx.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: xdtthng
 */

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include "CmdFunx.h"

using namespace std;


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

    pf = popen((cmd + " 2>&1").c_str(), "r");
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

