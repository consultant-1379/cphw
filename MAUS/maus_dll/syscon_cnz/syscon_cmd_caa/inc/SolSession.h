/*
 * SolSession.h
 *
 *  Created on: Nov 7, 2015
 *      Author: xdtthng
 */

#ifndef SOLSESSION_H_
#define SOLSESSION_H_

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

extern "C"
{
#include <OpenIPMI/ipmiif.h>
#include <OpenIPMI/ipmi_smi.h>
#include <OpenIPMI/ipmi_err.h>
#include <OpenIPMI/ipmi_auth.h>
#include <OpenIPMI/ipmi_lan.h>
#include <OpenIPMI/ipmi_posix.h>

#include <OpenIPMI/internal/ipmi_int.h>
#include <OpenIPMI/ipmi_sol.h>
#include <OpenIPMI/ipmi_debug.h>
};

#include <string>
#include "AppEvent.h"
//#include "Buffer.h"
#include "Keyboard.h"
#include "acs_alog_drainer.h"


class SolSession
{
public:

	typedef enum {
		Running,
		Exiting,
		ExitNow,
	} ExitConditionT;


	enum SolErrorT
	{
		NO_ERROR,
		FAILED_TO_GET_OS_HANDLER,
		FAILED_TO_PARSE_SOL_IP_ADDR,
		FAILED_TO_SETUP_IPMI_CONNECTION,
		FAILED_TO_SETUP_IPMI_SOL_CONNECTION,
		FAILED_TO_REG_SOL_INCOMING_DATA_CALLBACK,
		FAILED_TO_REG_SOL_CONN_STATE_CALLBACK,
		INIT_ERROR,
		CONNECTION_CLOSED,
		FATAL_ERROR
	};

	enum
	{
		SOL_ERROR_MSG_LEN = 50
	};

	enum CmdState
	{
		SolEvent,
		Waiting_for_Sol_Resp
	};

	SolSession(const std::string& solIpAddr, int cpId, int side, int multiCpSystem);
	~SolSession();

	static void logger(os_handler_t *handler, const char *format,
		enum ipmi_log_type_e log_type, va_list ap);

	void init();
	void start();
	void stop();
	void run();
	bool isCmdExit() const;
	int getStatus() const;
	void join();

	int getConnCount()
	{
		return m_connCount;
	}

	// Call back functions
	static int solDataAvailable(ipmi_sol_conn_t *conn, const void *data, size_t count, void *user_data);
	static void connectionState(ipmi_sol_conn_t *conn, ipmi_sol_state state, int error, void *cb_data);
	static void connectionChanged(ipmi_con_t* ipmi, int err, unsigned int port_num, int any_port_up, void *cb_data);
	static void clientDataAvailable(int fd, void *cb_data, os_hnd_fd_id_t *id);

	static void transmitComplete(ipmi_sol_conn_t *conn, int error, void *user_data);

	static void shutdownSolSession();

private:

	int		m_cpId;
	int		m_side;
	int		m_multiCpSystem;

	os_handler_t*		m_osHandler;
	ipmi_args_t*		m_args;
	SolErrorT			m_status;
	std::string			m_solIpAddr;
	ipmi_con_t*			m_ipmi;
	ipmi_sol_conn_t*	m_activeConn;
	ExitConditionT		m_exitCondition;		// provide multi thread protection later
	int					m_connectionUp;			// provide multi thread protection later
	int					m_id;
	std::string			m_cmd;

	boost::mutex		m_cs;
	bool				m_cmdExit;
	boost::thread		m_thread;
	int					m_lastAnyPortUp;

	// Handle to the Keyboard associated with STDIN.
	//Keyboard 	m_keyboard;
	int			m_connCount;
	acs_alog_Drainer m_alog;
	bool	m_solDataBuffering;


	static int			s_solSessionId;
	static os_handler_t*		s_osHandler;

	int send(const char* buffer, int count);
	void clientDataAvailable(int, void*);
	void solDataAvailable(ipmi_sol_conn_t *conn, const char*, size_t);
	int getId() const;
	void disconnect();
	int configSol();

};

inline
int SolSession::getId() const
{
	return m_id;
}

#endif /* SOLSESSION_H_ */
