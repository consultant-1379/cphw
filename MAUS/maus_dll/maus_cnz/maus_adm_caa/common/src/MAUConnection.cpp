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

//----------------------------------------------------------------------------
//  Imported Interfaces, Types & Definitions
//----------------------------------------------------------------------------
#include <errno.h>
#include <mqueue.h>
#include <cstddef>
#include "MAUConnection.h"
#include "MAUMsgQNames.h"
#include <unistd.h>
//----------------------------------------------------------------------------
//  Class Variable Initializations
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  The constructor for MAUConnection 
//
//>
//----------------------------------------------------------------------------
MAUConnection::MAUConnection(int instance, const char* name) :
   m_isAttached(false),
   //m_inboundQueue(instance),
   //m_outboundQueue(instance),
   m_inboundQueue(instance, name),
   m_outboundQueue(instance, name),
   m_callbackIn(NULL),
   m_callbackOut(NULL),
   m_congestionIn(false),
   m_congestionOut(false),
   m_inDesc(-1),
   m_outDesc(-1)
{
}

//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  The destructor for MAUConnection
//
//>
//----------------------------------------------------------------------------
MAUConnection::~MAUConnection()
{
   m_isAttached = false;
   m_callbackOut = NULL;
   m_callbackIn = NULL;
   m_congestionIn = false;
   m_congestionOut = false;
   m_inDesc = -1;
   m_outDesc = -1;
   m_outboundQueue.detach();
   m_inboundQueue.detach();
}

MauReturnCode::Return MAUConnection::getDescriptor(int queueType, mqd_t& val) const
{
   if (queueType==MAUMsgQ::LocalMAUInboundQ && m_isAttached)
   {
      val = m_inDesc;
      return MauReturnCode::OK;
   }

   if (queueType==MAUMsgQ::LocalMAUOutboundQ && m_isAttached)
   {
      val = m_outDesc;
      return MauReturnCode::OK;
   }

   return MauReturnCode::FAIL;
}

//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  Connects to the inbound and outbound queue 
//
//
//  \retval MauReturnCode::OK           Successful
//  \retval MauReturnCode::FAIL         Failed
//  \retval MauReturnCode::ENOT_FOUND   Message queue not ready. Retry
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUConnection::connect()
{
   MauReturnCode::Return ret = MauReturnCode::FAIL;
 
   bool inboundQAttach=false;
   bool outboundQAttach=false;
   m_isAttached = false;

   for( int i =0;i<10;i++ )
   {
      // Connect client inbound to local MAU outbound queue
      ret = m_inboundQueue.attach(MAUMsgQ::LocalMAUOutboundQ);
      if (ret != MauReturnCode::OK)
      {
        inboundQAttach=false;
        usleep(1000);
        continue;
      }

     inboundQAttach=true;
     break;
   }


  if( inboundQAttach ==false ) return ret;


   for( int i =0;i<10;i++ )
   {

     // Connect client outbound to local MAU inbound queue
     ret = m_outboundQueue.attach(MAUMsgQ::LocalMAUInboundQ);
     if (ret != MauReturnCode::OK)
     {
       outboundQAttach=false;
       usleep(1000);
      continue;
     }
    
    outboundQAttach=true;
    break;
  }


  if( outboundQAttach ==false ) return ret;


   m_inDesc = m_inboundQueue.getFileDescriptor();
   m_outDesc = m_outboundQueue.getFileDescriptor();

   m_isAttached = true;
   return MauReturnCode::OK;
}


//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  Detach from to the inbound and outbound queue
//
//>
//----------------------------------------------------------------------------
void MAUConnection::disconnect()
{
   m_inboundQueue.detach();
   m_outboundQueue.detach();
   m_isAttached = false;
}

//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  Check the health of the queue and issue callback if necessary
//
//>
//----------------------------------------------------------------------------
void MAUConnection::checkStatus()
{
   struct mq_attr attr;
   int ret;

   if (m_callbackIn)
   { 
	// Check inbound queue
	ret = mq_getattr(m_inboundQueue.getFileDescriptor(), &attr);

	if (ret < 0)
	{
	   m_callbackIn(MAUConnection::ErrnoSet, errno);
	}

	if (attr.mq_curmsgs >= attr.mq_maxmsg-2 && !m_congestionIn)
	{
	  // Msg is piling up in queue. Most like due to congestion.
	  m_congestionIn = true;
	  m_callbackIn(MAUConnection::Congestion,0);
	}

	if (attr.mq_curmsgs <= 2 && m_congestionIn)
	{
	  m_congestionIn = false;
	  m_callbackIn(MAUConnection::Normal,0);
	}
   }

   if (m_callbackOut)
   { 
	// Check outbound queue
	ret = mq_getattr(m_outboundQueue.getFileDescriptor(), &attr);

	if (ret < 0)
	{
	   m_callbackOut(MAUConnection::ErrnoSet, errno);
	}

	if (attr.mq_curmsgs >= attr.mq_maxmsg-2 && !m_congestionOut)
	{
	  // Msg is piling up in queue. Most like due to congestion.
	  m_congestionOut = true;
	  m_callbackOut(MAUConnection::Congestion,0);
	}

	if (attr.mq_curmsgs <= 2 && m_congestionOut)
	{
	  m_congestionOut = false;
	  m_callbackOut(MAUConnection::Normal,0);
	}
   }
}
//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//   Poll for incoming message
//
//  \pin    msg     Pointer to buffer where message will be put
//  \pinout msg_len Size of data buffer in, output the number of bytes written
//                  to the buffer
//
//  \retval MauReturnCode::OK           Successful
//  \retval MauReturnCode::FAIL         Failed
//  \retval MauReturnCode::ENOT_FOUND   Message queue not ready. Retry
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUConnection::poll(char* msg, size_t* msg_len)
{
   checkStatus();

   return m_inboundQueue.receive(msg, msg_len);
}

//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//   Send outgoing message
//
//  \pin msg      Data to put into message queue
//  \pin msg_len  Size of data, maximum message size is MaxMsg
//
//  \retval MauReturnCode::OK           Successful
//  \retval MauReturnCode::FAIL         Failed
//  \retval MauReturnCode::ENOT_FOUND   Message queue not ready. Retry
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUConnection::send(const char* msg, size_t msg_len)
{
   return m_outboundQueue.send(msg, msg_len);
}

//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  Get max. length for CPT signal 

//  \retval The length
//>
//----------------------------------------------------------------------------

size_t MAUConnection::getMaxCptSigSize()
{
   return MAUMsgQ::getMaxMsgSize();
}


//----------------------------------------------------------------------------
//< \ingroup MAUAPI
//
//  Register callback for queue events
//
//  \pin CBType         Callback type 
//  \pin cb             The callback function
//
//  \retval Os::OK      Succesful
//  \retval Osi::FAIL   Unable to register
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUConnection::registerEvent(int CBType, mqError cb)
{
   if (CBType == Inbound && !m_callbackIn)
   {
      m_callbackIn = cb;
      return MauReturnCode::OK;
   }

   if (CBType == Outbound && !m_callbackOut)
   {
      m_callbackOut = cb;
      return MauReturnCode::OK;
   }
   return MauReturnCode::FAIL;
}

