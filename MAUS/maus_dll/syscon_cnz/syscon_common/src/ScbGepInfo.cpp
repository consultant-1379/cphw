/*
 * ScbGepInfo.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: xdtthng
 */

#include <iostream>
#include <ostream>
#include <iomanip>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/regex.hpp>

#include "ACS_CS_API.h"
#include "ScbGepInfo.h"
#include "AppTrace.h"

using namespace std;

string ScbGepInfo::s_OID_bladeProductName = "1.3.6.1.4.1.193.154.2.1.2.1.1.1.5";

void ScbMaster::set(uint16_t s, uint32_t ip1, uint32_t ip2, uint16_t status)
{
	m_slotNumber = s;
	m_ipAddr = ip1;
	m_ipAddrSec = ip2;
	m_status = status;
	m_magazine = 0xffff;
}

ostream& operator<<(std::ostream& os, const ScbMaster& board)
{
	namespace bip = boost::asio::ip;

	boost::system::error_code ec;
	os << "slot " << dec << setw(4) << board.m_slotNumber << dec
			<< " ipEthA " << bip::address_v4(board.m_ipAddr).to_string(ec)
			<< " ipEthB " << bip::address_v4(board.m_ipAddrSec).to_string(ec)
			<< " status " << setw(2) << board.m_status
			<< " mag " << hex << setw(6) << board.m_magazine << dec
			;

	return os;
}


void ScbBoard::set(uint32_t m, uint32_t ip1, uint32_t ip2, uint16_t s)
{
	m_magazine = m;
	m_ipAddr = ip1;
	m_ipAddrSec = ip2;
	m_slotNumber = s;
}

ostream& operator<<(std::ostream& os, const ScbBoard& board)
{
	namespace bip = boost::asio::ip;

	boost::system::error_code ec;
	os << "slot " << dec << setw(4) << board.m_slotNumber << dec
			<< " magazine " << hex << setw(8) << board.m_magazine << dec
			<< " ipEthA " << bip::address_v4(board.m_ipAddr).to_string(ec)
			<< " ipEthB " << bip::address_v4(board.m_ipAddrSec).to_string(ec)
			//<< " slotnumber " << setw(2) << board.m_slotNumber
			//<< " sysType " << setw(4) << board.m_sysType
			//<< " sysId " << setw(4) << board.m_sysId
			//<< " side " << setw(4) << board.m_side
			;

	return os;
}


ScbGepInfo::ScbGepInfo(): m_scbMaster(), m_scbBoard()
{
	//readHWTable();
}

ScbGepInfo::~ScbGepInfo()
{

}


bool ScbGepInfo::readScbMasterIpAddr(uint32_t mag, uint32_t& ipa, uint32_t& ipb)
{
	if (m_fatal)
		return !m_fatal;

	vector<ScbMaster>::iterator it = find_if(m_scbMaster.begin(), m_scbMaster.end(),
			[mag](const ScbMaster& b) { return b.m_magazine == mag; });

	return (it != m_scbMaster.end())? (ipa = it->m_ipAddr, ipb = it->m_ipAddrSec, true) : (false);

}


void ScbGepInfo::updateScbMaster()
{
	// For each entry in m_scbMaster
	// Look for it in m_scbBoard
	// If found, update the entry in m_scbMaster with magazine number in m_scbBoard

	std::vector<ScbMaster>::iterator mit;
	std::vector<ScbBoard>::iterator bit;

	for (mit = m_scbMaster.begin(); mit != m_scbMaster.end(); ++mit)
	{
		for(bit = m_scbBoard.begin(); bit != m_scbBoard.end(); ++bit)
		{
			if (mit->m_ipAddr == bit->m_ipAddr && mit->m_ipAddrSec == bit->m_ipAddrSec)
			{
				mit->m_magazine = bit->m_magazine;
			}
		}
	}
	//copy(m_scbMaster.begin(), m_scbMaster.end(), ostream_iterator<ScbMaster>(cout, "\n"));
}

bool ScbGepInfo::readScbMaster()
{
	const string cmd = "hwmscbls";
	const boost::regex regExpr("(\\d+?)\\s+?"
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\w+?)\\s+?(.+?)"
			);

	vector<string> lines;
	execCmd(cmd, lines);
	//copy(lines.begin(), lines.end(), ostream_iterator<string>(cout, ""));

	uint16_t slot;
	uint32_t ipAddr;
	uint32_t ipAddrSec;
	uint16_t status;			// 0 == passive, 1 == active
	ScbMaster aMaster;
	boost::smatch m;
	for (unsigned int i = 0; i < lines.size(); ++i)
	{
		if (boost::regex_match(lines[i], m, regExpr))
		{
			if (m[1].matched && m[2].matched && m[3].matched && m[4].matched && m[4].matched)
			{
				status = m[4].str() == "master"? 1 : 0;
				if (status)
				{
					slot = strtoul(m[1].str().c_str() , NULL, 10);
					ipAddr = str_toIpAddress(m[2].str());
					ipAddrSec = str_toIpAddress(m[3].str());
					aMaster.set(slot, ipAddr, ipAddrSec, status);
					m_scbMaster.push_back(aMaster);
				}
			}
		}
	}

	//copy(m_scbMaster.begin(), m_scbMaster.end(), ostream_iterator<ScbMaster>(cout, "\n"));
	return true;
}

bool ScbGepInfo::readScbBoard()
{

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
		boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCBRP);

		ACS_CS_API_IdList boardList;
		if (hwcTable->getBoardIds(boardList, boardSearch) != ACS_CS_API_NS::Result_Success)
		{
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
			ACS_CS_API::deleteHWCInstance(hwcTable);
			m_fatal = true;
			break;
		}

		uint32_t magazine;
		uint32_t ipEthA;
		uint32_t ipEthB;
		uint16_t slotNumber;

		//cout << "GepInfo::readScbBoard() boardList.size() " << boardList.size() << endl;
		m_fatal = false;
		for (unsigned int i = 0; i < boardList.size(); ++i)
		{
			if (hwcTable->getMagazine(magazine, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getMagazine()" << endl;
				TRACE(("Fatal error when getMagazine()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getIPEthA(ipEthA, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getBgciIPEthA()" << endl;
				TRACE(("Fatal error when getBgciIPEthA()"));
				m_fatal = true;
				break;
			}
			if (hwcTable->getIPEthB(ipEthB, boardList[i]) != ACS_CS_API_NS::Result_Success)
			{
				//cout << "Fatal error when getBgciIPEthB()" << endl;
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

			ScbBoard aBoard;
			memset(&aBoard, 0, sizeof(ScbBoard));
			aBoard.set(magazine, ipEthA, ipEthB, slotNumber);
			m_scbBoard.push_back(aBoard);
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

	//copy(m_scbBoard.begin(), m_scbBoard.end(), ostream_iterator<ScbBoard>(cout, "\n"));
	return !m_fatal;

}

int ScbGepInfo::getGepVersion(uint16_t sysId, uint16_t side)
{
	if (m_fatal)
	{
		//cout << "this->isGood() returns " << this->isGood() << endl;
		TRACE(("Fatal error"));
		return -1;
	}


	readScbMaster();
	readScbBoard();
	updateScbMaster();

	// Find a board with sysId and side
	Board aBoard;
	if (!getBoard(aBoard, sysId, side))
	{
		TRACE(("Fatal error"));
		m_fatal = true;
		return -1;
	}

	uint32_t ipm[2];
	readScbMasterIpAddr(aBoard.m_magazine, ipm[0], ipm[1]);

	//boost::regex regExpr(".+?=\\s+?STRING:\\s+?.+?(GEP.+?)\\s+?.*");
	boost::regex regExpr("SNMPv2-SMI::enterprises.+?=\\s+?STRING:\\s+?.+?(GEP.+?)\\s+?.*");
	boost::regex regExprAny("SNMPv2-SMI::enterprises.+?");

	int gepNo = -1;
	for (int tside = 0; tside < 2; ++tside)
	{
		string cmd = this->buildSnmpGetCmd(ipm[tside], aBoard.m_slotNumber);
		//cout << "<" << cmd << ">" << endl;
		vector<string> gepstr;
		this->execCmd(cmd, gepstr);
		//copy(gepstr.begin(), gepstr.end(), ostream_iterator<string>(cout, "\n"));
		boost::smatch m;
		if (boost::regex_match(gepstr[0], m, regExpr))
		{
			if (m[1].matched)
			{
				TRACE(("GEP Version <%s>", m[1].str().c_str()));
				gepNo = m[1].str()[3] - '0';
				break;
			}
			else if (boost::regex_match(gepstr[0], m, regExprAny))
			{
				// Some different version
				gepNo = 1;
				break;
			}
		}
	}

	return gepNo;

}

string ScbGepInfo::buildSnmpGetCmd(const uint32_t ipm, int slot)
{
	namespace bip = boost::asio::ip;

	string cmdline("");
	do
	{
		// bip::address_v4(board.m_ipAddr).to_string(ec)

		cmdline = "snmpget -v 2c -c public ";
		boost::system::error_code ec;
		cmdline += bip::address_v4(ipm).to_string(ec);
		cmdline += ' ';
		string oid = s_OID_bladeProductName;
		oid += '.';
		oid += dynamic_cast<ostringstream &>(ostringstream() << slot).str();
		cmdline += oid;
	}
	while (false);
	return cmdline;
}
