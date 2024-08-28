/*
 * ScxGepInfo.cpp
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
#include "ScxGepInfo.h"
#include "AppTrace.h"

using namespace std;

string ScxGepInfo::s_OID_bladeProductName = "1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.4";

void ScxBoard::set(uint32_t m , uint16_t s, const string& ip1, const string& ip2, uint16_t status)
{
	m_magazine = m;
	m_slot = s;
	m_ipAddr = ip1;
	m_ipAddrSec = ip2;
	m_status = status;
}

ostream& operator<<(std::ostream& os, const ScxBoard& board)
{

	os	<< "magazine " << hex << setw(8) << board.m_magazine << dec
		<< " slotnumber " << setw(2) << board.m_slot
		<< " IP Addr " << board.m_ipAddr
		<< " IP Addr Sec " << board.m_ipAddrSec
		<< " status  " << setw(4) << board.m_status;

	return os;
}


ScxGepInfo::ScxGepInfo(): GepInfo(), m_scxBoards()
{
	//readHWTable();
}

ScxGepInfo::~ScxGepInfo()
{

}

bool ScxGepInfo::getScxMaster(ScxBoard& board, uint32_t mag)
{

	//cout << "ScxGepInfo::getScxMaster() m_fatal " << m_fatal << endl;
	//cout << "search for mag " << hex << mag << dec << endl;

	if (m_fatal)
		return !m_fatal;

	vector<ScxBoard>::iterator it = find_if(m_scxBoards.begin(), m_scxBoards.end(),
			[mag](const ScxBoard& b) { return b.m_magazine == mag && b.m_status == 1; });

	return (it != m_scxBoards.end())? (board = *it, true) : (false);
}



int ScxGepInfo::getGepVersion(uint16_t sysId, uint16_t side)
{
	TRACE(("ScxGepInfo::getGepVersion() sysId <%d> side <%d>", sysId, side));
	if (m_fatal)
	{
		//cout << "this->isGood() returns " << this->isGood() << endl;
		TRACE(("Fatal error"));
		return -1;
	}

	this->readScxBoard();

	// Read from HW table the board
	Board board;
	if (!getBoard(board, sysId, side))
	{
		//cout << "Failed to get hw table" << endl;
		TRACE(("Failed to get hw table"));
		return -1;
	}

	// Read the SCX master
	ScxBoard master;
	if (!getScxMaster(master, board.m_magazine))
	{
		//cout << "Failed to get SCX master" << endl;
		TRACE(("Failed to get SCX master"));
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
	boost::regex regExpr("SNMPv2-SMI::enterprises.+?=\\s+?Hex-STRING:(.*)");
	//boost::regex regExprAny("SNMPv2-SMI::enterprises.+?");

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

		Tokenizer tok(m[1].str(), sep);
		string gep("");
		unsigned long val;
		for (Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
		{
			//cout << *it << '\n';
			val = strtoul(it->c_str() , NULL, 16);
			gep += char(val);
			if (!val)
				break;
		}

		//cout << gep << endl;
		gepNo = gep[3] - '0';
		break;
	}
	m_fatal = gepNo == -1? true : false;
	return gepNo;
}

string ScxGepInfo::buildSnmpGetCmd(const ScxBoard& master, int slot, int side)
{
	namespace bip = boost::asio::ip;
	string cmd("");
	do
	{
		// Build command
		string snmpget = "snmpget -v 2c -c NETMAN ";
		string oid = " -v 2c -c NETMAN ";
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


void ScxGepInfo::readScxBoard()
{
	const string cmd = "hwmxls";
	const boost::regex regExpr("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}).+?"
			"(\\d{1,3}).+?SCXB.+?"			// slot number
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\w+?)\\s+?(.+?)"
			);

	vector<string> lines;
	execCmd(cmd, lines);
	ScxBoard board;
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
	//cout << "the whole ScxBoards is" << endl;
	//copy(m_scxBoards.begin(), m_scxBoards.end(), ostream_iterator<ScxBoard>(cout, "\n"));
}

