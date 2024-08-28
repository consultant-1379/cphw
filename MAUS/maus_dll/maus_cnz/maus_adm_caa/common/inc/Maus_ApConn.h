/*
 * Maus_ApConn.h
 *
 *  Created on: Sep 16, 2014
 *      Author: xdtthng
 */

/*
NAME
   File_name: Maus_ApConn.h

COPYRIGHT Ericsson AB, Sweden 2015. All rights reserved.

   The Copyright to the computer program(s) herein is the property of Ericsson AB, Sweden.
   The program(s) may be used and/or copied only with the written permission from
   Ericsson AB or in accordance with the terms and conditions
   stipulated in the agreement/contract under which the program(s) have been
   supplied.

DESCRIPTION
   This class implements the sending of requests for information to Mau Core.
   The request is sent over the message queue encapsulated by MAUConnection class

DOCUMENT NO
   190 89-CAA 109

AUTHOR
   2014-09-14 by DEK/XDTTHNG Thanh Nguyen

SEE ALSO


Revision history
----------------
2014-09-14 Thanh Nguyen Created

*/

#ifndef MAUS_APCONN_H_
#define MAUS_APCONN_H_

#include <stdint.h>
#include "MAUConnection.h"

// Communication between GMServer and Application over a pair of POSIX message queue
// The server side is Application. The client side is GMServer
// -------------------------------------------------
// Message IDs in forward direction
// Return Message ID = Forward Message ID + 1
// --------------------------------------------------
// info[1] = 1, Message Id for Get Operational state
// info[1] = 3, Message Id for Get Quorum Information
// info[1] = 5, Message Id for Set Mask
// info[1] = 7, Message Id for Perform Checksum
// info[1] = 9, Message Id for Set FC State


// Message: Get Operational state
// GMServer ---> Application
// info[0] = 2, Length, including Length
// info[1] = 1, Message Id for Get Operational state
//
// Message: Get Operatonal State Response
// Application ---> GMServer
// info[0] = 4, Length
// info[1] = 2, Message Id for Get Operatonal state response
// info[2] = {0, others}, Error Code, 0 == OK,
// info[3] = Operational State if info[2] == OK.
//            0 == Available, 1 == Not Available
//
// -----------------------------------------------------------------------
//
// Message: Get Quorum Information
// GMServer ---> Application
// info[0] = 2, Length, including Length
// info[1] = 3, Message Id for Get Quorum Information
//
// Message: Get Get Quorum Information Response
// Application ---> GMServer
// info[0] = 7, Length
// info[1] = 4, Message Id for Get Operatonal state response
// info[2] = {0, others}, Error Code, 0 == OK,
// info[3] = connectivity view
// info[4] = quorum view
// info[5] = active index
// info[6] = active mask
//
// -----------------------------------------------------------------------
//
// Message: Set Mask
// GMServer ---> Application
// info[0] = 3, Length, including Length
// info[1] = 5, Message Id for Set Mask
// info[2] = mask to be set
//
// Message: Set Mask Response
// Application ---> GMServer
// info[0] = 4, Length
// info[1] = 6, Message Id for Set Mask Response
// info[2] = {0, others}, Error Code, 0 == OK,
// info[3] = actual mask returned
//
// -----------------------------------------------------------------------
//
// Message: Perform Checksum
// GMServer ---> Application
// info[0] = 3, Length, including Length
// info[1] = 7, Message Id for Perform Checksum
// info[2] = checksum indicator
//
// Message: Perform Checksum Response
// Application ---> GMServer
// info[0] = 3, Length
// info[1] = 8, Message Id for Perform Checksum Response
// info[2] = {0, others}, Error Code, 0 == OK,
//
//-----------------------------------------------------------------------
//
// Message: Set FC State
// GMServer ---> Application
// info[0] = 3, Length, including Length
// info[1] = 9, Message Id for Set FC State
// info[2] = fcState to be set
//
// Message: Set Mask Response
// Application ---> GMServer
// info[0] = 4, Length
// info[1] = 10, Message Id for Set FC State Response
// info[2] = {0, others}, Error Code, 0 == OK,
// info[3] = actual mask returned
//
// -----------------------------------------------------------------------
//
// Message: Install Mibs
// GMServer ---> Application
// info[0] = 3, Length, including Length
// info[1] = 11, Message Id for installing Mibs file
// info[2] = operation indicator
//
// Message: Perform Install Mibs Response
// Application ---> GMServer
// info[0] = 3, Length
// info[1] = 12, Message Id for Perform Checksum Response
// info[2] = {0, others}, Error Code, 0 == OK,
//


class ApConn
{
public:

	// Construction with data passing to MAUConnection
	ApConn(int, int);
	~ApConn();

	// Check if connection is alredy established by MAUConnection
	MauReturnCode::Return connect();

	//void flush();

	// Send message to ApMau over MAUConneciton to read connectivity view
	// Return 0 == OK, 1 == error
	int getOperationalState(int32_t& state);
	int getQuorumInfo(int32_t*, int32_t);
	int setActiveMask(int32_t);
	int performChecksum(int32_t, int32_t);
	int setFCState(int32_t);

	int installMibs(int32_t, int32_t);

private:
	MAUConnection	m_conn;
	int				m_term;
	char			m_buffer[MAUMsgQ::MaxMsgSize];

	MauReturnCode::Return send(char* buffer, size_t& size);
};


#endif /* MAUS_APCONN_H_ */
