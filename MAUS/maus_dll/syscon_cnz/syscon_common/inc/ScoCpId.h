/*
 * ScoCpId.h
 *
 *  Created on: Nov 5, 2015
 *      Author: xdtthng
 */

#ifndef SCOCPID_H_
#define SCOCPID_H_

namespace ScoCpId
{
	int makeCpId(int cpid, int side);
};

inline
int ScoCpId::makeCpId(int cpid, int side)
{
	return (side << 16) | cpid;
}

#endif /* SCOCPID_H_ */
