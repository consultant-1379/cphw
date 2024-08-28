/****************************************************************************/
/**
 *  CPHW_GPRSHM_Server.cpp
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
 *  This is class implementation for CPHW_GPRSHM_Server class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-06  xdargas  Passivating service for single CP system.
 */

#include "CPHW_GPRSHM_Server.h"
#include "CPHW_GPRSHM_Trace.h"
#include "CPHW_GPRSHM_Event.h"
#include "CPHW_GPRSHM_Definitions.h"

#include "acs_prc_api.h"

#include <unistd.h>

using namespace std;

/**
 * @brief Constructor
 * @Param [in]: N/A
 * @Param [out]: N/A
 * @Return: N/A
 */
CPHW_GPRSHM_Server::CPHW_GPRSHM_Server() : m_stopEvent(),
   m_serverThread()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));
   m_stopEvent.resetEvent();
   m_ap2cpPH.reset();
}

/**
 * @Brief Destructor
 * @Param [in]: N/A
 * @Param [out]: N/A
 * @Return: N/A
 */
CPHW_GPRSHM_Server::~CPHW_GPRSHM_Server()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));
   if (m_ap2cpPH)
   {
      m_ap2cpPH->stop();
      m_ap2cpPH.reset();
   }
}

/****************************************************************************
 * Method:  start()
 * Description: Use to start INP server
 * Param [in]: N/A
 * Param [out]: N/A
 * Return: error code
 *****************************************************************************
 */
int CPHW_GPRSHM_Server::start()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));
   m_stopEvent.resetEvent();

   m_serverThread = boost::thread(&CPHW_GPRSHM_Server::run, this);

   return 0;
}


/**
 * @Brief run server
 * @Param [in]: N/A
 * @Param [out]: N/A
 * @return: N/A
 *****************************************************************************
 */
int CPHW_GPRSHM_Server::run()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   // Passivate service by returning if running on a single CP system
   
   // Read if multiple CP system using CS API.
   // Information might not be available yet so keep calling API until successful
   // or the service has been stopped.  
   
   int maxfd = 0;
   fd_set readset, allset;

   //Set allset to zero
   FD_ZERO(&allset);

   // Add stop event to allset
   FD_SET(m_stopEvent.getFd(), &allset);
   maxfd = m_stopEvent.getFd();

   ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NS::Result_Failure;
   bool multCPSys = false;
   timeval selectTimeout;
   int loopCount = 0;

   while (true)
   {
      ++loopCount;
      readset = allset;

      selectTimeout.tv_sec = 0;
      selectTimeout.tv_usec = 1000 * 100; ///< 100 milliseconds

      int select_return = ::select(maxfd + 1, &readset, NULL, NULL, &selectTimeout);

      if (select_return == 0) //timeout
      {
         // Call CS API to read if multiple CP system
		   res = ACS_CS_API_NetworkElement::isMultipleCPSystem(multCPSys);

		   if (res == ACS_CS_API_NS::Result_Success)
		   {
		      // If a single CP system return so service is passivated.
		      if (!multCPSys)
		      {
		         EVENT((__LINE__, FILENAME, "Single CP system detected, service is passivated", __FUNCTION__));
		         return 0;
		      }
		      else
		      {
		         // Multiple CP system so continue with starting service.
			      TRACE((LOG_LEVEL_INFO, "%s %s(): Multiple CP system detected",
			             0, FILENAME, __FUNCTION__));
		         break;
		      }
		   }
		   else
		   {
		      // Only print failed result every 500 ms as it could take some time before successful result.
		      if ((loopCount % 5) == 0)
		      { 
			      TRACE((LOG_LEVEL_INFO, "%s %s(): NetworkElement::isMultipleCPSystem failed: result = %d",
			             0, FILENAME, __FUNCTION__, res));
			      continue;
			   }
		   }         
      }
      else if (select_return == -1) // error
      {
         EVENT((__LINE__, FILENAME, "Failed to start service: error during reading CS API isMultipleCPSystem", __FUNCTION__));
         return 0;
      }
      else  //Handle is signed.
      {
         if (FD_ISSET(m_stopEvent.getFd(), &readset))
         {
            m_stopEvent.resetEvent();
            TRACE((LOG_LEVEL_INFO, "Server received stop event during reading CS API isMultipleCPSystem", 0));
            return 0;
         }
      }
   } // End while


   m_ap2cpPH.reset(new CPHW_GPRSHM_AP2CPPH());
   if (m_ap2cpPH)
   {
      if (m_ap2cpPH->init() != 0)
      {
         EVENT((__LINE__, FILENAME, "Failed to start service: failed to initialise FTm", __FUNCTION__));
         return -1;
      }
   }

   int m_stop = false;

   while (!m_stop)
   {
      readset = allset;

      selectTimeout.tv_sec = 0;
      selectTimeout.tv_usec = 1000 * 10; ///< 10 milliseconds

      int select_return = ::select(maxfd + 1, &readset, NULL, NULL, &selectTimeout);

      if (select_return == 0) //timeout
      {
         if (m_ap2cpPH) m_ap2cpPH->poll();
      }
      else if (select_return == -1) // error
      {
         TRACE((LOG_LEVEL_ERROR, "%s %s(): select error", 0, FILENAME, __FUNCTION__));
         break;
      }
      else  //Handle is signed.
      {
         if (FD_ISSET(m_stopEvent.getFd(), &readset))
         {
            m_stopEvent.resetEvent();
            m_stop = true;

            TRACE((LOG_LEVEL_INFO, "Server receives stop event", 0));
         }
      }
   } // End while

   return 0;
}

/**
 * @brief Stop server
 * @Param [in]: N/A
 * @Param [out]: N/A
 * @return: N/A
 *****************************************************************************
 */
void CPHW_GPRSHM_Server::stop()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   m_stopEvent.setEvent();

   // waiting for server thread stop
   if (m_serverThread.joinable())
      m_serverThread.join();

   if (m_ap2cpPH)
   {
      m_ap2cpPH->stop();
      m_ap2cpPH.reset();
   }
}

