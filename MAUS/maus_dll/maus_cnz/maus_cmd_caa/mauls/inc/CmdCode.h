/*
 * CmdCode.h
 *
 *  Created on: Sep 10, 2014
 *      Author: xdtthng
 */

#ifndef CMDCODE_H_
#define CMDCODE_H_

#include <string>
#include <map>

enum EpCodeT
{
	apa	= 1,
	apb	= 2,
	cps = 4
};

typedef std::map<std::string, EpCodeT>	EpTableT;
typedef EpTableT::iterator	EpTableIter;


#endif /* CMDCODE_H_ */
