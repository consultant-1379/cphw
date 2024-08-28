/*
 * MAServer.h
 *
 *  Created on: Jul 4, 2015
 *      Author: xdtthng
 */

#ifndef MASERVER_H_
#define MASERVER_H_

#include "boost/thread/mutex.hpp"
#include "boost/thread/condition_variable.hpp"

#include "TCPServer.h"

class MAServer: public TCPServer
{
public:

	MAServer(void);
    virtual ~MAServer(void);
    virtual void incoming_data_handler(TCPConnection_ptr ptr, const char* data, std::size_t num);
    void processGetFexReqMsg(TCPConnection_ptr ptr, const char* data);
    void processSetFexReqMsg(TCPConnection_ptr ptr, const char* data);
    void processGetApgStateReqMsg(TCPConnection_ptr ptr, const char* data);

    virtual void start(void);
    virtual void stop(void);
    virtual void run(void);

    // Wait until Server is started
    // This function must be called by thread different than the Thread of MAServer
    void waitUntilRunning(void);

    // Removed but kept
    // void processGetAdminStateReqMsg(TCPConnection_ptr ptr, const char* data);

private:

    // For condition variable
    boost::mutex                m_mutex;
    boost::condition_variable   m_condition;
    boost::thread				m_serverThread;

};

#endif /* MASERVER_H_ */
