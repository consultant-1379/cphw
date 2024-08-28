/*
 * TCPServer.h
 *
 *  Created on: Aug 25, 2010
 *      Author: xquydao
 *              xdtthng 
 *      Modified based on boost::asio example
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "boost/asio.hpp"
#include "boost/noncopyable.hpp"
#include <set>
#include <string>

#include "TCPConnection.h"
#include "TCPAcceptor.h"
//#include "PlatformTypes.h"

//class CriticalSection;

class TCPServer : private boost::noncopyable
{
public:

	static const unsigned short MA_TCP_SERVER_PORT = 22222;

    TCPServer();
    virtual ~TCPServer(void);

    /// start the acceptor and run io_service loop (demultiplex event).
    virtual void run(void);

    /// Stop the server
    virtual void stop(void);

    /// Incoming Data Handler
    virtual void incoming_data_handler(TCPConnection_ptr connection, const char* data, std::size_t num) = 0;

    /// Incoming Connection Handler
    void incoming_connection_handler(TCPConnection_ptr connection);

    /// Connection closed handler
    void handle_close(TCPConnection_ptr connection);    

    /// Send data on connection
    void send(const TCPConnection_ptr connection, const char* data, const std::size_t& num);

    /// Check if TCPServer is running
    bool isRunning();
    
    /// TTT maus
    //
    //void fatalError(uint32_t);
    //uint32_t fatalError() const;

protected:
    /// initialize
    bool init(void);
    
    /// Stop request handler
    void handle_stop(void);

    /// TTT maus
    //CriticalSection& CS_TCPServer() const;

    /// Listening port number
    unsigned short port_;

	// Indicate if TCPServer and its BUServer is running
    bool m_running;
    
    // Fatal error
    uint32_t	m_fatal_error;

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service io_service_;

    /// Acceptors used to listen for incoming connections.
    std::set<TCPAcceptor_ptr> acceptor_list_;

    /// List of connected Socket
    std::set<TCPConnection_ptr> connection_list_;

    /// Listening addresses
    std::set<std::string> addresses_;
};

#endif

