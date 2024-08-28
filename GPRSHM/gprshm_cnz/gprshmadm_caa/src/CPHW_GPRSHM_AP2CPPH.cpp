/****************************************************************************/
/**
 *  CPHW_GPRSHM_AP2CPPH.cpp
 *
 *  190 89-CAA 109 1638
 *
 *  COPYRIGHT Ericsson AB 2015-2017. All rights reserved.
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
 *  This is class implementation for AP-CP communication protocol PROT1 over FTm.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-01-12  xjoschu  Added partition recovery.
 *  2017-02-10  xjoschu  Clean-up following internal review.
 *  2017-02-15  xjoschu  Clean-up following basic test.
 *  2017-05-09  xdargas  Changes due to TR HV77554.
 */

#include "CPHW_GPRSHM_AP2CPPH.h"
#include "CPHW_GPRSHM_Trace.h"
#include "CPHW_GPRSHM_Event.h"

#include "stdint.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>      // std::setw
#include <sys/time.h>
#include <assert.h>

using namespace std;
const char* CPHW_GPRSHM_AP2CPPH::s_slotFileName = "/etc/opensaf/slot_id";
const char* CPHW_GPRSHM_AP2CPPH::s_intf[] = {"eth3", "eth4"};
uint32_t CPHW_GPRSHM_AP2CPPH::s_GPRCache[MAX_CP][NO_OF_GPR] = { { 0 } };

/**
 * @brief Constructor
 */
CPHW_GPRSHM_AP2CPPH::CPHW_GPRSHM_AP2CPPH() :
   m_ftmMedia("GPRSHM"),
   m_ftmGroupName("GPRSHM"),
   m_state(NEW),
   m_bitMap(0),
   m_timeJoinedFTmGroup(0),
   m_gprFile(NULL),
   m_numberOfExpectedGPRCacheUpdates(0),
   m_GPRCacheUpdateTimer(0)
{
   /**< Init m_endpoints array*/
   for (int i = 0; i < NumberOfEndPoints; i++)
   {
      m_endpoints[i].m_inView = false;
      m_endpoints[i].m_updated = false;
   }
}

/**
 * @brief Destructor
 */
CPHW_GPRSHM_AP2CPPH::~CPHW_GPRSHM_AP2CPPH()
{
   this->m_ftmMedia.destroy();
}

/**
 * @brief AP2CPPH_STATE in readable text
 */
const char* CPHW_GPRSHM_AP2CPPH::stateName(AP2CPPH_STATE state)
{
   static const char *NAMES[] = {
      "NEW",
      "STARTED",
      "JOINED_FTM_GROUP",
      "WAITING_FOR_GRP_CACHE",
      "GPR_CACHE_MASTER",
      "STOPPED",
      "FAILED",
      "UNKNOWN"
   };
   static const unsigned MAX_INDEX = (sizeof(NAMES) / sizeof(NAMES[0])) - 1;
   unsigned index = static_cast<unsigned>(state);
   return NAMES[(index < MAX_INDEX) ? index : MAX_INDEX];
}

/**
 * @brief AP2CPPH_STATE in readable text
 */
void CPHW_GPRSHM_AP2CPPH::setState(AP2CPPH_STATE state)
{
   TRACE((LOG_LEVEL_INFO, "AP2CPPH.state changes from %s to %s", 0, stateName(m_state), stateName(state)));
   m_state = state;
}
/**
 * @brief This callback function will be called from FTm  when receiving message
 *
 * @param msg Message buffer
 * @param msg_size Message size
 * @return N/A
 */
void CPHW_GPRSHM_AP2CPPH::onFTmMsgReceived(const char* msg, size_t msg_size)
{
   vShlfGPRMsg theMessage(msg_size, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)));

   ostringstream trace;
   trace << FILENAME << " " << __FUNCTION__  << " from " << m_ftmMedia.lastSenderName() << ": " << endl;
   trace << theMessage << endl;
   TRACE((LOG_LEVEL_TRACE, "%s", 0, trace.str().c_str()));

   /**
    * Handle receive message from FTm
    */
   switch (theMessage.getID())
   {
      case vShlfGPRMsg::GPR_COMPLETE_CACHE_UPDATE:
      {
         if (m_GPRCacheUpdateTimer != 0)
         {
            if (m_numberOfExpectedGPRCacheUpdates > 0)
            {
               int sender_id;
               const char *sender_name = m_ftmMedia.lastSenderName();

               // Update the GPR cache for the new endpoints in the view for a
               // GPR cache update received from a sender that was not in
               // the previous view. We only need to update the GPR cache
               // for the new end points. And we only trust the GPR cache
               // from the new end points because only they know when their
               // cache is valid.
               uint32_t gpr_cache[MAX_CP][NO_OF_GPR];
               theMessage.getGPRCacheValue(gpr_cache);

               bool sender_was_in_prev_view = false;
               if (memcmp(sender_name, "APA", 3) == 0)
               {
                  sender_was_in_prev_view = wasEndpointInPrevView(APA);
               }
               else if (memcmp(sender_name, "APB", 3) == 0)
               {
                  sender_was_in_prev_view = wasEndpointInPrevView(APB);
               }
               else if ((sscanf(sender_name, "BC%d", &sender_id) == 1) &&
                        (sender_id >= BC0) && (sender_id <= BC63))
               {
                  sender_was_in_prev_view = wasEndpointInPrevView((EndPoint)sender_id);
               }
               else
               {
                  TRACE((LOG_LEVEL_WARN,
                         "Ignore cache update message due to invalid sender %s", 0, sender_name));
                  return;
               }

               if (memcmp(sender_name, "AP", 2) != 0)
               {
                  for (int ep = (int)BC0; ep <= (int)NumberOfEndPoints; ep++)
                  {
                     if (ep < APA)
                     {
                        if ((!sender_was_in_prev_view) && 
                           (!wasEndpointInPrevView((EndPoint)ep)) && 
                           (isEndpointInCurrView((EndPoint)ep)))
                        {
                           char endpoint_name[8] = "";
                           snprintf(endpoint_name, sizeof(endpoint_name), "BC%d", ep);

                           TRACE((LOG_LEVEL_INFO,
                                  "%s and the sender (%s) were in the same "
                                  "view => update GPR cache for %s",
                                  0, endpoint_name, sender_name, endpoint_name));

                           memcpy(s_GPRCache[ep], gpr_cache[ep], sizeof(gpr_cache[ep]));
                        }
                     }
                  }
                  storeGPRCache();
               }

               --m_numberOfExpectedGPRCacheUpdates;

               if (m_numberOfExpectedGPRCacheUpdates < 1)
               {
                  TRACE((LOG_LEVEL_INFO, "%s", 0, "Update of GPR cache completed"));
                  m_GPRCacheUpdateTimer = 0;
               }
               else
               {
                  TRACE((LOG_LEVEL_INFO,
                         "Expecting %d more cache update to complete cache update",
                         0, m_numberOfExpectedGPRCacheUpdates));
               }

               break;
            }
         }
         else if (m_state == WAITING_FOR_GPR_CACHE)
         {
            TRACE((LOG_LEVEL_INFO,
                   "Received GPR cache update in state WAITING_FOR_GPR_CACHE", 0));

            ///< Update GPR cache
            theMessage.getGPRCacheValue(s_GPRCache);
            ///< Update the GPR mapping file to synchronize with the cache
            storeGPRCache();
            ///< Change AP2CPPH state for this FTm end point to work as GPR cache master
            setState(GPR_CACHE_MASTER);

            ///< Rebroadcast complete GPR cache in case blades are waiting for 2nd GPR cache update.
            CPHW_GPRSHM_FTmMedia::FTmState state = sendGPRCache(NULL);
            TRACE((LOG_LEVEL_TRACE,
                  "Rebroadcast GPR cache to waiting end points, sendGPRCache executed with state %s",
                  0,
                  CPHW_GPRSHM_FTmMedia::stateName(state)));
         }
         else
         {
            int sender_id;
            const char *sender_name = m_ftmMedia.lastSenderName();
 
            if ((memcmp(sender_name, "APA", 3) == 0) ||
                (memcmp(sender_name, "APB", 3) == 0))
            {
               TRACE((LOG_LEVEL_INFO,
                     "Can safely ignore own GPR cache update.", 0));
            }
            else
            {
               TRACE((LOG_LEVEL_WARN,
                     "Unexpected GPR cache update "
                     "=> ignore cache update request",
                     0));
            }
         }
         break;
      }

      case vShlfGPRMsg::GPR_READ_REQ:
      {
         uint32_t result = 0;       ///< successfully.
         uint32_t value = 0;

         ///< Check if the Target CPID and GPR Address are correct
         if (theMessage.targetCPID() > BC63)
         {
            TRACE((LOG_LEVEL_WARN, "CPID %d is not correct", 0, theMessage.targetCPID()));
            break;
         }
         else if ((theMessage.GPRAddr() < BASE_GPR_ADDRESS) || (theMessage.GPRAddr() >= BASE_GPR_ADDRESS + NO_OF_GPR) )
         {
            TRACE((LOG_LEVEL_WARN, "GPR address %d is not correct", 0, theMessage.GPRAddr()));
            result = vGPRShlfMgrErrorCodes::INCORRECT_GPR_ADDRESS;
         }
         else
         {
            result = vGPRShlfMgrErrorCodes::SUCCESS;
            value = this->s_GPRCache[theMessage.targetCPID()][theMessage.GPRAddr() - BASE_GPR_ADDRESS];
         }

         ///< Send GPR response message
         sendGPRReadRsp(theMessage.targetCPID(), theMessage.GPRAddr(), result, value, m_ftmMedia.lastSenderName());

         break;
      }

      case vShlfGPRMsg::GPR_WRITE:
      {
         ///< Check if the Target CPID and GPR Address are correct
         if (theMessage.targetCPID() > BC63)
         {
            TRACE((LOG_LEVEL_WARN, "CPID %d is not correct", 0, theMessage.targetCPID()));
            break;
         }
         else if ((theMessage.GPRAddr() < BASE_GPR_ADDRESS) || (theMessage.GPRAddr() >= BASE_GPR_ADDRESS + NO_OF_GPR) )
         {
            TRACE((LOG_LEVEL_WARN, "GPR address %d is not correct", 0, theMessage.GPRAddr()));
            break;
         }
         else if (m_numberOfExpectedGPRCacheUpdates > 0)
         {
            TRACE((LOG_LEVEL_WARN, "GPR cache update is in progress, none the less, updating BC%d GPR %d to %d", 0, theMessage.targetCPID(), theMessage.GPRAddr(), theMessage.GPRValue()));
         }

         ///< Update GPR cache
         updateGPRCache(theMessage.targetCPID(), theMessage.GPRAddr(), theMessage.GPRValue());
         break;
      }

      default:
      {
         TRACE((LOG_LEVEL_TRACE, "%s %d", 0, "Unknown primitive", theMessage.getID()));
      }
   }
}

/**
 * @brief This callback function will be called from FTm  when receiving message
 *
 * @param msg Message buffer
 * @return N/A
 */
void CPHW_GPRSHM_AP2CPPH::onFTmMsgReceived(native_vni_msg_t msg)
{
   SUPPRESS_UNUSED_MESSAGE(msg);
   TRACE((LOG_LEVEL_ERROR, "%s %s is called un-expectedly", 0, FILENAME, __FUNCTION__));
}

/**
 * @brief This callback function will be called from FTm  when the view is changed
 *
 * @param endpoint_names: Array of endpoint names
 * @param endpoint_count Number of endpoint in current view
 */
void CPHW_GPRSHM_AP2CPPH::onFTmViewChanged(char const* const* endpoint_names, size_t endpoint_count)
{

   int i = 0;
   timeval tv;
   ostringstream trace;
   trace << "AP2CPPH (state = " << stateName(m_state) << ") received new view: ";
   for (i = 0; endpoint_names && (i < (int)endpoint_count); i++)
   {
      const char *endpoint_name = const_cast<char*>(endpoint_names[i]);
      if (endpoint_name == NULL) continue;
      trace << endpoint_names[i] << " ";
   }
   trace << endl;
   TRACE((LOG_LEVEL_INFO, "%s", 0, trace.str().c_str()));

   m_prevBCbitmap = m_currBCbitmap;
   m_prevAPbitmap = m_currAPbitmap;

   m_currBCbitmap = 0;
   m_currAPbitmap = 0;

   for (i = 0; endpoint_names && (i < (int)endpoint_count); i++)
   {
      int endpoint_id = 0;
      const char *endpoint_name = const_cast<char*>(endpoint_names[i]);
      if (endpoint_name == NULL) continue;

      // First, to find BC nodes ...
      if (sscanf(endpoint_name, "BC%d", &endpoint_id) == 1)
      {
         if ((endpoint_id >= BC0) && (endpoint_id <= BC63))
         {
            m_currBCbitmap |= (static_cast<uint64_t>(1) << endpoint_id);
         }
         else
         {
            TRACE((LOG_LEVEL_WARN, "Invalid CP blade ID %d", 0, endpoint_id));
            continue;
         }
      }

      // Then, to find AP nodes ...
      if (strcmp(endpoint_name, "APA") == 0)
      {
         m_currAPbitmap |= (static_cast<uint8_t>(1));
      }
      else if (strcmp(endpoint_name, "APB") == 0)
      {
         m_currAPbitmap |= (static_cast<uint8_t>(1) << 1);
      }
   }

   /**
    * Update endpoint list
    */
   bool cpInView = false;
   bool newCpJoined = false;

   for (i = 0; i < NumberOfEndPoints; i++)
   {
      m_endpoints[i].m_updated = false;
   }

   for (i = 0; i < (int)endpoint_count; i++)
   {
      EndPoint ep = endPointId(endpoint_names[i]);
      if (ep < NumberOfEndPoints)
      {
         if (ep < APA) cpInView = true;

         m_endpoints[ep].m_updated = true;
         if (!m_endpoints[ep].m_inView)
         {
            memcpy(m_endpoints[ep].m_name, endpoint_names[i], strlen(endpoint_names[i]) + 1);
            m_endpoints[ep].m_inView = true;
            newCpJoined = true;     // Used for sending complete GPR cache

            TRACE((LOG_LEVEL_TRACE, "viewHandler added %s", 0, endpoint_names[i]));
         }
      }
      else
      {
         // we don't care about tester endpoints, except maybe flag
         // that the system is being tested
         TRACE((LOG_LEVEL_WARN, "viewHandler unrecognized endpoint %s", 0, endpoint_names[i]));
      }
   }
   for (i = 0; i < NumberOfEndPoints; i++)
   {
      if (!m_endpoints[i].m_updated && m_endpoints[i].m_inView)
      {
         m_endpoints[i].m_inView = false;
         TRACE((LOG_LEVEL_TRACE, "viewHandler deleted %s", 0, m_endpoints[i].m_name));
      }
   }

   /**
    * Handle GPR cache update. If this is the first time to join the view, it will receive
    * GPR cache update from other GPR endpoint i.e BC0.
    */

   TRACE((LOG_LEVEL_TRACE, "CP in view %d", 0, cpInView));

   if (m_state == STARTED) ///< First onFTmViewChanged happens
   {
      ///< Start supervision timer to check to the BC(s) has (have)
      ///< been already joined the FTm group
      gettimeofday(&tv, NULL);
      m_timeJoinedFTmGroup = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
      setState(JOINED_FTM_GROUP);

   }
   else if (m_state == JOINED_FTM_GROUP)
   {
      if (cpInView == true) ///< Waiting for GPR cache update from CP
      {
         // There are other endpoints already in the view.

         TRACE((LOG_LEVEL_INFO, "Waiting for GPR cache update from CP", 0));
         setState(WAITING_FOR_GPR_CACHE);
      }
   }
   else if (m_state == WAITING_FOR_GPR_CACHE)
   {
      ///< No CP sending Cache before all leaving Ftm group
      if (cpInView == false)
      {
         setState(GPR_CACHE_MASTER);
      }
   }

   if ((m_state == GPR_CACHE_MASTER) && newCpJoined)
   {
      // 1 or more end points have joined view. We expect 2 GPR cache updates,
      // one from the leader of end points in the previous view (which is
      // AP) and one from the leader of newly joined end point(s). This includes
      // the case where only 1 end point joined. If this end point is starting
      // up their cache is not valid. In this case the end point waits for cache
      // update from AP and rebroadcasts the cache update.
      gettimeofday(&tv, NULL);
      m_GPRCacheUpdateTimer = (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
      m_numberOfExpectedGPRCacheUpdates = 2;
      TRACE((LOG_LEVEL_INFO,
             "GPR cache updates expected as 1 or more endpoints joined "
             "view; expecting 2 GPR cache broadcast(s)",
             0));

      TRACE((LOG_LEVEL_INFO,
             "m_prevBCbitmap:%#.16lx m_prevAPbitmap:%#.1x "
             "m_currBCbitmap:%#.16lx m_currAPbitmap:%#.1x\n",
             0, m_prevBCbitmap, m_prevAPbitmap, m_currBCbitmap, m_currAPbitmap));

      // Send GPR cache update.
      CPHW_GPRSHM_FTmMedia::FTmState state = sendGPRCache(NULL);
      TRACE((LOG_LEVEL_TRACE, "sendGPRCache executed with state %s", 0, CPHW_GPRSHM_FTmMedia::stateName(state)));
   }
}

/**
 * @brief This callback function will be called when FTm block requested
 *
 * @param N/A
 * @return N/A
 */
void CPHW_GPRSHM_AP2CPPH::onFTmBlockRequest()
{
   TRACE((LOG_LEVEL_TRACE, "%s %s", 0, FILENAME, __FUNCTION__));
   m_ftmMedia.sendBlockAck();
   TRACE((LOG_LEVEL_TRACE, "Send FTm Block Ack", 0));
}

/**
 * @brief This callback function is called from FTm
 *
 * @param
 */
void CPHW_GPRSHM_AP2CPPH::onFTmCongestionChanged(bool congested, unsigned sender_rank)
{
   TRACE((LOG_LEVEL_INFO, "%s %s %s, sender rank = %u", 0, FILENAME, __FUNCTION__,
          congested ? "congested" : "no-congestion", sender_rank));
}

/**
 * @brief ???
 *
 * @param
 */
void CPHW_GPRSHM_AP2CPPH::onFTmEndpointFound(char const* endpoint, native_vni_evolve_stack_t const*)
{
   TRACE((LOG_LEVEL_INFO, "%s %s %s", 0, FILENAME, __FUNCTION__, endpoint));
}

/**
 * @brief This is used to call the FTm poll() to active the FTm does the work
 *
 * @param N/A
 * @return N/A
 */
void CPHW_GPRSHM_AP2CPPH::poll()
{
   uint64_t anchor;
   uint64_t now;
   timeval tv;

   gettimeofday(&tv, NULL);
   anchor = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;

   // Check if we have timed out waiting for a 2nd view change.
   // This would mean there are no other end points in the view.
   if (m_state == JOINED_FTM_GROUP)
   {
      if (anchor - m_timeJoinedFTmGroup > 1000 * SUPERVISION_TIME)
      {
         ///< Supervision time expired. Work as GPR cache master
         TRACE((LOG_LEVEL_INFO, "Supervision time expired. Work as GPR cache master", 0));
         setState(GPR_CACHE_MASTER);
      }
   }

   // If no GPR cache update is received within specified time,
   // declare ourselves as master and send cache update to ensure
   // we have GPR cache coherency.
   if (m_state == WAITING_FOR_GPR_CACHE)
   {
      if (anchor - m_timeJoinedFTmGroup > 1000 * SUPERVISION_TIME)
      {
         ///< Supervision time expired. Work as GPR cache master
         TRACE((LOG_LEVEL_INFO, "poll detected supervision time expired. Work as GPR cache master", 0));
         setState(GPR_CACHE_MASTER);

         ///< Send full GPR cache since working as GPR cache master
         CPHW_GPRSHM_FTmMedia::FTmState state = sendGPRCache(NULL);
         TRACE((LOG_LEVEL_TRACE, "sendGPRCache executed with state %s", 0, CPHW_GPRSHM_FTmMedia::stateName(state)));
      }
   }

   // Check to see if we have timed out waiting for GPR cache updates during normal
   // operation (not during start-up).
   if (m_GPRCacheUpdateTimer != 0)
   {
      if (anchor - m_GPRCacheUpdateTimer > 1000 * SUPERVISION_TIME_GPRCACHE)
      {
         m_numberOfExpectedGPRCacheUpdates = 0;
         m_GPRCacheUpdateTimer = 0;
         TRACE((LOG_LEVEL_WARN, "%s", 0, "Timed out waiting for GPR cache updates"));
      }
   }

   while (m_ftmMedia.poll())
   {
      gettimeofday(&tv, NULL);
      now = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
      if (now - anchor < 100)
      {
         // allow FTm to run max 100us
         continue;
      }
      break;
   }
}

/**
 * @brief This function is used to init the cache from cache file, FTm,...
 *
 * @param N/A
 * return Error code
 */
int CPHW_GPRSHM_AP2CPPH::init()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   int errorCode = 0;

   ///< Create or open GpR cache
   m_gprFile = CPHW_GPRSHM_GPRFile::getInstance(errorCode);
   if (errorCode != 0)
   {
      EVENT((__LINE__, FILENAME, "Failed to start service: Error opening GPR cache file", __FUNCTION__));
      TRACE((LOG_LEVEL_ERROR, "%s. Error code: %d. Error text: %s", 0, "Failed to open GPR cache file",
             errorCode, strerror(errorCode)));
      return errorCode;
   }

   ///< Update GPR cache from file
   if (!readGPRCache())
   {
      EVENT((__LINE__, FILENAME, "Failed to start service: Error updating GPR cache from file", __FUNCTION__));
      return -1;
   }

   if (!isFTmMediaOperational())
   {
      m_ftmMedia.setDebug(false);

      // Join the CP group within the FTm media.
      setEndPointName();

      TRACE((LOG_LEVEL_INFO, "%s", 0, "AP2CPPH: Join FTm group"));

      if ((m_ftmMedia.state() < CPHW_GPRSHM_FTmMedia::Inited) &&
          m_ftmMedia.setEndpointFormat("AP%hu"))
      {
         char const* ipAddrs[] = { s_intf[0], s_intf[1] };
         size_t number_of_vlans = 2;

         // Init FTm media.
         if (!m_ftmMedia.init(ipAddrs, number_of_vlans, true, FTmMedia::PORT_VSHMGPR))
         {
            EVENT((__LINE__, FILENAME, "Failed to start service: Failed to initialise FTmMedia", __FUNCTION__));
            return -1;
         }
      }
      else
      {
         EVENT((__LINE__, FILENAME, 
               "Failed to start service: Unexpected FTm state or incorrect end point format",
               __FUNCTION__));
         return -1;
      }


      // Join the CP group within the FTm media.
      if ((m_ftmMedia.state() == CPHW_GPRSHM_FTmMedia::Inited) &&
          /*m_ftmMedia.makeEndpointName(1, endpointName, sizeof(endpointName)) &&*/
          m_ftmMedia.joinGroup(m_endpointName, m_ftmGroupName, this, false))
      {
         setState(CPHW_GPRSHM_AP2CPPH::STARTED);
      }
      else
      {
         EVENT((__LINE__, FILENAME, 
               "Failed to start service: Unexpected FTm state or failed to join FTm group",
               __FUNCTION__));
         return -1;         
      }
   }

   EVENT((__LINE__, FILENAME, "Successfully start GPRSHM service", __FUNCTION__));
   return 0;
}


/**
 * @brief Check if FTm media is operational.
 *
 * @return true:  If Ftm media is operational
 *         false: If Ftm media is not operational
 */
bool CPHW_GPRSHM_AP2CPPH::isFTmMediaOperational()
{
   return ((m_ftmMedia.state() >= CPHW_GPRSHM_FTmMedia::JoinedGroup) ? true : false);
}

/**
 * @brief This method is used to send message to FTm in unicast or broadcast
 *
 * @param  thevShlfGPRMsg this message object that has data buffer to send
 * @param destEndPoint is the name of FTm end point to receive the message, NULL if sending in broadcast
 * @return Returns the received media status of type CPHW_GPRSHM_AP2CPPH::FTmState.
 */
CPHW_GPRSHM_FTmMedia::FTmState CPHW_GPRSHM_AP2CPPH::sendMsgtoFTm(vShlfGPRMsg& thevShlfGPRMsg, const char* destEndPoint)
{
   CPHW_GPRSHM_FTmMedia::FTmState theMediaStatus = CPHW_GPRSHM_FTmMedia::Failed;

   const char* msg_buf = reinterpret_cast<const char*>(thevShlfGPRMsg.getData());

   if (destEndPoint == NULL) // Send broadcast
   {
      ostringstream trace;
      trace << FILENAME << " " << __FUNCTION__ << " len=" << thevShlfGPRMsg.getLen() << ":" << endl;
      trace << thevShlfGPRMsg << endl;
      TRACE((LOG_LEVEL_TRACE, "%s", 0, trace.str().c_str()));

      theMediaStatus = m_ftmMedia.broadcastFTmMsg(msg_buf, thevShlfGPRMsg.getLen());
   }
   else  // Send unicast
   {
      if ((thevShlfGPRMsg.getID() == vShlfGPRMsg::GPR_READ_REQ) ||
          (thevShlfGPRMsg.getID() == vShlfGPRMsg::GPR_READ_RES))
      {
         ostringstream trace;
         trace << "Send unicast message to endpoint: " << destEndPoint << endl;
         trace << thevShlfGPRMsg << endl;

         TRACE((LOG_LEVEL_TRACE, "%s", 0, trace.str().c_str()));

         theMediaStatus = m_ftmMedia.unicastFTmMsg(destEndPoint, msg_buf, thevShlfGPRMsg.getLen());
      }
   }

   return theMediaStatus;
}

/**
 * @brief This method is used to send GPR cache to an endpoint
 *
 * @param destEndPoint is the name of FTm end point to receive the message, NULL if sending in broadcast
 * @return Returns the received media status of type CPHW_GPRSHM_AP2CPPH::FTmState.
 */
CPHW_GPRSHM_FTmMedia::FTmState CPHW_GPRSHM_AP2CPPH::sendGPRCache(const char* destEndPoint)
{
   SUPPRESS_UNUSED_MESSAGE(destEndPoint);

   vShlfGPRMsg thevShlfGPRMsg;

   thevShlfGPRMsg.setVer(1);
   thevShlfGPRMsg.setID(vShlfGPRMsg::GPR_COMPLETE_CACHE_UPDATE);
   thevShlfGPRMsg.setGPRCacheValue(/*(const uint32_t**)*/ CPHW_GPRSHM_AP2CPPH::s_GPRCache);

   return sendMsgtoFTm(thevShlfGPRMsg, NULL);
}

/**
 * @brief This method is used to send GPR Read response
 *
 * @param targetCPID CP ID that the gpr belongs to
 * @param grpAddress Address of GPR (8 to 15)
 * @param result 0 if successful, error code if failed
 * @value value of gpr with address above. Invalid if result is not zero
 * @param destEndPoint is the name of FTm end point to receive the message, NULL if sending in broadcast
 * @return Returns the received media status of type CPHW_GPRSHM_AP2CPPH::FTmState.
 */
CPHW_GPRSHM_FTmMedia::FTmState CPHW_GPRSHM_AP2CPPH::sendGPRReadRsp(const uint32_t targetCPID,
                                                                   const uint32_t gprAddress,
                                                                   const uint32_t result,
                                                                   const uint32_t value,
                                                                   const char*    destEndPoint)
{
   vShlfGPRMsg thevShlfGPRMsg;

   thevShlfGPRMsg.setVer(1);
   thevShlfGPRMsg.setID(vShlfGPRMsg::GPR_READ_RES);
   thevShlfGPRMsg.senderCPID(0xFFFFFFFFU);  ///< TODO: which value for AP?
   thevShlfGPRMsg.targetCPID(targetCPID);
   thevShlfGPRMsg.GPRAddr(gprAddress);
   thevShlfGPRMsg.result(result);
   thevShlfGPRMsg.resValue(value);
   thevShlfGPRMsg.setLength(READ_GPR__RSP_LEN);

   return sendMsgtoFTm(thevShlfGPRMsg, destEndPoint);
}

/**
 * @brief This method is used to update GPR cache for both internal data and cache file
 *
 * @param targetCPID CP ID that the gpr belongs to
 * @param grpAddress Address of GPR (8 to 15)
 * @param result 0 if successful, error code if failed
 * @value value of gpr with address above. Invalid if result is not zero
 * @return N/A
 */
void CPHW_GPRSHM_AP2CPPH::updateGPRCache(uint32_t targetCpId, uint32_t GPRAddress, uint32_t GPRValue)
{
   assert(targetCpId <= BC63);
   assert(GPRAddress >= BASE_GPR_ADDRESS);
   assert(GPRAddress < BASE_GPR_ADDRESS + NO_OF_GPR);

   TRACE((LOG_LEVEL_TRACE, "%s %s CP[%u].GPR[%u] = %u", 0, FILENAME, __FUNCTION__,
          targetCpId, GPRAddress - BASE_GPR_ADDRESS, GPRValue));

   ///< Update cache
   CPHW_GPRSHM_AP2CPPH::s_GPRCache[targetCpId][GPRAddress - BASE_GPR_ADDRESS] = GPRValue;

   ///< Update cache file
   if (m_gprFile != NULL)
   {
      m_gprFile->set(targetCpId, GPRAddress - BASE_GPR_ADDRESS, GPRValue);
   }
}

/**
 * @brief This method is used to set end point name
 *
 * @param endPointName end point name
 * @return N/A
 */
void CPHW_GPRSHM_AP2CPPH::setEndPointName()
{
    TRACE((LOG_LEVEL_DEBUG, "CPHW_GPRSHM_AP2CPPH::setEndPointName()",0));
 
    // Set the FTm end point name by working out which APG side we are on.

    // Set default FTm end point name.
    strncpy(m_endpointName, "APA", sizeof(m_endpointName));

    // Determine APG side by reading file containing slot number.
    ifstream slotFile(s_slotFileName);

   if (slotFile.good()) {
      int slot = 1;
      slotFile >> slot;
      slotFile.close();

      if (slotFile.good())
      {
         if (slot != 1)
         {
            // B side
            strncpy(m_endpointName, "APB", sizeof(m_endpointName));
         }
      }
      else
      {
         TRACE((LOG_LEVEL_WARN,
               "Error reading APG slot from file %s, using default FTm end point name",0, s_slotFileName));
      }
   }
   else
   {
      TRACE((LOG_LEVEL_WARN,
            "Error opening file %s to read APG slot, using default FTm end point name.",0, s_slotFileName));
   }

   TRACE((LOG_LEVEL_INFO, "Using FTm end point name: %s.",0, m_endpointName));
    
   return;
}

/**
 * @brief This method is used to get end point name
 *
 * @param endPointName end point name
 * @return N/A
 */
const char* CPHW_GPRSHM_AP2CPPH::getEndPointName()
{
   return this->m_endpointName;
}

/**
 * @brief This method is used to store GPR cache to cache file
 *
 * @param N/A
 * @return N/A
 */
bool CPHW_GPRSHM_AP2CPPH::storeGPRCache()
{
   TRACE((LOG_LEVEL_INFO, "%s %s", 0, FILENAME, __FUNCTION__));

   for (int i = 0; i < MAX_CP; ++i)
   {
      for (int j = 0; j < NO_OF_GPR; ++j)
      {
         if ((m_gprFile != NULL) && (!m_gprFile->set(i, j, s_GPRCache[i][j])))
         {
            TRACE((LOG_LEVEL_ERROR, "Failed to store GPR[%d] = %d for BC%d", 0, j, s_GPRCache[i][j], i));
            return false;
         }
      }
   }

   return true;
}

/**
 * @brief This method is used to fill the GPR cache from the cache file
 *
 * @param N/A
 * @return N/A
 */
bool CPHW_GPRSHM_AP2CPPH::readGPRCache()
{
   TRACE((LOG_LEVEL_INFO, "%s %s", 0, FILENAME, __FUNCTION__));

   for (int i = 0; i < MAX_CP; ++i)
   {
      for (int j = 0; j < NO_OF_GPR; ++j)
      {
         if ((m_gprFile != NULL) && (!m_gprFile->get(i, j, s_GPRCache[i][j])))
         {
            TRACE((LOG_LEVEL_ERROR, "Failed to get GPR[%d] for BC%d", 0, j, i));
            return false;
         }
      }
   }

   return true;
}

void CPHW_GPRSHM_AP2CPPH::stop()
{
   m_GPRCacheUpdateTimer = 0;
   CPHW_GPRSHM_GPRFile::deleteInstance(m_gprFile);
}

/**
 * @brief Count number of endpoints in the given bitmap
 * @param Bitmap which represents endpoints
 * @return Number of endpoints in bitmap
 */
int CPHW_GPRSHM_AP2CPPH::countEndpoint(uint64_t bitmap)
{
   int count = 0;
   while (bitmap != 0)
   {
      if (bitmap & 1)
         count++;
      bitmap = bitmap >> 1;
   }
   return count;
}

/**
 * @brief Check if the given endpoint presents in the current view
 *
 * @param endpoint 
 * @return true if endpoint presents in the current view, false otherwise
 */
bool CPHW_GPRSHM_AP2CPPH::isEndpointInCurrView(EndPoint endpoint)
{
    if (endpoint >= APA)
    {
       if (m_currAPbitmap != 0)
          return true;
       else
          return false;
    }
    uint64_t mask = (static_cast<uint64_t>(1) << endpoint);
    if (m_currBCbitmap & mask)
        return true;
    else
        return false;
}

/**
 * @brief Check if the given endpoint presents in the previous view
 *
 * @param endpoint 
 * @return true if endpoint presents in the previous view, false otherwise
 */
bool CPHW_GPRSHM_AP2CPPH::wasEndpointInPrevView(EndPoint endpoint)
{
    if (endpoint >= APA)
    {
       if (m_prevAPbitmap != 0)
          return true;
       else
          return false;
    }
    uint64_t mask = (static_cast<uint64_t>(1) << endpoint);
    if (m_prevBCbitmap & mask)
        return true;
    else
        return false;
}
