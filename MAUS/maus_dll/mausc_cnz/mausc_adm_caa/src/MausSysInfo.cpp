/*
 * MausSysInfo.cpp
 *
 *  Created on: Mar 15, 2015
 *      Author: xdtthng
 */

#include "MausSysInfo.h"
#include "MAUFunx.h"
#include "maus_trace.h"
#include "ACS_CS_API.h"

int SysInfo::s_cpId[2];


void SysInfo::getCpId(int cpId[])
{
	TRACE(("SysInfo::getCpId()"));

	s_cpId[0] = MAUFunx::cpNameToCpId("cp1");
	s_cpId[1] = MAUFunx::cpNameToCpId("cp2");

	TRACE(("SysInfo::getCpId(), Dual Sided cp1 <%d>", s_cpId[0]));
	TRACE(("SysInfo::getCpId(), Dual Sided cp2 <%d>", s_cpId[1]));

	cpId[0] = s_cpId[0];
	cpId[1] = s_cpId[1];

	TRACE(("SysInfo::getCpId() return"));
}

void SysInfo::getSysInfo(int& architecture, int mauType[])
{
	TRACE(("SysInfo::getSysInfo()"));
	architecture = -1;
	mauType[0] = mauType[1] = -1;

	int arch = MAUFunx::getNodeArchitecture();
	if (arch == ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED)
	{
		// In CEE, mauType concept does not exist in CS.
		// It is set to MAUS in SW MAU
		TRACE(("SysInfo::getSysInfo(), CEE detected, no need to get mauType information"));
		architecture = arch;
		mauType[0] = mauType[1] = 2;
		return;
	}

	int type = MAUFunx::getMauType(1);
	if (type == -1)
	{
		TRACE(("SysInfo::getSysInfo() failed to get mauType for CP1"));
		// EAH logging
	}
	mauType[0] = type;
	type = MAUFunx::getMauType(2);
	if (type == -1)
	{
		TRACE(("SysInfo::getSysInfo() failed to get mauType for CP2"));
		// EAH logging
	}
	mauType[1] = type;

	TRACE(("SysInfo::getSysInfo() returns"));
}
