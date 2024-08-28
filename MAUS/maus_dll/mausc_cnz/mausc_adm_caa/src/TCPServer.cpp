/*
 * TCPServer.cpp
 *
 *  Created on: Aug 25, 2010
 *      Author: xquydao
 *              xdtthng 
 *      Modified based on boost::asio example
 */
#include <algorithm>
#include "boost/bind.hpp"
#include <boost/thread/thread.hpp>

#ifdef _APGUSERSVC_
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif


//#include "Config.h"
#include "TCPServer.h"

#include "maus_trace.h"
#include "Maus_AehEvent.h"

//#include "CPS_BUSRV_Trace.h"
//#include "CPS_Utils.h"
//#include "CriticalSection.h"

//#include "CodeException.h"
//#include "BUService.h"

//extern BUService* g_pService;
//extern BUServer* g_pServer;
//static boost::posix_time::milliseconds sleepTime = boost::posix_time::milliseconds(2000);

/// TTT maus
//
#if 0
CriticalSection& TCPServer::CS_TCPServer() const
{
	// This is to ensure that s_cs is constructed exactly once on first use
	static CriticalSection s_cs;
	return s_cs;
}


void TCPServer::fatalError(uint32_t code)
{
	TRACE(("TCPServer::fatalError(%u)", code));

	/// TTT maus
	//AutoCS a(CS_TCPServer());
	m_fatal_error = code;
}

uint32_t TCPServer::fatalError() const
{
	/// TTT maus
	//
	//AutoCS a(CS_TCPServer());
	return m_fatal_error;
}
#endif

/// TTT maus
// user port 22222
//TCPServer::TCPServer() : port_(Config::instance().busrvPort()), m_running(false), m_fatal_error(0)
TCPServer::TCPServer() : port_(MA_TCP_SERVER_PORT), m_running(false), m_fatal_error(0)
{
    //newTRACE((LOG_LEVEL_INFO, "TCPServer::TCPServer()", 0));
}

TCPServer::~TCPServer(void)
{
    //newTRACE((LOG_LEVEL_INFO, "TCPServer::~TCPServer()", 0));
}

bool TCPServer::isRunning()
{
    TRACE(("TCPServer::isRunning(void)"));

    return m_running;
}

bool TCPServer::init(void)
{
    TRACE(("TCPServer::init()"));
    
    acceptor_list_.clear();
    connection_list_.clear();
    addresses_.clear();
    io_service_.reset();

    static const char* IP_LISTENING_ADDRESS[4] = {
        "192.168.169.1",
        "192.168.169.2",
        "192.168.170.1",
        "192.168.170.2"
    };

    boost::asio::ip::address addr;
    TCPAcceptor_ptr acceptor_ptr;

    try { 
        // This loop returns if successfully bound at least an IP address  
        for (int i = 0; i < 2; ++i ) {
        	TRACE(("TCPServer attempts binding IP address <%s>", IP_LISTENING_ADDRESS[i]));
            addr = boost::asio::ip::address::from_string(IP_LISTENING_ADDRESS[i]);
            acceptor_ptr.reset(new TCPAcceptor(boost::asio::ip::tcp::endpoint(addr, port_), *this, io_service_));
            if (acceptor_ptr->bind()) {
                acceptor_list_.insert(acceptor_ptr);

                /// TTT maus
                //
                //Config::instance().simAp(Config::APG_ONE_IP_ADDRESS);
                TRACE(("Bind successful on address: %s ", IP_LISTENING_ADDRESS[i]));
                addr = boost::asio::ip::address::from_string(IP_LISTENING_ADDRESS[i + 2]);
                acceptor_ptr.reset(new TCPAcceptor(boost::asio::ip::tcp::endpoint(addr, port_), *this, io_service_));
                
                if (acceptor_ptr->bind()) {

                	/// TTT maus
                	//
                    //Config::instance().simAp(Config::APG_TWO_IP_ADDRESS);
                    TRACE(("Bind successful on address: %s", IP_LISTENING_ADDRESS[i+2]));
                    acceptor_list_.insert(acceptor_ptr);    // Both IP addresses are bound 
                }
                return true; // Could be both IP addresses or one IP address are bound
            }
            else {
				addr = boost::asio::ip::address::from_string(IP_LISTENING_ADDRESS[i + 2]);
                acceptor_ptr.reset(new TCPAcceptor(boost::asio::ip::tcp::endpoint(addr, port_), *this, io_service_));
                
                if (acceptor_ptr->bind()) {

                	/// TTT mau
                	//
                    //Config::instance().simAp(Config::APG_ONE_IP_ADDRESS);
                    acceptor_list_.insert(acceptor_ptr);    // One IP address is bound
                    TRACE(("Bind successful on address: %s", IP_LISTENING_ADDRESS[i+2]));
                    return true;
                }
            }
            // No IP address has been bound successfully at this point
        } // for 
              
		// None of the IP_LISTENING_ADDRESS can be bound, try to bind all ip adress
        // For SimAP, none of IP_LISTENING_ADDRESS is defined.
        //
	    TRACE(("TCPServer::init attemps binding on all IP addresses used in SimAP"));
        addr = boost::asio::ip::address_v4::any();
        acceptor_ptr.reset(new TCPAcceptor(boost::asio::ip::tcp::endpoint(addr, port_), *this, io_service_));

        if (acceptor_ptr->bind()) {

        	/// TTT maus
        	//
            //Config::instance().simAp(Config::SIMAP_ALL_IP_ADDRESS);
            acceptor_list_.insert(acceptor_ptr);
			TRACE(("Bind successful on all address used in SimAP"));
            return true;	// All IP addresses are bound
        }
        else {
        	TRACE(("TCPServer::init; unable to bind on all IP addresses used in SimAP"));
        }

        /// TTT maus
        //THROW_XCODE("TCP init failed", CodeException::TCP_INIT_FAILED);
    }

    /// TTT maus
    //catch (CodeException& e) {
    //    EventReporter::instance().write(e.what().c_str());
    //    TRACE(("%s", e.what().c_str()));
    //}
    catch (boost::system::system_error &e) {
        boost::system::error_code ec = e.code();
        std::ostringstream ss;
        ss << "TCPServer::init() failed. Boost Error code: " << ec.value() << " - " << ec.message().c_str();
        TRACE(("%s", ss.str().c_str()));

        /// TTT maus
        //EventReporter::instance().write(ss.str().c_str());
    }
    catch (...) {

    	/// TTT maus
    	//
        //EventReporter::instance().write("TCPServer::init(); Unknown exception");
        TRACE(("TCPServer::init(); Unknown exception"));
    }
    TRACE(("TCPServer::init() returns false"));
    return false;
}

void TCPServer::run(void)
{
    TRACE(("TCPServer::run()"));

    try
    {
        //if (isTRACEABLE()) {
            std::ostringstream ss;
            ss << "TCPServer::run() thread id "<< boost::this_thread::get_id();
            TRACE(("%s", ss.str().c_str()));
        //}



 	// init the server in 3 times
 		int retries = 0;
        TRACE(("Attempt to bind IP address; initial running flag is %d", m_running));
    	while (!this->init()) {
        	TRACE(("Failed to initialize the server: %d times", ++retries));

        	/// TTT maus
        	//

			// Do not retry if run as console 
        	//if ((g_pService || g_pServer) && retries < 3) {
            if (retries < 3) {
            	//boost::this_thread::sleep(sleepTime);
    			struct timespec req;
    			req.tv_sec = 2UL;    // Sleeps for 2 seconds
    			req.tv_nsec = 0L;
    			nanosleep(&req, NULL);			 
            	continue;
        	}

        	// Try 3 times but failed, exit the loop and report the error
        	// Report error to AMF if it's running as service
        	m_running = false;

        	/// TTT maus
        	//
#if 0
        	if (g_pService) {
            	TRACE(("Reporting component error to AMF"));
            	g_pService->componentReportError(ACS_APGCC_COMPONENT_RESTART);
				//EventReporter::instance().write("Reporting component error to AMF");
        	}
#endif
       		TRACE(("Attempt to bind IP address failed; running flag is %d", m_running));
         	return;
    	}
       	TRACE(("Attempt to bind IP address succeeded; running flag is %d", m_running));


        // Start acceptor to listen to client connections
        for_each(acceptor_list_.begin(), acceptor_list_.end(), boost::bind(&TCPAcceptor::start, _1));

        // The io_service::run() call will block until all asynchronous operations
        // have finished. While the server is running, there is always at least one
        // asynchronous operation outstanding: the asynchronous accept call waiting
        // for new incoming connections.
        TRACE(("TCPServer::run() about to call io_service_.run()"));
        io_service_.run();
    	TRACE(("TCPServer::run() exits returning from io_service_.run() without error"));
    	return;
    }
    catch (boost::system::system_error& e)
    {
        boost::system::error_code ec = e.code();
        TRACE(("TCPServer::run() failed. Boost Error code: %d - %s", ec.value(), ec.message().c_str()));
        /// stop the server to clean up
        this->handle_stop();
    }

    /// TTT maus
    //
#if 0
    catch (CodeException& e) {
        TRACE(("TCPServer::run() failed - %s", e.what().c_str()));
        /// stop the server to clean up
        this->handle_stop();
    }
#endif
    /// TTT maus
    //
#if 0
	if (g_pService) {
    	TRACE(("Reporting component error to AMF"));
		g_pService->componentReportError(ACS_APGCC_COMPONENT_RESTART);
		//EventReporter::instance().write("Reporting component error to AMF");
	}
#endif
    TRACE(("TCPServer::run() returns with some error conditions"));
}

void TCPServer::stop(void)
{
    TRACE(("TCPServer::stop()"));

    // Post stop request to io_server
    // in order to shut down server gracefully
    // Note: this function can be called at any threads.
    io_service_.post(boost::bind(&TCPServer::handle_stop, this));    
}

void TCPServer::handle_stop(void)
{
    TRACE(("TCPServer::handle_stop()"));
    /// Close the acceptors
    std::set<TCPAcceptor_ptr>::iterator accIt = acceptor_list_.begin();

    for_each(accIt, acceptor_list_.end(), boost::bind(&TCPAcceptor::stop, _1));
    acceptor_list_.clear();

    // Close all the connections
    // The connection will be automatically remove from the connection_list
    std::set<TCPConnection_ptr>::iterator connIt = connection_list_.begin();
    
    TRACE(("Size of connection list is %d", connection_list_.size()));
    while (connIt != connection_list_.end())
        (*connIt++)->stop();

   // io_service_.stop();
    //io_service_.~io_service();

    // There should be no handles queueing for io_service at this point
    // Attempt to reset io_service internal state, preparing for the next run
    TRACE(("TCPServer::handle_stop(), about to call io_service.reset()"));
    io_service_.reset();
    //io_service_.~io_service();
    TRACE(("TCPServer::handle_stop(), io_service.reset() returns"));
        
    TRACE(("TCPServer::handle_stop(), Server successfully stopped"));
}

#if 0
void TCPServer::incoming_data_handler(TCPConnection_ptr, const char*, std::size_t)
{
    /// Do nothing, the real service is handled by subclass
}
#endif

void TCPServer::incoming_connection_handler(TCPConnection_ptr connection)
{
    /// Add the connection to the list
    connection_list_.insert(connection);
}

void TCPServer::handle_close(TCPConnection_ptr connection)
{
    /// Connection is closed, remove it from the list
    connection_list_.erase(connection);
}

void TCPServer::send(const TCPConnection_ptr connection, const char* data, const std::size_t& num)
{
    connection->send(data, num);
    
    // Perform house keeping if needed on this connection
}
