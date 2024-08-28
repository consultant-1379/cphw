/****************************************************************************/
/**
 *  CPHW_GPRSHM_AP2CPPH.h
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
 *  This is class declaration for AP-CP communication protocol PROT1 over FTm.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-01-12  xjoschu  Added partition recovery.
 *  2017-02-10  xjoschu  Clean-up after internal review.
 *  2017-02-14  xjoschu  Clean-up following basic test.
 *  2017-05-02  xdargas  Changes due to TR HV77554.
 */

#ifndef _CPHW_GPRSHM_AP2CPPH_H
#define _CPHW_GPRSHM_AP2CPPH_H

#include "FTmMedia.hxx"
#include "CPHW_GPRSHM_FTmMedia.h"
#include "CPHW_GPRSHM_GPRFile.h"
#include "CPHW_GPRSHM_Definitions.h"
#include "vShlfGPRMsg.h"

#include "stdint.h"
#include <string.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"
/* ignoring warning deprecated conversion from string constant to 'char*' */

using namespace vGPRShlfMgrEndPoint;

class vShlfGPRMsg;

class CPHW_GPRSHM_AP2CPPH : public FTmEventHandler
{
public:
   /**
    * End point data for manage end point in view
    */
   struct EndPointData
   {
      char m_name[10];
      bool m_inView;            ///< Whether this endpoint is in view
      bool m_updated;           ///< temporary variable for CPHW_GPRSHM_AP2CPPH::onFTmViewChanged

      EndPointData() : m_inView(false) {}
   };

   /**
    * State of the instance of this class
    */
   enum AP2CPPH_STATE
   {
      NEW = 0,
      STARTED,
      JOINED_FTM_GROUP,
      WAITING_FOR_GPR_CACHE,
      GPR_CACHE_MASTER,
      STOPED,
      FAILED
   };

   /**
    * Constructor
    */
   CPHW_GPRSHM_AP2CPPH();

   /**
    * Destructor
    */
   ~CPHW_GPRSHM_AP2CPPH();

   /**
    * @brief AP2CPPH_STATE in readable text
    */
   static const char* stateName(AP2CPPH_STATE);

   /**< Override the virtual callback functions from FTmEventHandler */

   /**
    * @brief This callback function will be called from FTm  when receiving message
    *
    * @param msg Message buffer
    * @param msg_size Message size
    */
   void onFTmMsgReceived(const char* msg, size_t msg_size);

   /**
    * @brief This callback function is called by FTm when receiving message.
    *        Currently, this function is not used by FTm. Don't know why it is
    *        declared from FTmEventHandler class.
    *
    * @param
    */
   void onFTmMsgReceived(native_vni_msg_t);

   /**
    * @brief This callback function is called when FTm view change
    *
    * @param[in]: endPoints - Array of FTm endpoint names in current view
    * @param[in]: nEndPoint - Number of FTm endpoint in the endPoints array
    */
   void onFTmViewChanged(char const* const* endPoints, size_t nEndPoint);

   /**
    * @brief This callback function is called when FTm determines that the group
    * membership needs to be changed to reflect the an additional and/or
    * removal of endpoints to/from the group.
    *
    * @param[in]: N/A
    * @param[out]: N/A
    */
   void onFTmBlockRequest();

   /**
    * @brief This method is called when the Media is congested.
    *
    * @param
    */
   void onFTmCongestionChanged(bool, unsigned);

   /**
    * @brief ???
    *
    * @param
    */
   void onFTmEndpointFound(char const*, native_vni_evolve_stack_t const*);

   /**
    * @brief This is used to call the FTm poll() to active the FTm does the work
    * This function is called in every 10 millisecond or use the vni_sockets function...
    * Will check later
    */
   void poll();

   /**
    * @brief This function is used to init the FTm.
    */
   int init();

   /**
    * @brief This function is used to stop the FTm.
   */
   void stop(void);

private:
   void setState(AP2CPPH_STATE);
   CPHW_GPRSHM_FTmMedia::FTmState sendMsgtoFTm(vShlfGPRMsg& thevShlfGPRMsg, const char* destEndPoint = NULL);
   CPHW_GPRSHM_FTmMedia::FTmState sendGPRCache(const char* destEndPoint);
   CPHW_GPRSHM_FTmMedia::FTmState sendGPRReadRsp(const char* destEndPoint);
   CPHW_GPRSHM_FTmMedia::FTmState sendGPRWrite(void);
   void updateGPRCache(uint32_t targetCpId, uint32_t gprAddress, uint32_t gprValue);
   void setEndPointName();
   const char* getEndPointName();
   static EndPoint endPointId(const char*);

   /**
    * @brief Check if FTm media is operational.
    *
    * @return true:  If FTm media is operational
    *         false: If FTm media is not operational
    */
   bool isFTmMediaOperational();

   /**
    * @brief Store GPR to file
    *
    * @return true:  Successfully stored
    *         false: Failed to store
    */
   bool storeGPRCache();

   bool readGPRCache();

   CPHW_GPRSHM_FTmMedia::FTmState sendGPRReadRsp(const uint32_t targetCPID,
                                                 const uint32_t gprAddress,
                                                 const uint32_t result,
                                                 const uint32_t value,
                                                 const char*    destEndPoint);

   int countEndpoint(uint64_t endpoint_bitmap);
   bool wasEndpointInPrevView(vGPRShlfMgrEndPoint::EndPoint endpoint);
   bool isEndpointInCurrView(vGPRShlfMgrEndPoint::EndPoint endpoint);

   static uint32_t s_GPRCache[MAX_CP][NO_OF_GPR];

   static const char* s_slotFileName;  // File used to determine APG side
   static const char* s_intf[];        // Hard coded interfaces for FTm

   CPHW_GPRSHM_FTmMedia m_ftmMedia;
   const char* m_ftmGroupName;
   char m_endpointName[20];
   AP2CPPH_STATE m_state;
   uint64_t m_bitMap;
   uint64_t m_timeJoinedFTmGroup;
   EndPointData m_endpoints[NumberOfEndPoints];
   CPHW_GPRSHM_GPRFile* m_gprFile;

   // Indicates how many GPR Cache messages we are expecting
   uint8_t m_numberOfExpectedGPRCacheUpdates;

   // Timer for reception GPR cache broadcasts
   uint64_t m_GPRCacheUpdateTimer;

   uint64_t m_currBCbitmap; // Blade bit map indicates whether a blade is in FTM view
                            // 1: Yes, 0: No
                            // e.g. 0x3 means only blade 0 and 1 are in FTM view
   uint64_t m_prevBCbitmap; // The same as m_currBCbitmap, but indicates previous view
   uint8_t  m_currAPbitmap; // APG bit map indicates whether a AP blade is in FTM view
                            // 1: Yes, 0: No
                            // bit0: APA   bit1: APB
   uint8_t  m_prevAPbitmap; // The same as m_currAPbitmap, but indicates previous view
};

/**
 * @brief Convert an endpoint string to its id
 *
 * @param name End point name
 */
inline EndPoint CPHW_GPRSHM_AP2CPPH::endPointId(const char* name)
{
   int len = strlen(name);

   if ((len != 3) && (len != 4))
   {
      return Tester;
   }
   else if (!memcmp(name, "BC", 2))
   {

      int endpointId = atoi(name + 2);
      if ( (endpointId >= BC0) && (endpointId <= BC63) )
      {
         return static_cast<EndPoint>(endpointId);
      }
      else
      {
         return Tester;
      }
   }
   else if (!memcmp(name, "APA", 3))
   {
      return APA;
   }
   else if (!memcmp(name, "APB", 3))
   {
      return APB;
   }
   else
   {
      return Tester;
   }
}

#endif /* _CPHW_GPRSHM_AP2CPPH_H */
