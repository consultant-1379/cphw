//  **************************************************************************
//  **************************************************************************
//  ***                                                                    ***
//  ***  COPYRIGHT Ericsson AB 2014 All rights reserved.                   ***
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
//  Responsible ............ TEI/XPN
//
//< \file
//
//  Local MAU message queue interface
//>

#ifndef MAU_MSG_Q_HXX
#define MAU_MSG_Q_HXX

#include <mqueue.h>

//----------------------------------------------------------------------------
//  Imported Interfaces, Types & Definitions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Exported Types & Definitions
//----------------------------------------------------------------------------

namespace MauReturnCode
{
   enum Return
   {
      ENOT_FOUND            = -13,
      ESIZE                 = -12,
      ENOT_ENOUGH_RESOURCES = -11,
      EILLEGAL_STATE        = -10,
      EILLEGAL_PERMISSION   = -9,
      ENODEVICE       = -8,
      ETOO_MANY       = -7,
      EBAD_CONFIG     = -6,
      EBAD_ARG        = -5,
      ETIMEOUT        = -4,
      EBUSY_TRY_AGAIN = -3,
      EINTERNAL       = -2,
      FAIL            = -1,
      UNDEFINED       =  0,
      OK              =  1
   };
}



class MAUMsgQ
{
public:

   enum
   {
      MaxMsgSize = 252, ///<Max CPT signal length, includes header is 252 bytes.
      MaxNameLen = 32   ///< Max length of message queue name
   };

   enum QueueType ///< Some predefined queue names
   {
      LocalMAUInboundQ,  ///< Data towards the inbound queue
      LocalMAUOutboundQ  ///< Data from the outbound queue
   };

   enum QEvent
   {
      Detached,   ///< Previously attached queue is now detached
      NoQueue     ///< Unable to attach to queue
   };

   MAUMsgQ(int instance = 1, const char* = 0); 
   virtual ~MAUMsgQ() {};

   MauReturnCode::Return send(const char* msg, size_t msg_len);
   MauReturnCode::Return receive(char* msg, size_t* msg_len);
   MauReturnCode::Return attach(enum QueueType type);
   MauReturnCode::Return detach();
   MauReturnCode::Return create(enum QueueType type);
   MauReturnCode::Return setProtection(mode_t mode);
   MauReturnCode::Return destroy(bool unlink=false);
   MauReturnCode::Return eventCallback();
   static size_t getMaxMsgSize() { return MaxMsgSize; };

   mqd_t getFileDescriptor() const;

private:
   char* m_mausInBoundQueueType;
   char* m_mausOutBoundQueueType;

   char m_in[MaxNameLen];     ///< String name for in msg queue
   char m_out[MaxNameLen];    ///< String name for out msg queue

   char m_mqName[MaxNameLen]; ///< Storage for queue name
   mqd_t m_mqDesc;            ///< Message queue descriptor

   const char* queueName(enum QueueType type);

};
#endif
