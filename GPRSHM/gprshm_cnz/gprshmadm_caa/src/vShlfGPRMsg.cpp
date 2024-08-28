/****************************************************************************/
/**
 *  vShlfGPRMsg.cpp
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
 *  This is class implementation for vShlfGPRMsg class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 */

//----------------------------------------------------------------------------
//  Imported Interfaces, Types & Definitions
//----------------------------------------------------------------------------

#include "vShlfGPRMsg.h"

//----------------------------------------------------------------------------
//  Exported Types & Definitions
//----------------------------------------------------------------------------
void vShlfGPRMsg::setGPRCacheValue(const uint32_t GPRCache[][NO_OF_GPR], int index1, int index2)
{
   assert(index1 == MAX_CP);
   assert(index2 == NO_OF_GPR);

   uint32_t i;
   uint16_t cur_pos = 0;
   uint8_t* start_value = data + SIZEOFHEADER;  //Move to where value section starts

   for (i = 0; i < MAX_CP; i++)
   {
      *(reinterpret_cast<uint32_t*>(start_value + cur_pos)) = htonl(i);      //CPID
      cur_pos += CPID_SIZE;

      //Length
      *(reinterpret_cast<uint16_t*>(start_value + cur_pos)) = htons((CPID_SIZE + GPRCACHE_LEN_SIZE + GPRCACHE_LEN_PER_BLADE));
      cur_pos += GPRCACHE_LEN_SIZE;

      for (int j = 0; j < NO_OF_GPR; j++)
      {
         //List of GPR values
         uint32_t tmp = htonl(GPRCache[i][j]);
         memcpy((start_value + cur_pos), reinterpret_cast<uint8_t*>(&tmp), GPR_VALUE_SIZE);
         cur_pos += 4;
      }
   }

   len += cur_pos;  //Update the message length
   *(reinterpret_cast<uint16_t*>(data + 2)) = htons(len);
}

bool vShlfGPRMsg::getGPRCacheValue(uint32_t GPRCache[][NO_OF_GPR], int index1, int index2)
{
   assert(index1 == MAX_CP);
   assert(index2 == NO_OF_GPR);

   uint16_t cur_pos;
   uint32_t cpid;
   uint16_t GPRListLength;
   uint16_t sizeofGPRList;
   uint16_t msgLen = len;
   bool ret = true;

   for (cur_pos = SIZEOFHEADER; cur_pos < msgLen;)   //Starts from first CPID index
   {
      cpid = ntohl(*(reinterpret_cast<uint32_t*>(data + cur_pos)));
      if (cpid >= MAX_CP)
      {
         ret = false;
         break;             //The rest of the message is not trustable any more, and to simplify
                            //the design, we don't fallback the blades that already updated
      }

      cur_pos += CPID_SIZE;      //Len index

      GPRListLength = ntohs(*(reinterpret_cast<uint16_t*>(data + cur_pos)));

      cur_pos += GPRCACHE_LEN_SIZE;       //List of GPR index

      sizeofGPRList = GPRListLength - CPID_SIZE - GPRCACHE_LEN_SIZE;

      for (int j = 0; j < sizeofGPRList / 4; j++)
      {
         if (j > NO_OF_GPR)
         {
            ret = false;
            break;              //The rest of the message is not trustable any more, and to simplify
                                //the design, we don't fallback the blades that already updated
         }
         uint32_t tmp;
         memcpy(reinterpret_cast<uint8_t*>(&tmp), (data + cur_pos), GPR_VALUE_SIZE);
         GPRCache[cpid][j] = ntohl(tmp);
         cur_pos += GPR_VALUE_SIZE;
      }
   }

   return ret;
}

