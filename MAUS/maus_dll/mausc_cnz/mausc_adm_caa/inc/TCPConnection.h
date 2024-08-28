/*
 * TCPConnection.h
 *
 *  Created on: Aug 25, 2010
 *      Author: xquydao
 *              xdtthng 
 *      Modified based on boost::asio example
 */

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/shared_array.hpp"

/// Fwd declaration
class TCPServer;

/// Constant
const std::size_t DEFAULT_BUFFER_SIZE = 512;

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>,
                       private boost::noncopyable
{
public:
    /// Construct a connection with the given io_service.
    explicit TCPConnection(TCPServer& server, boost::asio::io_service& io_service, 
                            const std::size_t& buffer_size = DEFAULT_BUFFER_SIZE);
    ~TCPConnection(void);

    /// Get the socket.
    boost::asio::ip::tcp::socket& socket() {return socket_;};

    /// Send data
    void send(const char* data, const std::size_t& num);

    /// Start the first read asynchronous operation.
    void start(void);

    /// Stop all asynchronous operations.
    void stop(void);
    
    // Retrieve TCP Server 
    TCPServer& getTCPServer() const { return server_; }

private:
    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code& e,
                     std::size_t bytes_transferred);

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code& e,
                      std::size_t bytes_transferred);

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;    
  
    /// Buffer for incoming data.
    std::size_t buffer_size_;
    boost::shared_array<char> buffer_ptr_;

    /// TCPServer
    TCPServer& server_;
    bool m_abort;
};

typedef boost::shared_ptr<TCPConnection> TCPConnection_ptr;

#endif
