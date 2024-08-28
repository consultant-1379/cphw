/*
 * SmxGepInfo.h
 *
 *  Created on: Feb 5, 2018
 *      Author: xtuangu
 */

#ifndef SMXGEPINFO_H_
#define SMXGEPINFO_H_

#include "GepInfo.h"


struct SmxBoard
{
	uint32_t	m_magazine;
	uint16_t	m_slot;
	std::string	m_ipAddr;
	std::string	m_ipAddrSec;
	uint16_t	m_status;			// 0 == passive, 1 == active

	void set(uint32_t, uint16_t, const std::string&, const std::string&, uint16_t);
};

std::ostream& operator<<(std::ostream& os, const SmxBoard& board);

class SmxGepInfo: public GepInfo
{
public:
	SmxGepInfo();
	virtual ~SmxGepInfo();

	virtual int getGepVersion(uint16_t sysId, uint16_t side);

private:
	std::vector<SmxBoard>	m_scxBoards;

	void readSmxBoard();
	std::string buildSnmpGetCmd(const SmxBoard& master, int slot, int side = 0);
	bool getSmxMaster(SmxBoard& board, uint32_t mag);

	static std::string	s_OID_bladeProductName;

};


#endif /* SMXGEPINFO_H_ */
