/*
 * ScbGepInfo.h
 *
 *  Created on: Mar 1, 2016
 *      Author: xdtthng
 */

#ifndef SCBGEPINFO_H_
#define SCBGEPINFO_H_

#include "GepInfo.h"

struct ScbMaster
{
	uint16_t		m_slotNumber;
	uint32_t		m_ipAddr;
	uint32_t		m_ipAddrSec;
	uint16_t		m_status;			// 0 == passive, 1 == active
	uint32_t		m_magazine;

	void set(uint16_t, uint32_t, uint32_t, uint16_t);
};

std::ostream& operator<<(std::ostream& os, const ScbMaster& board);

struct ScbBoard
{
	uint16_t 		m_slotNumber;
	uint32_t		m_magazine;
	uint32_t		m_ipAddr;
	uint32_t		m_ipAddrSec;

	void set(uint32_t, uint32_t, uint32_t, uint16_t);
};

std::ostream& operator<<(std::ostream& os, const ScbBoard& board);

class ScbGepInfo: public GepInfo
{
public:
	ScbGepInfo();
	virtual ~ScbGepInfo();

	virtual int getGepVersion(uint16_t sysId, uint16_t side);

private:
	// Master SCBRP
	std::vector<ScbMaster>	m_scbMaster;
	std::vector<ScbBoard>	m_scbBoard;

	bool readScbMaster();
	bool readScbBoard();
	void updateScbMaster();
	bool readScbMasterIpAddr(uint32_t mag, uint32_t& ipa, uint32_t& ipb);
	std::string buildSnmpGetCmd(const uint32_t ipm, int slot);

	static std::string	s_OID_bladeProductName;

};


#endif /* SCBGEPINFO_H_ */
