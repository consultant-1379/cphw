//  **************************************************************************
//  **************************************************************************
//  ***                                                                    ***
//  ***  COPYRIGHT Ericsson AB 2009 All rights reserved.                   ***
//  ***                                                                    ***
//  ***  The information in this document is the property of Ericsson.     ***
//  ***  Except as specifically authorized in writing by Ericsson, the     ***
//  ***  receiver of this document shall keep the information contained    ***
//  ***  herein confidential and shall protect the same in whole or        ***
//  ***  in part from disclosure and dissemination to third parties.       ***
//  ***  Disclosure and disseminations to the receiver's employees shall   ***
//  ***  only be made on a strict need to know basis.                      ***
//  ***                                                                    ***
//  **************************************************************************
//  **************************************************************************
//
//  Responsible ............ EAB/AZA/DEF
//  Product Number...........(the product number)
//
//< \file
//
//  Local MAU message queue interface
//>

#ifndef MAU_CONNECTION_HXX
#define MAU_CONNECTION_HXX

#include "MAUMsgQ.h"

//----------------------------------------------------------------------------
//  Imported Interfaces, Types & Definitions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Exported Types & Definitions
//----------------------------------------------------------------------------


class MAUConnection
{
public:

   enum ErrType
   {
      Congestion,    // Queue is nearly full
      Normal,        // Queue was congested. Back to nnormal
      ErrnoSet,      // errNo is valid
      Undefined
   };

   enum CBType
   {
      Inbound,
      Outbound 
   };

   //<
   // Callback for queue status. err1 corresponds to ErrCode.
   //>
   typedef void (*mqError)(int ErrType, int errNo);

   MAUConnection(int instance=1, const char* = 0);
   ~MAUConnection();

   MauReturnCode::Return connect();
   void disconnect();
   MauReturnCode::Return send(const char* msg, size_t msg_len);
   MauReturnCode::Return poll(char* msg, size_t* msg_len);
   MauReturnCode::Return registerEvent(int CBType,  mqError cb);
   MauReturnCode::Return getDescriptor(int CBType, mqd_t& val) const;

   size_t getMaxCptSigSize();
   void checkStatus();
   bool connected() const;
   
private:
   bool    m_isAttached;
   MAUMsgQ m_inboundQueue;
   MAUMsgQ m_outboundQueue;
   mqError m_callbackIn;
   mqError m_callbackOut;
   bool    m_congestionIn;
   bool    m_congestionOut;

   mqd_t   m_inDesc;
   mqd_t   m_outDesc;
};


inline bool MAUConnection::connected() const
{
   return m_isAttached;
}

#endif
