/*
 * DmxGepInfo.h
 *
 *  Created on: Feb 25, 2016
 *      Author: xdtthng
 */

#ifndef DMXGEPINFO_H_
#define DMXGEPINFO_H_

#include "GepInfo.h"

class DmxGepInfo: public GepInfo
{
public:
	DmxGepInfo();
	virtual ~DmxGepInfo();

	virtual int getGepVersion(uint16_t sysId, uint16_t side);


private:

	static std::string	s_OID_bladeProductName;
	std::string buildSnmpGetCmd(const Board& board, int side = 0);

};


#endif /* DMXGEPINFO_H_ */
