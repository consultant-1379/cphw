/*
 * GepFunx.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: xdtthng
 */

#include "DmxGepInfo.h"
#include "ScbGepInfo.h"
#include "ScxGepInfo.h"
#include "SmxGepInfo.h"
#include "GepFunx.h"
#include "AppTrace.h"

#include "ACS_CS_API.h"

#include <iostream>

using namespace std;

namespace GepFunx
{
	GepInfo* s_gepInfo = 0;
};

int GepFunx::getGepVersion(int& gepVer, int cpId, int side)
{
	TRACE(("GepFunx::getGepVersion()"));
	s_gepInfo = 0;
	gepVer = -1;
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue infra;
	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);

        TRACE(("GepFunx::getGepVersion(). Infrastructure: %d", infra));

	if (res != ACS_CS_API_NS::Result_Success)
	{
		//cout << "Cannot get node architecture" << endl;
		return FailedToGetNodeArchitecture;
		//return aGep;
	}

	if (int(infra) > 4)
	{
		return NodeArchitectureNotSupported;
		//return aGep;
	}
      
	GepInfo* aGep = 0;
	switch (infra)
	{
	case GepInfo::SCB:
		aGep = new ScbGepInfo;
		break;

	case GepInfo::SCX:
		aGep = new ScxGepInfo;
		break;

	case GepInfo::DMX:
		aGep = new DmxGepInfo;
		break;

	case GepInfo::SMX:
                aGep = new SmxGepInfo;
                break;

        default:
		break;
	}

	if (aGep)
	{
		s_gepInfo = aGep;
		gepVer = aGep->getGepVersion(cpId, side);
	}

	//delete aGep;
	//aGep = 0;
	TRACE(("GepFunx::getGepVersion() returns gepVer <%d>", gepVer));
	return gepVer != -1 ? Success : FailedToGetGepVersion;
	//return aGep;
}

