/*
 * DmxGepInfo.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: xdtthng
 */

#include <iostream>
#include <ostream>
#include <iomanip>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/lambda/lambda.hpp> // For lambda expressions
#include <boost/lambda/bind.hpp>   // For bind expressions
#include <boost/regex.hpp>

#include "ACS_CS_API.h"
#include "AppTrace.h"
#include "ScoPinger.h"
//#include <acs_apgcc_paramhandling.h>
//#include <ACS_APGCC_CommonLib.h>

#include "DmxGepInfo.h"

using namespace std;

string DmxGepInfo::s_OID_bladeProductName = "1.3.6.1.4.1.193.177.2.2.8.2.2.1.1.7";

DmxGepInfo::~DmxGepInfo()
{
}

DmxGepInfo::DmxGepInfo(): GepInfo()

{
	//readHWTable();
}


string DmxGepInfo::buildSnmpGetCmd(const Board& board, int side)
{
	namespace bip = boost::asio::ip;
	string cmdline("");
	do
	{
		boost::system::error_code ec;
		string ipaddrStr;
		uint32_t ipaddr;

		ipaddr = (side == 0)? board.m_bgciIpEthA : board.m_bgciIpEthB;
		ipaddrStr = bip::address_v4(ipaddr).to_string(ec);
		if (ec)
			break;

		//boost::asio::ip::address_v4 ipab(aBoard.m_bgciIpEthA);
		//string astr = ipab.to_string();

		//cout << hex << ipaddr << dec << " " << ipaddrStr << endl;

		ipaddr &= 0xffffff00;
		ipaddr |= 1;
		ipaddrStr = bip::address_v4(ipaddr).to_string(ec);
		if (ec)
			break;

		//cout << hex << ipaddr << dec << " " << ipaddrStr << endl;

		string snmpget = "snmpget -v 2c -c dmx-community ";

		string data = DmxGepInfo::s_OID_bladeProductName;
		data += '.';
		//data += dynamic_cast<ostringstream &>(ostringstream() << board.m_index).str();
		data += to_string(board.m_magazine); //TR_HZ52661
		data += '.';
		//data += dynamic_cast<ostringstream &>(ostringstream() << board.m_slotNumber).str();
		data += to_string(board.m_slotNumber);

		cmdline = snmpget;
		cmdline += ipaddrStr;
		cmdline += ' ';
		cmdline += data;

	}
	while (false);

	return cmdline;
}

int DmxGepInfo::getGepVersion(uint16_t sysId, uint16_t side)
{
	boost::regex regExpr("SNMPv2.+?193\\.177\\.2\\.2\\.8\\.2\\.2\\.1\\.1\\.7.*?"
			"\\s+?=\\s+?STRING:\\s+?\"GEP(\\d{1,}).*?");

	boost::regex regExprAny("SNMPv2.+?193\\.177\\.2\\.2\\.8\\.2\\.2\\.1\\.1\\.7.*?"
			"\\s+?=\\s+?STRING:.*");

	TRACE(("DmxGepInfo::getGepVersion() sysId <%d>, side <%d>", sysId, side));

	if (m_fatal)
	{
		//cout << "this->isGood() returns " << this->isGood() << endl;
		TRACE(("DmxGepInfo::getGepVersion() returns Fatal error"));
		return -1;
	}

	this->sortByMagazine();

	Board aBoard;
	if (!this->getBoard(aBoard, sysId, side))
	{
		//cout << "Cannot find sysId " << sysId << " with " << side << endl;
		TRACE(("DmxGepInfo::getGepVersion() returns; Cannot find sysId <%d> with side <%d>", sysId, side));
		m_fatal = true;
		return -1;
	}

	vector<string> gepstr;
	ScoPinger echo;
	int gepNo = -1;
	for (int tside = 0; tside < 2; ++tside)
	//for (int idx = 0; idx < 2; ++idx)
	{

		string cmdline = this->buildSnmpGetCmd(aBoard, tside);
		this->execCmd(cmdline, gepstr);
		//cout << "size of gepstr " << gepstr.size() << endl;
		//copy(gepstr.begin(), gepstr.end(), ostream_iterator<string>(cout, "\n"));

		boost::smatch m;

		if (boost::regex_match(gepstr[0], m, regExpr))
		{
			if (m[1].matched)
			{
				//cout << m[1].str() << endl;
				gepNo = atoi(m[1].str().c_str());
				break;
			}

		}
		else if (boost::regex_match(gepstr[0], m, regExprAny))
		{
			// Some different version
			gepNo = 1;
			break;
		}
		else
		{
			TRACE(("GEP5 string not found"));
		}
		vector<string>().swap(gepstr);
	}
	m_fatal = gepNo == -1? true : false;
	//cout << "gepNo is " << gepNo << endl;

	TRACE(("DmxGepInfo::getGepVersion() returns gepNo <%d>", gepNo));
	return gepNo;
}
