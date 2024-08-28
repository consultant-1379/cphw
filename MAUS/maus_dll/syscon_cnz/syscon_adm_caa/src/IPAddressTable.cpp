/*
 * IPAddressTable.cpp
 *
 *  Created on: Nov 4, 2015
 *      Author: xdtthng
 */

#include <fstream>
#include <cstring>
#include "boost/regex.hpp"

#include "IPAddressTable.h"
#include "Funx.h"
#include "SysFunx.h"
#include "AppTrace.h"

using namespace std;
using namespace Funx;

string	IPAddressTable::s_iniPath = "/data/cphw/data/syscon/";
string	IPAddressTable::s_iniName = "syscon_ipaddress.ini";

IPAddressTable::~IPAddressTable()
{

}

IPAddressTable::IPAddressTable(): m_addrTable()
{
	TRACE(("IPAddressTable::IPAddressTable()"));

	// These are temporary solutions
	// -----------------------------
	//m_addrTable.insert(make_pair(makeCpId(1002, 0), "192.168.169.200"));
	//m_addrTable.insert(make_pair(makeCpId(1002, 1), "192.168.169.201"));

	// matching line such as
	// cp1 0 192.168.169.200
	// cp1 1 192.168.169.201
	//boost::regex regExpr("\\s*?(cp|bc)(\\d{1,2})\\s+?(\\d)\\s+?(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s*");

	boost::regex regExpr("\\s*?(cp|bc)(\\d{1,2})\\s+?"
			"(\\d)\\s+?"
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s+?"
			"(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s*?"
			);
	boost::smatch m;

	string name = s_iniPath;
	name += s_iniName;
	ifstream file(name.c_str());
	if (!file.good())
		return;

	string cpName;
	int cpId;
	int side;
	string line;

	while(getline(file, line))
	{
		TRACE(("prcessing line <%s>", line.c_str()));
		if (!boost::regex_match(line, m, regExpr))
			continue;

		TRACE(("matched"));
		do
		{
			if (!m[1].matched)
				break;
			cpName = m[1].str();

			if (!m[2].matched)
				break;
			cpName += m[2].str();

			cpId = SysFunx::cpNameToCpId(cpName.c_str());
			TRACE(("name <%s>, cpId <%d>", cpName.c_str(), cpId));
			if (cpId < 0)
				break;

			if (!m[3].matched)
				break;
			side = atoi(m[3].str().c_str());
			if (side != 0 && side !=1)
				break;

			if (!m[4].matched || !m[5].matched)
				break;

			//m_addr.insert(make_pair(pack32(cpId, side), m[4].str()));
			TRACE(("cpId <%d> side <%d> sol Ip1 <%s>", cpId, side, m[4].str().c_str()));
			TRACE(("sol Ip2 <%s>", m[5].str().c_str()));

			//IpAddrPairT addr = boost::tuples::make_tuple(m[4].str(), m[5].str());
			//IpAddrPairT addr = IpAddrPairT(m[4].str(), m[5].str());
			//TRACE(("Boost tuple, sol Ip1 <%s>", addr.get<0>().c_str()));
			//TRACE(("Boost tuple, sol Ip2 <%s>", addr.get<1>().c_str()));

			m_addrTable.insert(make_pair(pack32(cpId, side), IpAddrPairT(m[4].str(), m[5].str())));

		}
		while (false);
	}

	TRACE(("IPAddressTable::IPAddressTable() returns"));
}


bool IPAddressTable::find(int cpId, int side, string& ipAddr1, string& ipAddr2)
{
	using namespace Funx;
	bool result = false;
	ipAddr1 = "";
	ipAddr2 = "";
	map<int, IpAddrPairT>::iterator it;
	it = m_addrTable.find(pack32(cpId, side));
	if (it != m_addrTable.end())
	{
		result = true;
		ipAddr1 = it->second.get<0>();
		ipAddr2 = it->second.get<1>();
	}
	return result;
}

bool IPAddressTable::find(int composite, string& ipAddr1, string& ipAddr2)
{

	bool result = false;
	ipAddr1 = "";
	ipAddr2 = "";
	map<int, IpAddrPairT>::iterator it;
	it = m_addrTable.find(composite);
	if (it != m_addrTable.end())
	{
		result = true;
		ipAddr1 = it->second.get<0>();
		ipAddr2 = it->second.get<1>();
	}
	return result;
}

#if 0

bool IPAddressTable::find(int cpId, int side, string& ipAddr)
{
	using namespace Funx;
	bool result = false;
	ipAddr = "";
	map<int, string>::iterator it;
	it = m_addr.find(pack32(cpId, side));
	if (it != m_addr.end())
	{
		result = true;
		ipAddr = it->second;
	}
	return result;
}

bool IPAddressTable::find(int composite, string& ipAddr)
{

	bool result = false;
	ipAddr = "";
	map<int, string>::iterator it;
	it = m_addr.find(composite);
	if (it != m_addr.end())
	{
		result = true;
		ipAddr = it->second;
	}
	return result;
}

#endif

