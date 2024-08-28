/*
 * SmxGepInfo.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: xdtthng
 */

#include <iostream>
#include <ostream>
#include <iomanip>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "ACS_CS_API.h"
#include "SmxGepInfo.h"
#include "AppTrace.h"

using namespace std;

string SmxGepInfo::s_OID_bladeProductName = "1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.4";

void SmxBoard::set(uint32_t m , uint16_t s, const string& ip1, const string& ip2, uint16_t status)
{
	m_magazine = m;
	m_slot = s;
	m_ipAddr = ip1;
	m_ipAddrSec = ip2;
	m_status = status;
}

ostream& operator<<(std::ostream& os, const SmxBoard& board)
{

	os	<< "magazine " << hex << setw(8) << board.m_magazine << dec
		<< " slotnumber " << setw(2) << board.m_slot
		<< " IP Addr " << board.m_ipAddr
		<< " IP Addr Sec " << board.m_ipAddrSec
		<< " status  " << setw(4) << board.m_status;

	return os;
}


SmxGepInfo::SmxGepInfo(): GepInfo(), m_scxBoards()
{
	//readHWTable();
}

SmxGepInfo::~SmxGepInfo()
{

}

bool SmxGepInfo::getSmxMaster(SmxBoard& board, uint32_t mag)
{

	//cout << "SmxGepInfo::getSmxMaster() m_fatal " << m_fatal << endl;
	//cout << "search for mag " << hex << mag << dec << endl;

	if (m_fatal)
		return !m_fatal;

	vector<SmxBoard>::iterator it = find_if(m_scxBoards.begin(), m_scxBoards.end(),
			[mag](const SmxBoard& b) { return b.m_magazine == mag && b.m_status == 1; });

	return (it != m_scxBoards.end())? (board = *it, true) : (false);
}



int SmxGepInfo::getGepVersion(uint16_t sysId, uint16_t side)
{
	TRACE(("SmxGepInfo::getGepVersion() sysId <%d> side <%d>", sysId, side));
	if (m_fatal)
	{
		cout << "this->isGood() returns " << this->isGood() << endl;
		TRACE(("Fatal error"));
		return -1;
	}

	this->readSmxBoard();

	// Read from HW table the board
	Board board;
	if (!getBoard(board, sysId, side))
	{
		cout << "Failed to get hw table" << endl;
		TRACE(("Failed to get hw table"));
		return -1;
	}

	// Read the SMX master
	SmxBoard master;
	if (!getSmxMaster(master, board.m_magazine))
	{
		cout << "Failed to get SMX master" << endl;
		TRACE(("Failed to get SMX master"));
		return -1;
	}

	/*

	// Build command
	string snmpget = "snmpget -v 2c -c NETMAN ";
	string oid = " -v 2c -c NETMAN ";
	oid += s_OID_bladeProductName;
	oid += '.';
	oid += dynamic_cast<ostringstream &>(ostringstream() << board.m_slotNumber).str();

	string cmd = snmpget;
	cmd += master.m_ipAddr;
	cmd += oid;

	*/


	int gepNo = -1;
	//boost::regex regExpr("SNMPv2-SMI::enterprises.+?=\\s+?Hex-STRING:(.*)");
        boost::regex regExpr("SNMPv2-SMI::enterprises.+?=\\s+?STRING:\\s+?.+?(GEP.+?)\\s+?.*");
        boost::regex regExprAny("SNMPv2-SMI::enterprises.+?");

	boost::regex regExprNotFound("SNMPv2-SMI::enterprises.+?=\\s+?No Such Instance currently exists");
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> sep(" ");
	for (int tside = 0; tside < 2; ++ tside)
	{
		// Execute command and parse results
		string cmd = this->buildSnmpGetCmd(master, board.m_slotNumber, tside);
		vector<string> gepstr;
		this->execCmd(cmd, gepstr);
		//copy(gepstr.begin(), gepstr.end(), ostream_iterator<string>(cout, "\n"));

		// Try to extract GEP number
		boost::smatch m;
		if (!boost::regex_match(gepstr[0], m, regExpr))
		{
			TRACE(("Failed to get correct snmp version string"));
			if (boost::regex_match(gepstr[0], m, regExprNotFound))
			{
				TRACE(("Failed to get correct snmp version, snmp not contactable"));
				TRACE(("%s", gepstr[0].c_str()));
				gepNo = -1;
				break;
			}
			continue;
		}
		else
		{
			TRACE(("Get correct snmp version string"));
			TRACE(("%s", gepstr[0].c_str()));
		}

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
	m_fatal = gepNo == -1? true : false;
	return gepNo;
}

string SmxGepInfo::buildSnmpGetCmd(const SmxBoard& master, int slot, int side)
{
	namespace bip = boost::asio::ip;
	string cmd("");
	do
	{
		// Build command
		string snmpget = "snmpget -v 2c -c NETMAN ";
		string oid = " ";

                oid += s_OID_bladeProductName;
                oid += '.';
                oid += to_string(slot);

		cmd = snmpget;
		cmd += side == 0 ? master.m_ipAddr : master.m_ipAddrSec;
		cmd += oid;

	}
	while (false);
	return cmd;
}


void SmxGepInfo::readSmxBoard()
{
	const string cmd = "hwmxls";
	const boost::regex regExpr("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}).+?"
			"(\\d{1,3}).+?SMXB.+?"			// slot number
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\w+?)\\s+?(.+?)"
			);

	vector<string> lines;
	execCmd(cmd, lines);
	SmxBoard board;
	uint32_t magNo;
	uint16_t slot;

	boost::smatch m;
	for (unsigned int i = 0; i < lines.size(); ++i)
	{
		if (boost::regex_match(lines[i], m, regExpr))
		{
			//cout << "matched ";
			if (m[1].matched && m[2].matched && m[3].matched && m[4].matched && m[4].matched)
			{
		        magNo = magNumToUInt(m[1].str());
		        slot = atoi(m[2].str().c_str());
		        board.set(magNo, slot, m[3].str(), m[4].str(), (m[5].str() == "master"?1:0));
		        m_scxBoards.push_back(board);
			}
		}
	}
	//cout << "the whole SmxBoards is" << endl;
	//copy(m_scxBoards.begin(), m_scxBoards.end(), ostream_iterator<SmxBoard>(cout, "\n"));
}

