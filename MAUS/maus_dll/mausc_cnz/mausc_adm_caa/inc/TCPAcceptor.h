/*
 * TCPAcceptor.h
 *
 *  Created on: Sep 15, 2010
 *      Author: xquydao
 *              xdtthng
 */

#ifndef TCP_ACCEPTOR_H_
#define TCP_ACCEPTOR_H_

#include "boost/asio.hpp"
#include "boost/noncopyable.hpp"
#include "boost/enable_shared_from_this.hpp"

#include "TCPConnection.h"

/// Fwd declaration
class TCPServer;

class TCPAcceptor : public boost::enable_shared_from_this<TCPAcceptor>,
                     private boost::noncopyable
{
public:
    /// Construct a TCPAcceptor
    explicit TCPAcceptor(boost::asio::ip::tcp::endpoint endpoint, TCPServer& server, boost::asio::io_service& io_service);
    virtual ~TCPAcceptor(void);

    /// Start the acceptor
    void start(void);

    /// Stop the acceptor
    void stop(void);
    
    /// Bind the endpoint
    bool bind();

private:
    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code& e);

    /// Address and port to listen
    boost::asio::ip::tcp::endpoint endpoint_;

    /// tcp server reference
    TCPServer& server_;

    /// acceptor to listen then accept connections
    boost::asio::ip::tcp::acceptor acceptor_;

    /// Current accepting connection
    TCPConnection_ptr current_connection_ptr_;

};

typedef boost::shared_ptr<TCPAcceptor> TCPAcceptor_ptr;

#endif /* TCP_ACCEPTOR_H_ */
