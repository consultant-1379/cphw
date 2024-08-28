/*
 * TCPConnection.cpp
 *
 *  Created on: Aug 25, 2010
 *      Author: xquydao
 *              xdtthng 
 *      Modified based on boost::asio example
 */

#include <sstream>
#include <iostream>

#include "boost/bind.hpp"

#include "TCPConnection.h"
#include "TCPServer.h"
//#include "CPS_BUSRV_Trace.h"

#include "maus_trace.h"
#include "Maus_AehEvent.h"

TCPConnection::TCPConnection(TCPServer& server, boost::asio::io_service& io_service, 
                               const std::size_t& buffer_size) : socket_(io_service), server_(server),
                               m_abort(false)
{
    buffer_size_ = buffer_size;
    buffer_ptr_.reset(new char[buffer_size_]);
}

TCPConnection::~TCPConnection(void)
{    
}

void TCPConnection::start(void)
{
    //TRACE(("TCPConnection::start(void)"));
 
    m_abort = false;
     //if (isTRACEABLE()) {
        //std::ostringstream ss;
        //ss << "Connection "<< socket_.remote_endpoint() << " started.";
        //TRACE(("%s", ss.str().c_str()));
    //}

    /// Read incoming message after connection is established
    boost::asio::async_read(socket_, boost::asio::buffer(buffer_ptr_.get(), buffer_size_),
                            boost::bind(&TCPConnection::handle_read, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void TCPConnection::stop(void)
{
    //TRACE(("TCPConnection::stop(void)"));

    m_abort = true;
    if (socket_.is_open())
    {
        std::ostringstream ss;
        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);

        if (ec)
        {
            // Can't get the endpoint so just print like this (no endpoint info)
            // This may because client side doesn't close the connection gracefully
            ss << "Connection closed.";
        }
        else
        {
            ss << "Connection " << endpoint << " closed.";
        }

        //TRACE(("%s", ss.str().c_str()));

        /// Shutdown before close. Mentioned in boost asio doc for close().
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
    else
    {
    	boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);

    }

    /// Notify server connection closed.
    server_.handle_close(shared_from_this());
}

void TCPConnection::handle_read(const boost::system::error_code& e,
                                 std::size_t bytes_transferred)
{
    //TRACE(("TCPConnection::handle_read()"));

    if (!e)
    {
        //-- no need for this if (isTRACEABLE()) {
    		// Comment off for virtual APZ
            //std::ostringstream ss;
            //ss << "Received from "<< socket_.remote_endpoint() << " : " << bytes_transferred << " bytes";
            //TRACE(("%s", ss.str().c_str()));
        //-- no need}
        
        /// Notify incoming data to server
        server_.incoming_data_handler(shared_from_this(), buffer_ptr_.get(), bytes_transferred);

        // Read incoming message
        // the handle_read will be called when the buffer is full of data
        boost::asio::async_read(socket_, boost::asio::buffer(buffer_ptr_.get(), buffer_size_),
                                boost::bind(&TCPConnection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else if (e != boost::asio::error::operation_aborted)
    {
        
        // Try not to print eof since it is not an error
        if (e != boost::asio::error::eof)
            TRACE(("Connection read error. Error code : %d - %s", e.value(), e.message().c_str()));

        /// Close connection
        this->stop();                
    }
}

void TCPConnection::send(const char* data, const std::size_t& num)
{
    // Send data asynchnously
    // the handle_write will be called when the whole data is completedly sent
    boost::asio::async_write(socket_, boost::asio::buffer(data, num),
                             boost::bind(&TCPConnection::handle_write, shared_from_this(), 
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void TCPConnection::handle_write(const boost::system::error_code&, std::size_t)
{
    /// TODO: handle error case
}
//TCPServer& TCPConnection::getTCPServer() const
//{ 
//    return server_;   
//}

