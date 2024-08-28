/*
 * IPAddressTable.h
 *
 *  Created on: Nov 4, 2015
 *      Author: xdtthng
 */

#ifndef IPADDRESSTABLE_H_
#define IPADDRESSTABLE_H_

#include <string>
#include <map>
#include <boost/tuple/tuple.hpp>

class TerminalServer;

class IPAddressTable
{
public:

	friend class TerminalServer;
	typedef boost::tuple<std::string, std::string> IpAddrStrPairT;

	IPAddressTable();
	~IPAddressTable();

	//bool find(int cpId, int side, std::string& ipAddr);
	//bool find(int composite, std::string& ipAddr);

	bool find(int cpId, int side, std::string& ipAddr1, std::string& ipAddr2);
	bool find(int composite, std::string& ipAddr1, std::string& ipAddr2);

private:
	//std::map<int, std::string>	m_addr;
	std::map<int, IpAddrStrPairT>	m_addrTable;

	static std::string	s_iniPath;
	static std::string	s_iniName;
};


#endif /* IPADDRESSTABLE_H_ */
