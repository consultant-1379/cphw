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
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <errno.h>

#include "MAUMsgQ.h"

//----------------------------------------------------------------------------
//  Class Variable Initializations
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Ctor for MAUMsgQ
//
//  \pin idx Queue index 
//>
//----------------------------------------------------------------------------
MAUMsgQ::MAUMsgQ(int instance, const char* name)
{ 
   m_mqDesc = -1; 

   if (name == NULL)
   {
	   snprintf(m_in, sizeof(m_in), "/MAUS_in_mq_%d", instance);
   	   snprintf(m_out, sizeof(m_out), "/MAUS_out_mq_%d", instance);
   }
   else
   {
	   snprintf(m_in, sizeof(m_in), "/%s_in_%d", name, instance);
   	   snprintf(m_out, sizeof(m_out), "/%s_out_%d", name, instance);

   }
    
   m_mausInBoundQueueType = m_in;
   m_mausOutBoundQueueType = m_out;
}

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Place a message in the message queue.
//
//  \pin msg      Data to put into message queue
//  \pin msg_len  Size of data, maximum message size is MaxMsg
//
//  \retval MauReturnCode::OK              Success
//  \retval MauReturnCode::EBAD_ARG        Message too large
//  \retval MauReturnCode::EILLEGAL_STATE  Queue not initialised ok.
//  \retval MauReturnCode::EINTERNAL       Other fault
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUMsgQ::send(const char* msg, size_t msg_len)
{
   int prio = 0;
   if (m_mqDesc == (mqd_t) -1)
   {
      return MauReturnCode::EILLEGAL_STATE;
   }

   if (msg_len > MaxMsgSize || msg == NULL)
   {
      return MauReturnCode::EBAD_ARG;
   }

   if (mq_send (m_mqDesc, msg, msg_len, prio))
   {
      int eno = errno;
      switch(eno)
      {
         case EAGAIN: return MauReturnCode::FAIL; //The queue was full try later
         case EMSGSIZE: return MauReturnCode::EBAD_ARG;
         default:
            return MauReturnCode::EINTERNAL;
      }
   }
   return MauReturnCode::OK;
}

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  MauReturnCode::Return a message from the message queue.
//  If there is no message available MauReturnCode::FAIL is returned.
//
//  \pin msg        Pointer to buffer where message will be put
//  \pinout msg_len Size of data buffer in, output the number of bytes written to
//                  the buffer
//
//  \retval MauReturnCode::OK       Success
//  \retval MauReturnCode::FAIL     Queue was empty
//  \retval MauReturnCode::EBAD_ARG Too small buffer supplied
//  \retval MauReturnCode::EILLEGAL_STATE  Queue not initialised ok.
//  \retval MauReturnCode::EINTERNAL   Other fault
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUMsgQ::receive(char* msg, size_t* msg_len)
{
   unsigned int prio;
   ssize_t sz;

   if(m_mqDesc == (mqd_t) -1)
   {
      return MauReturnCode::EILLEGAL_STATE;
   }
   // Msg len must be at least 12 bytes i.e CPT header size
   if(!msg_len || *msg_len < 12)
   {
      return MauReturnCode::EBAD_ARG;
   }

   if((sz = mq_receive (m_mqDesc, msg, *msg_len, &prio)) == -1)
   {
      int eno = errno;
      *msg_len = 0;
      if(eno == EAGAIN)
      {
         return MauReturnCode::FAIL; //The queue was empty
      }
      switch(eno)
      {
         case EMSGSIZE: return MauReturnCode::ESIZE;
         default:       return MauReturnCode::EINTERNAL;
      }
   }
   *msg_len = (unsigned int)sz;
   return MauReturnCode::OK;
}


//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Attach to existing message queue.
//
//  \pin name    Queu type
//
//  \retval MauReturnCode::OK Success
//  \retval MauReturnCode::ENOT_ENOUGH_RESOURCES Out of memory
//  \retval MauReturnCode::EILLEGAL_PERMISSION   Not allowed to open queue
//  \retval MauReturnCode::ENODEVICE              No such queue
//  \retval MauReturnCode::EINTERNAL                Other error
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUMsgQ::attach(enum QueueType type)
{
   if((m_mqDesc = mq_open(queueName(type), O_RDWR | O_NONBLOCK)) == -1)
   {
      int eno = errno;
      switch(eno)
      {
         case ENOMEM: return MauReturnCode::ENOT_ENOUGH_RESOURCES;
         case EACCES: return MauReturnCode::EILLEGAL_PERMISSION;
         case ENOENT: return MauReturnCode::ENODEVICE;
         default:     return MauReturnCode::EINTERNAL;
      }
   }

   return MauReturnCode::OK;
}

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Detach from existing message queue.
//
//  \retval MauReturnCode::OK        Success
//  \retval MauReturnCode::EBAD_ARG  Invalid mqDesc
//  \retval MauReturnCode::EINTERNAL Other error
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUMsgQ::detach()
{
   if (mq_close(m_mqDesc) != 0)
   {
      int eno = errno;
      switch(eno)
      {
         case EBADF:  return MauReturnCode::EBAD_ARG;
         default:     return MauReturnCode::EINTERNAL;
      }
   }
   return MauReturnCode::OK;
}


//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//   Get the name of a pre-defined queue
//
//  \pin type    Queue identifier
//
//   \returns Name of queue
//>
//----------------------------------------------------------------------------
const char* MAUMsgQ::queueName(enum QueueType type)
{
   switch(type)
   {
      case LocalMAUInboundQ:
         return m_mausInBoundQueueType;
      case LocalMAUOutboundQ:
         return m_mausOutBoundQueueType;
   }
   return NULL;
}

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Generate a new message queue with the name supplied.
//
//  \pin name    Name of message queue to create, must begin with a slash (/).
//
//  \retval MauReturnCode::OK Success
//  \retval MauReturnCode::ENOT_ENOUGH_RESOURCES Out of memory
//  \retval MauReturnCode::ETOO_MANY   Queue is already existent
//  \retval MauReturnCode::EINTERNAL   Other error
//  \retval MauReturnCode::FAIL        Unable create queue
//>
//----------------------------------------------------------------------------

// Allow MAUCore to reconnect to queue if exist
#define ALLOW_REUSE 1

MauReturnCode::Return MAUMsgQ::create(enum QueueType type)
{
   struct mq_attr attr;
   attr.mq_maxmsg = 10; //10 is default on build server
   attr.mq_msgsize = MaxMsgSize;
   attr.mq_flags = O_NONBLOCK;

   strncpy(m_mqName, queueName(type), MaxNameLen-1);
   m_mqName[MaxNameLen-1] = '\0';

   // Create queue and fail if already existent
   if((m_mqDesc = mq_open (m_mqName, 
                           O_RDWR | O_CREAT | 
#if !defined(ALLOW_REUSE)
                           O_EXCL | 
#endif
                           O_NONBLOCK,
                           S_IRWXU | S_IRWXG | S_IRWXO, &attr)) == -1)
   {
      int eno = errno;

      switch(eno)
      {
#if !defined(ALLOW_REUSE)
         case EEXIST: return MauReturnCode::ETOO_MANY;
#endif
         case ENOMEM: return MauReturnCode::ENOT_ENOUGH_RESOURCES;
         default:     return MauReturnCode::EINTERNAL;
      }
   }

   return MauReturnCode::OK;
}

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Remove the message Queue (if this process created it). Queue will be
//  removed once all references to it have been closed.
//
//  \pin    unlink   Indicate if queue should be unlinked
//
//  \retval OK       Removed queue ok.
//  \retval MauReturnCode::EILLEGAL_PERMISSION   Not allowed to remove queue
//  \retval MauReturnCode::NODEVICE              No such queue
//  \retval EINTERNAL   Failed to remove queue
//>
//----------------------------------------------------------------------------
MauReturnCode::Return MAUMsgQ::destroy(bool unlink)
{
   int ret;
   ret = mq_close(m_mqDesc);
 
   if (!unlink)
   {
      if (ret != -1)
      {
         return MauReturnCode::OK;
      }
      else
      {
         // Error code close will fail is EBADF
         return MauReturnCode::EBAD_ARG;
      }
   }
  
   if ((mq_unlink(m_mqName)) == -1)
   {
      int eno = errno;
      switch(eno)
      {
         case EACCES: return MauReturnCode::EILLEGAL_PERMISSION;
         case ENOENT: return MauReturnCode::ENODEVICE;
         default:     return MauReturnCode::EINTERNAL;
      }
   }
   return MauReturnCode::OK;
}

//----------------------------------------------------------------------------
//< \ingroup MAUCore
//
//  Get the file descriptor for the message queue. It is only intended to be
//  used in select, poll or epoll.
//
//  \return The file descriptor, or -1 if not attached.
//>
//----------------------------------------------------------------------------
mqd_t MAUMsgQ::getFileDescriptor() const
{
   return m_mqDesc;
}


