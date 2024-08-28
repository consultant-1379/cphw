#ifndef _TERMINAL_DSD_SERVER_H_
#define _TERMINAL_DSD_SERVER_H_

#include "DSDServer.h"
#include "AtomicFlag.h"
#include "IPAddressTable.h"
#include "TermConn.h"

class TerminalServer: public DSDServer
{
public:

	friend class SolSession;

    TerminalServer(const char* name, const char* domain);
    virtual ~TerminalServer();
    void virtual start();
    void virtual stop();

    virtual ssize_t incomingData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t processScoConnectMsg(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);

    virtual void closeConnection(acs_dsd::HANDLE handle);

private:

	// Indicate when CMW stop the service
	AtomicFlag<bool>	m_aborting;

	// IP address table built from CS
	IPAddressTable	m_ipAddrTable;

	// Terminal connections
	TermConn	m_termConn;

	// A container of Terminal Sessions
	std::map<int, int>	m_termSession;

	TerminalServer(const TerminalServer&);             // Non copyable
	TerminalServer& operator=(const TerminalServer&);  // Non copyable

};

#endif
