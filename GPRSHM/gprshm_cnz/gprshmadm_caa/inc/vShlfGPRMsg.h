/****************************************************************************/
/**
 *  vShlfGPRMsg.h
 *
 *  190 89-CAA 109 1638
 *
 *  COPYRIGHT Ericsson AB 2015-2016. All rights reserved.
 *
 *  The Copyright of the computer program(s) herein is the property of
 *  Ericsson AB, Sweden. The program(s) may be used and/or copied only
 *  with the written permission from Ericsson AB or in accordance
 *  with the terms and conditions stipulated in the agreement/contract
 *  under which the program(s) have been supplied.
 */
/****************************************************************************/

/**
 *  Description:
 *  ------------
 *  This file contains the vShlfGPRMsg class declarations and inline method
 *  definitions.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 */

#ifndef _VSHLFGPRMSG_H
#define _VSHLFGPRMSG_H

#include <cassert>
#include "stdint.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <netinet/in.h>

#include "CPHW_GPRSHM_Definitions.h"

using namespace std;

class vShlfGPRMsg
{
public:
   enum MessageID
   {
      GPR_WRITE = 128,
      GPR_COMPLETE_CACHE_UPDATE = 129,
      GPR_READ_REQ = 130,
      GPR_READ_RES = 131
   };

   uint8_t  getID(void) const    { return id; }
   uint8_t  getVer(void) const   { return ver; }
   uint16_t getLen(void) const   { return len; }
   uint8_t* getValue(void) const { return const_cast<uint8_t*>(data + SIZEOFHEADER); }
   uint8_t* getData(void) const  { return const_cast<uint8_t*>(data); }

   void setID(uint8_t _id)          { id = _id; data[MSG_ID_OFFSET] = _id; }
   void setVer(uint8_t _ver)        { ver = _ver; data[PROT_VER_OFFSET] = _ver; }
   void setLength(uint16_t _len);

   void setGPRCacheValue(const uint32_t GPRCache[][NO_OF_GPR], int index1 = MAX_CP, int index2 = NO_OF_GPR);

   bool getGPRCacheValue(uint32_t GPRCache[][NO_OF_GPR], int index1 = MAX_CP, int index2 = NO_OF_GPR);

   void targetCPID(uint32_t thetargetCPID); // Set target CP identity to be manipulated (e.g. write a GPR).
   uint32_t targetCPID();                   // Get target CP identity to be manipulated (e.g. write a GPR).

   void senderCPID(uint32_t thesenderCPID);
   uint32_t senderCPID();

   void GPRAddr(uint32_t);          // Set GPR address to be write/read
   uint32_t GPRAddr();              // Get GPR address to be write/read

   void result(uint32_t);           // Set GPR address to be write/read
   uint32_t result();               // Get GPR address to be write/read

   void resValue(uint32_t);         // Set GPR address to be write/read
   uint32_t resValue();             // Get GPR address to be write/read

   void GPRValue(uint32_t);         // Set GPR value
   uint32_t GPRValue();             // Get GPR value

   vShlfGPRMsg(uint8_t _len, uint8_t* _data);
   vShlfGPRMsg();
   ~vShlfGPRMsg();

private:
   uint8_t  id;
   uint8_t  ver;
   uint16_t len;
   uint8_t  data[MAX_MSG_DATA]; /* Hold the GPR message in Big-indian (Network byte order) */
};

//----------------------------------------------------------------------------
//<
//  Creates an vShlfGPRMsg instance which can interpret an inbound raw FTm
//  message.
//
//  \pin  theComSize  The size (in bytes) of the communication buffer.
//  \pin  theComData  A pointer to a communication buffer large enough to
//                    include the requested primitive. Note that _data
//>
//----------------------------------------------------------------------------
inline vShlfGPRMsg::vShlfGPRMsg(uint8_t _len, uint8_t* _data) :
   len(_len)
{
   assert((_data != NULL) &&
          "No message buffer was included!");

   assert((len >= SIZEOFHEADER) &&
          "The message length is to small!");

   assert((len < MAX_MSG_DATA) &&
          "The message length exceeds the MAX!");

   id = *(_data + MSG_ID_OFFSET);
   ver = *(_data + PROT_VER_OFFSET);

   len = ntohs(*(reinterpret_cast<uint16_t*>(_data + MSG_LEN_OFFSET)));
   memcpy(data, _data, len);
}

inline vShlfGPRMsg::vShlfGPRMsg() :
   len(SIZEOFHEADER)
{
   *(reinterpret_cast<uint16_t*>(data + MSG_LEN_OFFSET)) = htons(len);
}

inline vShlfGPRMsg::~vShlfGPRMsg()
{
}

inline void vShlfGPRMsg::setLength(uint16_t _len)
{
   len = _len;
   *(reinterpret_cast<uint16_t*>(data + MSG_LEN_OFFSET)) = htons(_len);
}

//----------------------------------------------------------------------------
//<
//  This method is used to set the target CP identity field
//
//  \pin  thetargetCPID  The CP identity to be manipulated (e.g. write a GPR).
//>
//----------------------------------------------------------------------------
inline void vShlfGPRMsg::targetCPID(uint32_t thetargetCPID)
{
   //TODO: assert the messages with receiver CPID field here

   *(reinterpret_cast<uint32_t*>(data + TARGET_CPID_OFFSET)) = htonl(thetargetCPID);
}

//----------------------------------------------------------------------------
//<
//  This method is used to get the target CP identity field
//
//  \return The CP identity to be manipulated (e.g. write a GPR).
//>
//----------------------------------------------------------------------------
inline uint32_t vShlfGPRMsg::targetCPID()
{
   //TODO: assert the messages with receiver CPID field here
   return ntohl(*(reinterpret_cast<uint32_t*>(data + TARGET_CPID_OFFSET)));
}

//----------------------------------------------------------------------------
//<
//  This method is used to set the sender CP identity field
//
//  \pin  thesenderCPID  The Sender CP identity
//>
//----------------------------------------------------------------------------
inline void vShlfGPRMsg::senderCPID(uint32_t thesenderCPID)
{
   //TODO: assert the messages with sender CPID field here

   *(reinterpret_cast<uint32_t*>(data + SENDER_CPID_OFFSET)) = htonl(thesenderCPID);
}

//----------------------------------------------------------------------------
//<
//  This method is used to get the sender CP identity field
//
//  \return   The Sender CP identity.
//>
//----------------------------------------------------------------------------
inline uint32_t vShlfGPRMsg::senderCPID()
{
   //TODO: assert the messages with sender CPID field here
   return ntohl(*(reinterpret_cast<uint32_t*>(data + SENDER_CPID_OFFSET)));
}

inline void vShlfGPRMsg::GPRAddr(uint32_t GPRAddr)
{
   //TODO: assert the messages with sender CPID field here

   *(reinterpret_cast<uint32_t*>(data + GPR_ADDR_OFFSET)) = htonl(GPRAddr);
}

//----------------------------------------------------------------------------
//<
//  This method is used to get the GPR Address
//
//  \return   GPR Address.
//>
//----------------------------------------------------------------------------
inline uint32_t vShlfGPRMsg::GPRAddr()
{
   return ntohl(*(reinterpret_cast<uint32_t*>(data + GPR_ADDR_OFFSET)));
}

inline uint32_t vShlfGPRMsg::result()
{
   return ntohl(*(reinterpret_cast<uint32_t*>(data + GPR_RESULT_OFFSET)));
}

inline void vShlfGPRMsg::result(uint32_t GPRValue)
{
   //TODO: assert the messages with sender CPID field here

   *(reinterpret_cast<uint32_t*>(data + GPR_RESULT_OFFSET)) = htonl(GPRValue);
}

inline uint32_t vShlfGPRMsg::resValue()
{
   return ntohl(*(reinterpret_cast<uint32_t*>(data + GPR_RESVALUE_OFFSET)));
}

inline void vShlfGPRMsg::resValue(uint32_t GPRValue)
{
   //TODO: assert the messages with sender CPID field here

   *(reinterpret_cast<uint32_t*>(data + GPR_RESVALUE_OFFSET)) = htonl(GPRValue);
}

inline void vShlfGPRMsg::GPRValue(uint32_t GPRValue)
{
   //TODO: assert the messages with sender CPID field here

   *(reinterpret_cast<uint32_t*>(data + GPR_VALUE_OFFSET)) = htonl(GPRValue);
}

//----------------------------------------------------------------------------
//<
//  This method is used to get the GPR value
//
//  \return   GPR value.
//>
//----------------------------------------------------------------------------
inline uint32_t vShlfGPRMsg::GPRValue()
{
   return ntohl(*(reinterpret_cast<uint32_t*>(data + GPR_VALUE_OFFSET)));
}

inline ostream& operator<<(ostream& o, vShlfGPRMsg& theMessage)
{
   o << setfill('0');
   o << endl << "<vShlfGPRMsg>----------------------------------------" << endl;
   o << "Primitive..............: "   << static_cast<int>(theMessage.getID()) << endl;
   o << "Version................: "   <<  static_cast<int>(theMessage.getVer()) << endl;
   o << "Buffer size (in bytes).: "   <<  theMessage.getLen() << endl;
   if (theMessage.getID() == vShlfGPRMsg::GPR_WRITE)
   {
      o << "Sender CPID............: " << theMessage.senderCPID() << endl;
      o << "Target CPID............: " << theMessage.targetCPID() << endl;
      o << "GPR address............: " << theMessage.GPRAddr() << endl;
      o << "GPR value..............: 0x" << std::hex << setw(8) << theMessage.GPRValue() << std::dec << endl;
   }
   else if (theMessage.getID() == vShlfGPRMsg::GPR_READ_REQ)
   {
      o << "Sender CPID............: " << theMessage.senderCPID() << endl;
      o << "Target CPID............: " << theMessage.targetCPID() << endl;
      o << "GPR address............: " << theMessage.GPRAddr() << endl;
   }
   if (theMessage.getID() == vShlfGPRMsg::GPR_READ_RES)
   {
      o << "Sender CPID............: " << theMessage.senderCPID() << endl;
      o << "Target CPID............: " << theMessage.targetCPID() << endl;
      o << "GPR address............: " << theMessage.GPRAddr() << endl;
      o << "GPR result.............: " << std::dec << theMessage.GPRValue() << endl;
      o << "GPR response value.....: 0x" << std::hex << setw(8) << theMessage.resValue() << std::dec << endl;
   }
   else if (theMessage.getID() == vShlfGPRMsg::GPR_COMPLETE_CACHE_UPDATE)
   {
      o << "GPR cache value" << endl;
      o << "CPID      Len  R8        R9        R10       R11       R12       R13       R14       R15       " << endl;
      uint8_t* tmp = theMessage.getData();
      for (int i = SIZEOFHEADER; i < theMessage.getLen(); i += 38)
      {
         for (int j = i; j < ((i + 38) <= theMessage.getLen() ? (i + 38) : theMessage.getLen()); ++j)
         {
            o << std::hex << setw(2) << (static_cast<int>(tmp[j] & 0x00FF));
            if (j % 2 != 0)
               o << " ";
         }
         o << endl;
      }
   }
   o << endl << "</vShlfGPRMsg>----------------------------------------" << endl;

   return o;
}

#endif /* _VSHLFGPRMSG_H */

