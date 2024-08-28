/*
 * GepFunx.h
 *
 *  Created on: Mar 3, 2016
 *      Author: xdtthng
 */

#ifndef GEPFUNX_H_
#define GEPFUNX_H_

class GepInfo;

namespace GepFunx
{

	enum GepFunxError
	{
		Success 						= 0,
		FailedToGetNodeArchitecture 	= 1,
		NodeArchitectureNotSupported 	= 2,
		FailedToGetGepVersion			= 3,

	};

	extern GepInfo* s_gepInfo;

	int getGepVersion(int& gepVer, int cpId, int side);
};

#endif /* GEPFUNX_H_ */
