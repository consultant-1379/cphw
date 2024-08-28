#ifndef _GMAU_DSD_SERVER_H_
#define _GMAU_DSD_SERVER_H_

#include "DSDServer.h"
#include "AtomicFlag.h"
#include "Maus_ApConn.h"

class Maus_Event;

class GMServer: public DSDServer
{
public:
    GMServer(const char* name, const char* domain, int, BoolFlagT&, Maus_Event&, Maus_Event&);
    virtual ~GMServer();
    void virtual start();
    void virtual stop();

    virtual ssize_t incomingData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t processGetOperState(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t processEnable(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t processDisable(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t processGetQuorumInfo(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t setActiveMask(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t performChecksum(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t setFCState(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    ssize_t installMibs(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);

private:

	GMServer(const GMServer&);             // Non copyable
	GMServer& operator=(const GMServer&);  // Non copyable

	int			m_instance;
	BoolFlagT&	m_apMausRunning;

	// Events from CpMauM MO
	Maus_Event&		m_enableEvent;
	Maus_Event&		m_disableEvent;

	Maus_Event		m_endEvent;
	ApConn			m_connection;

	BoolFlagT		m_msgHandling;

};

#endif
