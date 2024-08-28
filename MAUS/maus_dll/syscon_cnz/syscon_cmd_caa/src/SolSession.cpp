/*
 * SolSession.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: xdtthng
 *
 *  Updated on: Sept 28, 2017
 *  GEP7 support introduced
 *      Author: teisdel
 */


#include <iostream>
#include <sys/poll.h>
#include <cstring>
#include <string>
#include <signal.h>

#include "SolSession.h"
#include "AppTrace.h"
#include "AppTrace.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "GepFunx.h"
#include "CmdError.h"


using namespace std;

int SolSession::s_solSessionId = 0;
os_handler_t* SolSession::s_osHandler = 0;

const int GEP7 = 7;

void SolSession::shutdownSolSession()
{
	TRACE(("SolSession::shutdownSolSession()"));
	if (s_osHandler)
	{
		ipmi_shutdown();
		s_osHandler->free_os_handler(s_osHandler);
		s_osHandler = 0;
	}
	TRACE(("SolSession::shutdownSolSession() returns"));
}

SolSession::SolSession(const string& solIpAddr, int cpId, int side, int multiCpSystem):
								m_cpId(cpId),
								m_side(side),
								m_multiCpSystem(multiCpSystem),
								m_osHandler(0),
								m_args(0),
								m_status(NO_ERROR),
								m_solIpAddr(solIpAddr),
								m_ipmi(0),
								m_activeConn(0),
								m_exitCondition(Running),
								m_connectionUp(0),
								m_id(s_solSessionId++),
								m_cmd(""),
								//m_buffer(m_cpId, m_side, m_multiCpSystem),
								m_cs(),
								m_cmdExit(false),
								m_thread(),
								m_lastAnyPortUp(-1),
								m_connCount(0),
								m_alog(),
								m_solDataBuffering(false)

{
	TRACE(("SolSession::SolSession() ctor"));
	TRACE(("SolSession::SolSession() ctor return"));
}

void SolSession::join()
{
	if (m_thread.joinable())
	{
		TRACE(("SolSession::~SolSession() m_thread.join()"));
		m_thread.join();
	}

}

SolSession::~SolSession()
{
	TRACE(("SolSession::~SolSession() dtor"));

	if (m_thread.joinable())
	{
		TRACE(("SolSession::~SolSession() m_thread.join()"));
		m_thread.join();
	}

#if 0

	if (m_activeConn)
	{
		ipmi_sol_force_close(m_activeConn);
		ipmi_sol_free(m_activeConn);
		m_activeConn = 0;
	}

	if (m_osHandler)
	{
		m_osHandler->free_os_handler(m_osHandler);
		m_osHandler = 0;
	}

#endif

	TRACE(("SolSession::~SolSession() dtor return"));
}


bool SolSession::isCmdExit() const
{
	return m_cmdExit;
}


int SolSession::getStatus() const
{
	return m_status;
}

void SolSession::disconnect()
{
	if (!m_connectionUp)
	{
		m_exitCondition = ExitNow;
	}
	// If we aren't already shutting down, do a forcible shut down!
	else if (m_exitCondition == Running)
	{
		m_exitCondition = Exiting;
		ipmi_sol_close(m_activeConn);
	}
	else
	{
		//cout << "Forcing SoL connection closed." << endl;
		m_exitCondition = ExitNow;
	}
}

void SolSession::logger(os_handler_t *handler, const char *format,
		enum ipmi_log_type_e log_type, va_list ap)

{
	// Will send to ACS Logging
	(void) handler;
	(void) format;
	(void) log_type;
	(void) ap;
}


int SolSession::configSol()
{
	TRACE(("SolSession::configSol()"));

	int rv = 0;

	rv = ipmi_sol_register_data_received_callback(m_activeConn, solDataAvailable, this);
	if (rv)
	{
		TRACE(("SolSession::configSol() failed to set incoming data callback"));
		rv = FAILED_TO_REG_SOL_INCOMING_DATA_CALLBACK;
		return rv;
	}

	rv = ipmi_sol_register_connection_state_callback(m_activeConn, connectionState, this);
	if (rv)
	{
		TRACE(("SolSession::configSol() failed to set connection state change callback"));
		rv = FAILED_TO_REG_SOL_CONN_STATE_CALLBACK;
		return rv;
	}

	ipmi_sol_set_ACK_retries(m_activeConn, 10);
	ipmi_sol_set_ACK_timeout(m_activeConn, 1000000);

	TRACE(("SolSession::configSol() return"));

	return rv;
}


int SolSession::send(const char* buffer, int count)
{

	int rv = ipmi_sol_write(m_activeConn, buffer, count, transmitComplete, NULL);

	if (rv)
	{
		char buf[50];
		ipmi_get_error_string(rv, buf, 50);
		TRACE(("SolSession::send() error msg <%s>", buf));
	}
	return rv;
}


void SolSession::connectionState(ipmi_sol_conn_t *conn, ipmi_sol_state state, int error, void *cb_data)
{
	(void) conn;

	char errorMsg[SOL_ERROR_MSG_LEN];
	SolSession* thisSol = (SolSession*) cb_data;
	ipmi_get_error_string(error, errorMsg, SOL_ERROR_MSG_LEN);

	if ((state == ipmi_sol_state_connected || state == ipmi_sol_state_connected_ctu) && !thisSol->m_connectionUp)
	{
		thisSol->m_connectionUp = 1;
		cout << "Connected. Enter \"^]\" to exit." << endl;
	}

	if (state == ipmi_sol_state_closed)
		thisSol->m_exitCondition = ExitNow;
}

void SolSession::connectionChanged(ipmi_con_t *ipmi, int error, unsigned int port_num, int any_port_up, void* cb_data)
{
	(void) ipmi;
	(void) port_num;
	(void) error;


	SolSession* thisSol = (SolSession*) cb_data;
	ipmi_sol_conn_t* conn = thisSol->m_activeConn;

	//cout << "SolSession::connectionChanged() for m_id " << thisSol->getId() << endl;

	TRACE_DEBUG(("SolSession::connectionChanged() error <%d>, any_port_up <%d> last_any_port_up <%d> for m_id <%d>",
			error, any_port_up, thisSol->m_lastAnyPortUp, thisSol->m_id));

	if (any_port_up == thisSol->m_lastAnyPortUp)
	{
		TRACE(("any_port_up == thisSol->m_lastAnyPortUp <%d>", (any_port_up == thisSol->m_lastAnyPortUp)));
		return;
	}

	if (any_port_up == 0 && thisSol->m_lastAnyPortUp == -1)
	{
		//	thisSol->m_exitCondition = ExitNow;
		//if (thisSol->m_check)
		//	thisSol->m_checkErrorEvent.set();

		//	return;
		TRACE(("setting thisSol->m_status = INIT_ERROR"));
		thisSol->m_status = INIT_ERROR;
	}

	thisSol->m_lastAnyPortUp = any_port_up;
	int rv = error;
	if (any_port_up)
	{
		rv  = ipmi_sol_open(conn);
		TRACE_DEBUG(("SolSession::connectionChanged() ipmi_sol_open() result <%d> for m_id <%d>", rv, thisSol->m_id));
		if (rv == 0)
		{
			thisSol->m_status = NO_ERROR;
			thisSol->m_connCount += 1;
			TRACE(("setting thisSol->m_status = NO_ERROR"));
		}
		else
		{
			thisSol->m_status = FATAL_ERROR;
			TRACE(("setting thisSol->m_status = FATAL_ERROR"));
		}
	}
	else
	{
		TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
		TRACE(("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));

		char buf[50];
		TRACE(("No connection to BMC is available. %s", ipmi_get_error_string(error, buf, 50)));

		TRACE(("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
		TRACE((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
		rv = ipmi_sol_force_close(conn);
		TRACE_DEBUG(("SolSession::connectionChanged(), ipmi_sol_force_close() result <%d> for m_id <%d>", rv, thisSol->m_id));

		thisSol->m_exitCondition = ExitNow;

		ipmi_get_error_string(error, buf, 50);
		TRACE(("ipmi_sol_force_close() result. %s", ipmi_get_error_string(error, buf, 50)));
		thisSol->m_status = thisSol->m_status != INIT_ERROR ? FATAL_ERROR : INIT_ERROR;
		TRACE(("setting thisSol->m_status to <%d>", thisSol->m_status));
	}

	//cout << "SolSession::connectionChanged() m_status is " << thisSol->m_status << endl;
}


void SolSession::clientDataAvailable(int fd, void* data)
{
	(void) fd;
	(void) data;

	//TRACE(("SolSession::incomingDataFromClient()"));


	struct pollfd 	fds[1];

	// Stop syscon client thread
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;

	int res = 0;
	char ch;
	do
	{
		//TRACE(("SolSession::incomingDataFromClient() poll loop"));
		res = poll(fds, 1, 1500);
		//res = poll(fds, 1, -1);

		if (res == 0)
		{
			//continue;
			break;
		}

		if (res < 0)
		{
			// Disconnect Sol here
			disconnect();
			TRACE(("SolSession::incomingDataFromClient() poll error <%d>", errno));
			break;
		}

		if (0 == read(STDIN_FILENO, &ch, 1))
		{
			// Disconnect sol
			disconnect();
			break;
		}
		TRACE(("keyboard char input is <%c> <%d>", ch, ch));

		switch (ch)
		{
		case Keyboard::BS:
		case Keyboard::DEL:
			if(m_cmd.length() > 0)
			{
				// Remove the last character added to the input string.
				m_cmd.erase(m_cmd.end() - 1);
			}

			break;

			// Ctrl + ], control key following by a square bracket
		case Keyboard::GS:
			cout << "^]" << flush;
			disconnect();
			TRACE(("SolSession::incomingDataFromClient() ^] forces exit now"));
			m_cmdExit = true;
			cout << endl << flush;
			break;

		case Keyboard::CR:
		{
			// Send cmd to ALOG
			TRACE(("Commplete command <%s>", m_cmd.c_str()));

			if (m_alog.logData(Unixcmd, m_cmd.c_str(), "", "", m_cmd.length()) != acs_alog_ok)
			{
				TRACE(("Failed to send to ALOG command <%s>", m_cmd.c_str()));
			}
			//m_alog.logData(Unixcmd, data.c_str(), "", "", static_cast<int32>(data.size()),
			//		"", "", "", "", "", "", "", "", // Default values for the ALOG method.
			//		m_cpName.c_str(),
			//		cpID == UNDEF_CPID ? 0 : &cpID, // Just one CPId exists for CPT,as
			//				cpID == UNDEF_CPID ? 0 : 1);    // it doesn't support CP groups.


			m_cmd = "";
			break;
		}

		default:
			m_cmd += ch;
			break;
		}

		this->send(&ch, 1);

#if 0

		if (ch == Keyboard::CR)
		{
			TRACE(("string keyboard char input is <%s> ", m_cmd.c_str()));
			if (m_cmd == "exit\r")
			{
				cout << endl << flush;
				//m_exitCondition = ExitNow;
				disconnect();
				TRACE(("SolSession::incomingDataFromClient() \"exit\" forces exit now"));
				cout << endl << flush;
				m_cmdExit = true;
				break;
			}
			m_cmd = "";
		}

#endif

	}
	while (false);

}

void SolSession::transmitComplete(ipmi_sol_conn_t*, int error, void*)
{

	TRACE(("SolSession::transmitComplete() observer error code <%d>", error));
	if (!error)
		return;

	if (IPMI_IS_SOL_ERR(error) && (IPMI_GET_SOL_ERR(error) == IPMI_SOL_UNCONFIRMABLE_OPERATION))
	{
		TRACE(("SolSession::transmitComplete() error is Unconfirm operation"));
	}
	else
	{
		// Transmission failed
		char buf[50];
		ipmi_get_error_string(error, buf, 50);
		TRACE(("SolSession::transmitComplete() transmission fail <%s>", buf));
	}
}


void SolSession::clientDataAvailable(int fd, void *cb_data, os_hnd_fd_id_t *id)
{
	(void) id;
	(void) fd;

	SolSession* thisSol = (SolSession*) cb_data;
	thisSol->clientDataAvailable(fd, cb_data);

}

////////////////////////////////

void SolSession::init()
{

	int gepVer, maxSysconParam;

	GepFunx::getGepVersion(gepVer, m_cpId, m_side);

	if (gepVer < 0)

	{
		throw eFailedToGetGepVersion;
	}


	if (gepVer < GEP7){
		maxSysconParam = 3;  //in case of GEP5 3 parameters must be passed in SOL connection command
	}
	else {
		maxSysconParam = 9; //in case of GEP7 9 parameters must be passed in SOL connection command
	}                       //options -U<user> -P<pwd> -Luser are icluded


	TRACE(("SolSession::init()"));

	// OS handler allocated first
	// --------------------------
	// The following line works with SLES11
	// m_osHandler = ipmi_posix_thread_setup_os_handler();

	// Changing to SLES12, there should be only one openIPMI OS Handelr
	if (!s_osHandler)
		s_osHandler = ipmi_posix_setup_os_handler();

	// This is to allow the rest of the code using m_osHandler
	m_osHandler = s_osHandler;
	if (!m_osHandler) {
		m_status = FAILED_TO_GET_OS_HANDLER;
		TRACE(("SolSession::init() return, failed to get os handler"));
		return;
	}
	TRACE(("SolSession::init() get os handler ok"));

	m_osHandler->set_log_handler(m_osHandler, SolSession::logger);
	TRACE(("SolSession::init() register logger ok"));

	// Initialize the OpenIPMI library
	ipmi_init(m_osHandler);
	TRACE(("SolSession::init() ipmi_init ok"));
	TRACE(("SolSession::init() ip address from CS is <%s>", m_solIpAddr.c_str()));

#if 1

	char* argv[maxSysconParam];
	int argc = maxSysconParam;


	for( int index = 0; index < maxSysconParam; index++ ) {
		argv[index] = new char [255];
	}

	strcpy(argv[0], "sol");
	strcpy(argv[1], "lan");
	strncpy(argv[maxSysconParam - 1], m_solIpAddr.c_str(), m_solIpAddr.length());

	if (gepVer > 5){

		strcpy(argv[2], "-U");
		strcpy(argv[3], "soluser");
		strcpy(argv[4], "-P");
		strcpy(argv[5], "solsecret");
		strcpy(argv[6], "-L");
		strcpy(argv[7], "user");

	}

#else
	// Faking the following
	// ---------------------
	int argc = 3;
	char* argvstr[3];
	argvstr[0] = (char*)"sol";
	argvstr[1] = (char*)"lan";
	argvstr[2] = (char*)"192.168.169.200";
#endif

	int curr_arg = 1;
	TRACE(("SolSession::init() argv[2] <%s> curr_arg <%d>", argv[2], curr_arg));

	int rv = ipmi_parse_args2(&curr_arg, argc, argv, &m_args);

	if (rv)
	{
		m_status = FAILED_TO_PARSE_SOL_IP_ADDR;
		TRACE(("SolSession::init() return, failed to get sol ip address"));
		return;
	}
	TRACE(("SolSession::init() ipmi_parse_args2() ok"));

	for( int index = 0; index < maxSysconParam; index++ ) {
			delete argv[index];
	}

	rv = ipmi_args_setup_con(m_args, m_osHandler, NULL, &m_ipmi);
	if (rv)
	{
		m_status = FAILED_TO_SETUP_IPMI_CONNECTION;
		TRACE(("SolSession::init() return, failed to setup ipmi connection"));
		return;
	}

	rv = ipmi_sol_create(m_ipmi, &m_activeConn);
	if (rv)
	{
		m_status = FAILED_TO_SETUP_IPMI_SOL_CONNECTION;
		TRACE(("SolSession::init() return, failed to setup ipmi sol connection"));
		return;
	}
	else{

		rv = ipmi_sol_set_use_authentication(m_activeConn, 0);
	}

	rv = this->configSol();
	if (rv)
	{
		m_status = SolSession::SolErrorT(rv);
		TRACE(("SolSession::init() return, configSol() failed"));
		return;
	}

	rv = m_ipmi->add_con_change_handler(m_ipmi, connectionChanged, this);
	if (rv)
	{
		m_status = SolSession::SolErrorT(rv);
		TRACE(("SolSession::init() return, add connection change handler failed"));
		return;
	}


	// Start processing on a connection.  Note that the handler *must*
	// be called with the global read lock not held, because the
	// handler must write lock the global lock in order to add the MC
	// to the global list.  This will report success/failure with the
	// con_changed_handler, so set that up first.
	rv = m_ipmi->start_con(m_ipmi);

	if (rv)
	{
		m_status = SolSession::SolErrorT(rv);
		TRACE(("SolSession::init() return, start connection failed"));
		return;
	}


	TRACE(("SolSession::init() return ok"));
}


void SolSession::start()
{
	m_thread = boost::thread(boost::bind(&SolSession::run, this));

}

void SolSession::run()
{
	TRACE(("SolSession::run() m_id <%d>", m_id));

	os_hnd_fd_id_t *stdin_id;

	m_osHandler->add_fd_to_wait_for(m_osHandler,
			STDIN_FILENO,
			clientDataAvailable, /* os_data_ready_t */
			this, /* cb_data */
			NULL, /*os_fd_data_freed_t */
			&stdin_id);

	TRACE(("SolSession::run() m_id <%d> complete register for incomingDataFromClient callback", m_id));

	m_exitCondition = Running;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	unsigned int count = 0;

	while (m_exitCondition != ExitNow)
	{
		if (++count%30 == 0)
		{
			TRACE(("SolSession::run() m_id <%d> looping", m_id));
		}

		//m_osHandler->perform_one_op(m_osHandler, NULL);
		m_osHandler->perform_one_op(m_osHandler, &tv);
	}

	if (m_activeConn)
	{
		ipmi_sol_force_close(m_activeConn);
		ipmi_sol_free(m_activeConn);
		m_activeConn = 0;
	}

	// Keep this comment and the following code
	// The following code is needed for SLES11
	// When changing to SLES12, this will crash when creating a new openIPMI OS Handler
	//

#if 0

	if (m_osHandler)
	{
		ipmi_shutdown();
		m_osHandler->free_os_handler(m_osHandler);
		m_osHandler = 0;
	}

#endif

	TRACE(("SolSession::run() m_id <%d> returns", m_id));
}

void SolSession::stop()
{
	disconnect();
}

int SolSession::solDataAvailable(ipmi_sol_conn_t *conn, const void *data, size_t count, void *user_data)
{
	(void) conn;
	(void) user_data;

	//if (count == 1)
	//{
	//	cout << reinterpret_cast<const unsigned char*>(data)[0] << flush;
	//	return 0;
	//}

	SolSession* thisSol = (SolSession*) user_data;
	thisSol->solDataAvailable(conn, (const char*)data, count);

	return 0;
}

void SolSession::solDataAvailable(ipmi_sol_conn_t*, const char* data, size_t n)
{
	//TRACE(("SolSession::solDataAvailable()"));
	// The menu is as the following
	// ----------------------------

	// *** ================================================= ***
	// *** SOL CONNECTION BUSY. CHOOSE ALTERNATIVE FROM LIST ***
	// *** ================================================= ***
	//  B  Boot other session from IP-Address 169.254.215.1
	//  W  Wait for SOL connection free
	//  X  Exit                                              TBI
	//
	// Enter selection:


	static string s_signature = "SOL CONNECTION BUSY. CHOOSE ALTERNATIVE FROM LIST";
	static string s_end = "Enter selection: ";
	static const char* s_boot = "BB";
	static const size_t s_oneChar = 1;

	string buffer(data, n);
	cout << buffer << flush;

	int res;
	if (!m_solDataBuffering)
	{
		if (n > 1 && buffer.find(s_signature) != string::npos)
		{
			TRACE(("SolSession::solDataAvailable() menu starts wiht n <%d>", n));
			m_solDataBuffering = true;
		}
	}
	else
	{
		if (n > 1 && buffer.find(s_end) != string::npos)
		{
			m_solDataBuffering = false;

			TRACE(("SolSession::solDataAvailable() menu ends; try to send B to SOL"));

			usleep(200000);
			res = this->send(s_boot, s_oneChar);
			usleep(200000);
			res = this->send(s_boot, s_oneChar);
			TRACE(("SolSession::solDataAvailable() send B twice with res <%d>", res));

			// Boot SOL, if failed try again
			if (res)
			{
				TRACE(("SolSession::solDataAvailable() menu ends; second attempt to send B to SOL"));
				usleep(200000);
				res = this->send(s_boot, s_oneChar);
				usleep(200000);
				res = this->send(s_boot, s_oneChar);
			}
			TRACE(("SolSession::solDataAvailable() menu ends with n <%d> send B result <%d>", n, res));
		}

	}
	//TRACE(("SolSession::solDataAvailable() returns"));

}

