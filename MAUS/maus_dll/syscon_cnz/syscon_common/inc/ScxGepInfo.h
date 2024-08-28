/*
 * ScxGepInfo.h
 *
 *  Created on: Mar 3, 2016
 *      Author: xdtthng
 */

#ifndef SCXGEPINFO_H_
#define SCXGEPINFO_H_

#include "GepInfo.h"


struct ScxBoard
{
	uint32_t	m_magazine;
	uint16_t	m_slot;
	std::string	m_ipAddr;
	std::string	m_ipAddrSec;
	uint16_t	m_status;			// 0 == passive, 1 == active

	void set(uint32_t, uint16_t, const std::string&, const std::string&, uint16_t);
};

std::ostream& operator<<(std::ostream& os, const ScxBoard& board);

class ScxGepInfo: public GepInfo
{
public:
	ScxGepInfo();
	virtual ~ScxGepInfo();

	virtual int getGepVersion(uint16_t sysId, uint16_t side);

private:
	std::vector<ScxBoard>	m_scxBoards;

	void readScxBoard();
	std::string buildSnmpGetCmd(const ScxBoard& master, int slot, int side = 0);
	bool getScxMaster(ScxBoard& board, uint32_t mag);

	static std::string	s_OID_bladeProductName;

};


#endif /* SCXGEPINFO_H_ */
