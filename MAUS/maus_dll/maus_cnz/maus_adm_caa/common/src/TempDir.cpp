/*
 * TempDir.cpp
 *
 *  Created on: Jan 14, 2016
 *      Author: xdtthng
 */

#include <climits>
#include <unistd.h>
#include "TempDir.h"
#include "maus_trace.h"

using namespace std;

TempDir::TempDir(const string& newdir) : m_olddir("")
{
	char curr[PATH_MAX];

	if (getcwd(curr, PATH_MAX) == NULL)
	{
	    return;
	}

	m_olddir = curr;
	TRACE(("TempDir::TempDir(), old dir <%s>", curr));

    if (chdir(newdir.c_str()) != 0)
    {
    	TRACE(("TempDir::TempDir(), cannot change to new dir <%s>", newdir.c_str()));
    }
    else
    {
    	TRACE(("TempDir::TempDir(), OK to change to new dir <%s>", newdir.c_str()));
    }
}

TempDir::~TempDir()
{

    if (chdir(m_olddir.c_str()) != 0)
    {
    	// TRACE
    }

}

