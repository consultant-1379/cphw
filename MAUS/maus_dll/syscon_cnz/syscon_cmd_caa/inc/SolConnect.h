/*
 * SolConnect.h
 *
 *  Created on: Mar 2, 2016
 *      Author: xdtthng
 */

#ifndef SOLCONNECT_H_
#define SOLCONNECT_H_

#include <boost/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
//#include <bitset>

#include "Keyboard.h"
#include "SysconClient.h"
#include "ScoPinger.h"
#include "IPAddressTable.h"

typedef boost::scoped_ptr<SysconClient> Terminal;

class SolConnect
{
public:

	//typedef std::bitset<64> Bit64_t;
	//typedef std::bitset<8> Bit8_t;

	enum N100
	{
		ECHO_REPEAT = 10
	};

	SolConnect(int cpId, int side, int multiCpSystem);
	~SolConnect();
	void init();

	int connect(int index);

	int getIpErrCode(int index)
	{
		return m_ipError[index];
	}

	std::string getIpAddr(int index)
	{
		return m_ipAddress[index];
	}

private:

	ScoPinger 	m_echo;
	Terminal 	m_terminal;

	int			m_cpId;
	int 		m_side;
	int			m_multiCpSystem;
	//Keyboard	m_keyboard;
	bool		m_solConn;

	// IP address table built from CS
	IPAddressTable	m_ipAddrTable;

	std::string m_ipAddress[2];
	int 		m_ipError[2];
	int 		m_solError[2];

	void ipCheck();

	static std::string s_ipNetName[2];
	static std::string	s_connIniPath;

	//Bit64_t		m_solConnBc;
	//Bit8_t		m_solConnNcd;

	bool isSolConnect();
	int getCpId(const std::string& line);
	int getSide(const std::string& line);


};


#endif /* SOLCONNECT_H_ */
