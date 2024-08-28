/*
 * GepInfo.h
 *
 *  Created on: Feb 25, 2016
 *      Author: xdtthng
 */

#ifndef GEPINFO_H_
#define GEPINFO_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <ostream>

#include <map>
#include <boost/tuple/tuple.hpp>

struct Board
{
	uint32_t m_magazine;
	uint16_t m_index;
	uint32_t m_bgciIpEthA;
	uint32_t m_bgciIpEthB;

	uint32_t m_ipEthA;
	uint32_t m_ipEthB;

	uint16_t m_slotNumber;
	uint16_t m_sysId;			// CpId
	uint16_t m_side;
	uint16_t m_sysType;

	uint32_t m_solIpA;
	uint32_t m_solIpB;

	void set(uint32_t magazine, uint32_t bgciIpEthA, uint32_t bgciIpEthB, uint32_t ipa, uint32_t ipb,
			uint16_t slotNumber, uint16_t sysId, uint16_t side, uint16_t sysType, uint32_t solIpA, uint32_t solIpB);
};

std::ostream& operator<<(std::ostream& os, const Board& board);

class GepInfo
{
public:
	enum {SCB = 0, SCX = 1, DMX = 2, VIRTUALIZED = 3, SMX = 4};
	//typedef boost::tuple<uint32_t, uint32_t> IpAddrPairT;
	//typedef boost::tuple<std::string, std::string> IpAddrStrPairT;
	typedef boost::tuple<std::string, std::string> IpAddrPairT;

	GepInfo();
	virtual ~GepInfo();

	virtual int getGepVersion(uint16_t sysId, uint16_t side);
	virtual void printHWTable();

	virtual int getSolIpAddr(std::map<int, IpAddrPairT>& solTable);

protected:
	bool				m_fatal;
	std::vector<Board>	m_boards;

	// Leave this here for now
	// Move when needed elsewhere
	virtual int execCmd(const std::string& cmd, std::vector<std::string>& result);
	virtual bool isGood();

	virtual bool readHWTable();
	virtual void sortByMagazine();
	virtual bool cmpMagazine(const Board& b1, const Board& b2);
	virtual bool getBoard(Board&, uint16_t sysId, uint16_t side = 0);
	virtual uint32_t magNumToUInt(const std::string&);
	virtual uint32_t str_toIpAddress(const std::string&);

private:

};



#endif /* GEPINFO_H_ */
