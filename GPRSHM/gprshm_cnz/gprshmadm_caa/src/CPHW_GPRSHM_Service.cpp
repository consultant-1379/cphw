/****************************************************************************/
/**
 *  CPHW_GPRSHM_Service.cpp
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
 *  This is class implementation for CPHW_GPRSHM_Service class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-06  xdargas  Passivating service for single CP system.
 */

#include "CPHW_GPRSHM_Service.h"
#include "CPHW_GPRSHM_Trace.h"
#include "CPHW_GPRSHM_Definitions.h"

/***
 * @brief This is the class constructor.
 * @paramm[in]: daemon_name - name of application.
 * @param[in]: user_name - user is used to start application.
 * @return: N/A
 */
CPHW_GPRSHM_Service::CPHW_GPRSHM_Service(const char* daemon_name, const char* user_name) :
   ACS_APGCC_ApplicationManager(daemon_name, user_name),
   m_isAppStarted(false),
   m_pServer()
{
}

/***
 * @brief This is the class destructor.
 * @param: N/A
 * @return: N/A
 */
CPHW_GPRSHM_Service::~CPHW_GPRSHM_Service()
{

}

/**
 * @brief This function is used to activate amf service loop.
 * @param: N/A
 * @return: The error code
 */
ACS_APGCC_HA_ReturnType CPHW_GPRSHM_Service::init()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   ACS_APGCC_HA_ReturnType errorCode = ACS_APGCC_HA_SUCCESS;

   errorCode = activate();

   if (errorCode == ACS_APGCC_HA_FAILURE)
   {
      TRACE((LOG_LEVEL_ERROR, "cphw_gprshmd. HA Activation Failed", 0));
   }

   if (errorCode == ACS_APGCC_HA_FAILURE_CLOSE)
   {
      TRACE((LOG_LEVEL_ERROR, "cphw_gprshmd, HA Application Failed to Gracefully closed!!", 0));
   }

   if (errorCode == ACS_APGCC_HA_SUCCESS)
   {
      TRACE((LOG_LEVEL_INFO, "cphw_gprshmd, HA Application Gracefully closed!!", 0));
   }

   return errorCode;
}

/**
 * @brief This functions is used to run the application.
 * @param: N/A
 * @return: N/A
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::start()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   m_pServer.reset(new CPHW_GPRSHM_Server());
   if (m_pServer)
   {
      m_pServer->start();
      return ACS_APGCC_SUCCESS;
   }

   return ACS_APGCC_FAILURE;
}


/**
 * @brief This functions is used to stop application.
 * @param: N/A
 * @return: N/A
 */
void CPHW_GPRSHM_Service::stop()
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   if (m_pServer)
   {
      m_pServer->stop();
      m_pServer.reset();
   }
}

/**
 * @brief This is a callback functions that is called from AMF when application's state changes to active.
 * @param[in]: previousHAState - the previous state of application
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   /** Check if we have received the ACTIVE State Again.
    * This means that, our application is already Active and
    * again we have got a callback from AMF to go active.
    * Ignore this case anyway. This case should rarely happens
    */

   ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;

   if (ACS_APGCC_AMF_HA_ACTIVE == previousHAState)
      return rc;

   /** Our application has received state ACTIVE from AMF.
    * Start off with the activities needs to be performed
    * on ACTIVE
    */

   /** Handle here what needs to be done when you are given ACTIVE State */
   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: received ACTIVE state assignment!!!", 0));

   ///< Run application.
   if (!m_isAppStarted)
   {
      TRACE((LOG_LEVEL_INFO, "Start application", 0));
      rc = start();
      m_isAppStarted = true;
   }

   return rc;
}

ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
   SUPPRESS_UNUSED_MESSAGE(previousHAState);
   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: received Passive state assignment!!!", 0));
   return ACS_APGCC_SUCCESS;
}

/**
 * @brief This is a callback functions that is called from AMF when application's state changes to queising.
 * @param[in]: previousHAState - the previous state of application
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT /**previousHAState*/)
{
   /**
    * We were active and now losing active state due to some shutdown admin
    * operation performed on our SU.
    * Inform the thread to go to "stop" state
    */

   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: received QUIESING state assignment!!!", 0));

   ///< Stop application.
   if (m_isAppStarted)
   {
      TRACE((LOG_LEVEL_INFO, "Stop application", 0));
      stop();
      m_isAppStarted = false;
   }

   return ACS_APGCC_SUCCESS;
}

/**
 * @brief This is a callback functions that is called from AMF when application's state changes to queisced.
 * @param[in]: previousHAState - the previous state of application
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT /**previousHAState*/)
{
   /**
    * We were Active and now losting Active state due to Lock admin
    *  operation performed on our SU.
    *  Inform the thread to go to "stop" state
    */

   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: received QUIESCED state assignment!", 0));

   ///< Stop application.
   if (m_isAppStarted)
   {
      TRACE((LOG_LEVEL_INFO, "Stop application", 0));
      stop();
      m_isAppStarted = false;
   }

   return ACS_APGCC_SUCCESS;
}

/**
 * @brief This is a callback functions that is called from AMF in every healthcheck interval.
 * @param: N/A
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performComponentHealthCheck()
{
   TRACE((LOG_LEVEL_TRACE, "%s %s", 0, FILENAME, __FUNCTION__));
   ACS_APGCC_ReturnType rc = ACS_APGCC_SUCCESS;
   return rc;
}

/**
 * @brief This is a callback functions that is called from AMF when application is terminated.
 * @param: N/A
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performComponentTerminateJobs(void)
{
   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: received terminate callback!", 0));

   ///< Stop application.
   if (m_isAppStarted)
   {
      TRACE((LOG_LEVEL_INFO, "Stop application", 0));
      stop();
      m_isAppStarted = false;
   }

   return ACS_APGCC_SUCCESS;
}

/**
 * @brief This is a callback functions that is called from AMF when application is stopped.
 * @param: N/A
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performComponentRemoveJobs(void)
{
   /**
    * Application has received Removal callback. State of the application
    * is neither Active nor Standby. This is with the result of LOCK admin operation
    * performed on our SU. Terminate the thread by informing the thread to go "stop" state.
    */

   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: Application Assignment is removed now", 0));

   ///< Stop application.
   if (m_isAppStarted)
   {
      TRACE((LOG_LEVEL_INFO, "Stop application", 0));
      stop();
      m_isAppStarted = false;
   }

   return ACS_APGCC_SUCCESS;
}

/**
 * @brief This is a callback functions that is called from AMF when shutting down AMF.
 * @param: N/A
 * @return: ACS_APGCC_SUCCESS
 */
ACS_APGCC_ReturnType CPHW_GPRSHM_Service::performApplicationShutdownJobs(void)
{
   TRACE((LOG_LEVEL_INFO, "CPHW_GPRSHM_Service: Shutting down the application", 0));

   ///< Stop application.
   if (m_isAppStarted)
   {
      TRACE((LOG_LEVEL_INFO, "Stop application", 0));
      stop();
      m_isAppStarted = false;
   }

   return ACS_APGCC_SUCCESS;
}

