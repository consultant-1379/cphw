/*
NAME
    File_name: GMClent.h

Ericsson AB

    COPYRIGHT Ericsson AB, Sweden 2000. All rights reserved.

    The Copyright to the computer program(s) herein is the property of
    Ericsson AB, Sweden. The program(s) may be used and/or
    copied only with the written permission from Ericsson AB
    or in accordance with the terms and conditions stipulated in the
    agreement/contract under which the program(s) have been supplied.

DESCRIPTION

	This class provides DSD based communication between CpMauM and MAUS service on both
	Active and Passive nodes.

DOCUMENT NO
    190 89-CAA 109

AUTHOR
    2015-02-11 by XDT/DEK xdtthng


SEE ALSO
    -

Revision history
----------------
2015-02-11 xdtthng Created

*/


#ifndef _GM_MAUS_CLIENT_H_
#define _GM_MAUS_CLIENT_H_

#include <boost/thread/thread.hpp>

#include "ACS_DSD_Client.h"
#include "ACS_DSD_Session.h"
//#include "maus_event.h"
//#include "AtomicFlag.h"
#include <string>

class GMClient
{
public:
	enum ResultCode {
		RC_OK,						// Command was executed
		RC_NOCONTACT,				// No contact with server
		RC_INTERNAL_ERROR,			// Error due to DSD, Linux ... 
		RC_FUNCTION_NOT_SUPPORTED
    };

	// Ctor and Dtor to establish DSD connection either Active or
	// Passive node
    GMClient(const char* name, const char* domain, int);
    ~GMClient();

    // Establish DSD session to either Active or Passive node
    bool connect();

    // Send message to either Active or Passive node
    ssize_t send(void* buf, size_t size);

    // Get current result code
    int getResultCode();

    // Get side
    int getSide();

    
private:
    ResultCode      mResultCode;
    ACS_DSD_Client  mClient;
    ACS_DSD_Session mSession;
    std::string		mName;
    std::string 	mDomain;
    int				mState;
    bool			mConn;
    int				mSide;
};

inline
int GMClient::getResultCode()
{
    return mResultCode; 
}

#endif
