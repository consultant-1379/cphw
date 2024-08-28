/*
 * GepInfo.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: xdtthng
 */

#include <iostream>
#include <iomanip>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>


#include "ACS_CS_API.h"
#include "GepInfo.h"
#include "AppTrace.h"
#include "CmdFunx.h"
#include "Funx.h"

using namespace std;

ostream& operator<<(std::ostream& os, const Board& board)
{
	namespace bip = boost::asio::ip;

	boost::system::error_code ec;
	os << "index: " << board.m_index
			<< " mag " << hex << setw(8) << board.m_magazine << dec
			<< " bgciA " << bip::address_v4(board.m_bgciIpEthA).to_string(ec)
			<< " bgciB " << bip::address_v4(board.m_bgciIpEthB).to_string(ec)
			<< " ipA " << bip::address_v4(board.m_ipEthA).to_string(ec)
			<< " ipB " << bip::address_v4(board.m_ipEthB).to_string(ec)
			<< " slot " << setw(2) << board.m_slotNumber
			<< " sysType " << setw(4) << board.m_sysType
			<< " sysId " << setw(4) << board.m_sysId
			<< " side " << setw(2) << board.m_side
			<< " SolA " << bip::address_v4(board.m_solIpA).to_string(ec)
			<< " SolB " << bip::address_v4(board.m_solIpB).to_string(ec)
			;


	return os;
}

void Board::set(uint32_t magazine, uint32_t bgciIpEthA, uint32_t bgciIpEthB,
		uint32_t ipa, uint32_t ipb,
		uint16_t slotNumber, uint16_t sysId, uint16_t side, uint16_t sysType, uint32_t solIpA, uint32_t solIpB)
{

	m_magazine = magazine;
	m_bgciIpEthA = bgciIpEthA;
	m_bgciIpEthB = bgciIpEthB;
	m_ipEthA = ipa;
	m_ipEthB = ipb;
	m_slotNumber = slotNumber;

	m_sysId = sysId;			// CpId
	m_side = side;
	m_sysType = sysType;

	m_solIpA = solIpA;
	m_solIpB = solIpB;

	m_index = 0;	// Only use this for Dmx
}



GepInfo::GepInfo(): m_fatal(false), m_boards()
{
	readHWTable();
}


GepInfo::~GepInfo()
{

}

bool GepInfo::isGood()
{
	return !m_fatal;
}

int GepInfo::getGepVersion(uint16_t, uint16_t)
{
	// Return an invalid version
	return -1;
}

int GepInfo::execCmd(const string& cmd, vector<string>& result)
{

#if 1

	return CmdFunx::execCmd(cmd, result);

#else

	static const int SIZE = 255;

    FILE *pf;
    char data[SIZE];

    pf = popen((cmd + " 2>&1").c_str(), "r");
    //pf = popen(cmd.c_str(), "r");
    if (!pf) {
      return -1;
    }

    int count = 0;
    while (!feof(pf)) {
        if (fgets(data, SIZE , pf)) {
        	result.push_back(data);
        }
        ++count;
    }

    // Might fail! What can we do at this point??
    pclose(pf);

	return count;

#endif

}


bool GepInfo::readHWTable()
{
	//cout << "GepInfo::readHWTable()" << endl;

	do
	{
		ACS_CS_API_HWC* hwcTable = ACS_CS_API::createHWCInstance();
		if (!hwcTable)
		{
			m_fatal = true;
			break;
		}
		ACS_CS_API_BoardSearch* boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (!boardSearch)
		{
			ACS_CS_API::deleteHWCInstance(hwcTable);
			m_fatal = true;
			break;
		}
		boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_CPUB);

		ACS_CS_API_IdList boardList;
		if (hwcTable->getBoardIds(boardList, boardSearch) != ACS_CS_API_NS::Result_Success)
		{
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
			ACS_CS_API::deleteHWCInstance(hwcTable);
			m_fatal = true;
			break;
		}

		uint32_t magazine;
		uint32_t bgciIpEthA;
		uint32_t bgciIpEthB;
		uint32_t ipa;
		uint32_t ipb;

		uint16_t slotNumber;
		uint16_t sysId;			// CpId
		uint16_t side;
		uint16_t sysType;
		uint32_t solIpA;
		uint32_t solIpB;
		uint32_t waSolIp;

		//cout << "GepInfo::readHWTable() boardList.size() " << boardList.size() << endl;

		m_fatal = false;
		Board aBoard;
		for (unsigned int i = 0; i < boardList.size(); ++i)
		{
			if (hwcTable->getMagazine(magazine, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getMagazine()" << endl;
				TRACE(("Fatal error when getMagazine()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getBgciIPEthA(bgciIpEthA, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getBgciIPEthA()" << endl;
				TRACE(("Fatal error when getBgciIPEthA()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getBgciIPEthB(bgciIpEthB, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getBgciIPEthB()" << endl;
				TRACE(("Fatal error when getBgciIPEthB()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getIPEthA(ipa, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				cout << "Fatal error when getBgciIPEthA()" << endl;
				TRACE(("Fatal error when getBgciIPEthA()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getIPEthB(ipb, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				cout << "Fatal error when getBgciIPEthB()" << endl;
				TRACE(("Fatal error when getBgciIPEthB()"));
				m_fatal = true;
				break;
			}

			if (hwcTable->getSlot(slotNumber, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getSlot()" << endl;
				TRACE(("Fatal error when getSlot()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getSysId(sysId, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getSysId()" << endl;
				TRACE(("Fatal error when getSysId()"));
				m_fatal = true;
				break;
			}

			if (sysId < 63)
			{
				side = 0;
			}
			else if (hwcTable->getSide(side, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getSide()" << endl;
				TRACE(("Fatal error when getSide()"));
				m_fatal = true;
				break;
			}

			if (hwcTable->getSysType(sysType, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				m_fatal = true;
				break;
			}

			if (hwcTable->getSolIPEthA(solIpA, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				m_fatal = true;
				break;
			}

			if (hwcTable->getSolIPEthB(solIpB, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				m_fatal = true;
				break;
			}

			// This is the workaround for solIpA and solIpB
			// ---------------------------------------------

#if 0

			waSolIp = solIpA & 0xff;
			if (sysId == 1001 || sysId == 1002)
			{
				waSolIp = (175 + sysId - 1001 + side);
			}
			else if (sysId >= 0 && sysId <=44)
			{
				waSolIp = 180 + sysId;
			}
			solIpA &= 0xffffff00;
			solIpB &= 0xffffff00;
			solIpA |= waSolIp;
			solIpB |= waSolIp;

#endif

			memset(&aBoard, 0, sizeof(Board));
			aBoard.set(magazine, bgciIpEthA, bgciIpEthB, ipa, ipb,
					slotNumber, sysId, side, sysType, solIpA, solIpB);
			m_boards.push_back(aBoard);

		}

		ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		ACS_CS_API::deleteHWCInstance(hwcTable);
	}
	while (false);

	if (!m_fatal)
	{
		//cout << "GepInfo::readHWTable() returns with m_boards.size() " << m_boards.size() << endl;
		TRACE(("GepInfo::readHWTable() returns with m_boards.size() <%d>", m_boards.size()));
	}
	else
	{
		//cout << "GepInfo::readHWTable() fatal errror" << endl;
		TRACE(("GepInfo::readHWTable() fatal errror"));
	}

	return !m_fatal;
}

bool GepInfo::getBoard(Board& board, uint16_t sysId, uint16_t side)
{
	TRACE(("GepInfo::getBoard() sysId <%d> side <%d> m_fatal<%d>", sysId, side, m_fatal));

	if (m_fatal)
	{
		TRACE(("GepInfo::getBoard() m_fatal true"));
		return !m_fatal;
	}

	vector<Board>::iterator it = find_if(m_boards.begin(), m_boards.end(),
			[sysId, side](const Board& b) { return b.m_sysId == sysId && b.m_side == side; });

	return (it != m_boards.end())? (board = *it, true) : (false);
}

bool GepInfo::cmpMagazine(const Board& b1, const Board& b2)
{
	return b1.m_magazine < b2.m_magazine;
}

void GepInfo::sortByMagazine()
{
	sort(m_boards.begin(), m_boards.end(), boost::bind(&GepInfo::cmpMagazine, this, _1, _2));
	uint32_t magNo = m_boards[0].m_magazine;
	uint16_t index = 0;
	for (unsigned int i = 0; i < m_boards.size(); ++i)
	{
		if (m_boards[i].m_magazine > magNo)
		{
			m_boards[i].m_index = ++index;
			magNo = m_boards[i].m_magazine;
		}
		else
		{
			m_boards[i].m_index = index;
		}
	}
}

void GepInfo::printHWTable()
{

	cout << "m_boards.size() " << m_boards.size() << endl;

	copy(m_boards.begin(), m_boards.end(), ostream_iterator<Board>(cout, "\n"));
}

/*
uint32_t GepInfo::magNumToUInt(const char* str)
{
    char *token;
    char *save;

    uint32_t magNo = 0;
    int bits = 0;
    token = strtok_r((char*) str, ".", &save);
    unsigned long val = strtoul(token, NULL, 16);
    while (token != NULL) {
        //cout << token << endl;
    	val = strtoul(token, NULL, 16);
    	//cout << val << " ";
    	val <<= bits;
    	magNo |= val;
    	bits += 8;
        token = strtok_r(NULL, ".", &save);
    }
    //cout << " ---- "<< hex << magNo << endl;
    return magNo;
}
*/

uint32_t GepInfo::magNumToUInt(const string& str)
{
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

	boost::char_separator<char> sep(".");
	Tokenizer tok(str, sep);

    uint32_t magNo = 0;
    int bits = 0;
    unsigned long val;
	for (Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
	{
	    val = strtoul(it->c_str() , NULL, 10);
    	val <<= bits;
    	magNo |= val;
    	bits += 8;
	}
	//cout << " ---- "<< hex << magNo << endl;
    return magNo;
}



uint32_t GepInfo::str_toIpAddress(const string& str)
{
	boost::asio::ip::address_v4 ipa = boost::asio::ip::address_v4::from_string(str);
	return ipa.to_ulong();
}

int GepInfo::getSolIpAddr(map<int, IpAddrPairT>& solTable)
{
	using namespace Funx;
	namespace bip = boost::asio::ip;

	if (m_fatal)
		return -1;

	boost::system::error_code ec;
	string solAstr, solBstr;

	for(const Board& boad : m_boards)
	{
		// m_addrStrTable.insert(make_pair(pack32(cpId, side), IpAddrStrPairT(m[4].str(), ipAddr)));
		solAstr = bip::address_v4(boad.m_solIpA).to_string(ec);
		solBstr = bip::address_v4(boad.m_solIpB).to_string(ec);
		solTable.insert(make_pair(pack32(boad.m_sysId, boad.m_side), IpAddrPairT(solAstr, solBstr)));
	}


	return 0;
}
