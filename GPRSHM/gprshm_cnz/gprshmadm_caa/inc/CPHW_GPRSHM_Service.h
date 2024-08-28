/****************************************************************************/
/**
 *  CPHW_GPRSHM_Service.h
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
 *  This is class declaration for CPHW_GPRSHM_Service class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-06  xdargas  Passivating service for single CP system.
 */

#ifndef _CPHW_GPRSHM_SERVICE_H
#define _CPHW_GPRSHM_SERVICE_H

#include <ACS_APGCC_ApplicationManager.h>
#include "CPHW_GPRSHM_Server.h"
#include <boost/thread.hpp>

class CPHW_GPRSHM_Service : public ACS_APGCC_ApplicationManager
{
public:
   CPHW_GPRSHM_Service(const char* daemon_name, const char* user_name);
   virtual ~CPHW_GPRSHM_Service();

   ACS_APGCC_HA_ReturnType init();

   ACS_APGCC_ReturnType performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
   ACS_APGCC_ReturnType performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
   ACS_APGCC_ReturnType performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
   ACS_APGCC_ReturnType performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
   ACS_APGCC_ReturnType performComponentHealthCheck(void);
   ACS_APGCC_ReturnType performComponentTerminateJobs(void);
   ACS_APGCC_ReturnType performComponentRemoveJobs(void);
   ACS_APGCC_ReturnType performApplicationShutdownJobs(void);

private:
   ACS_APGCC_ReturnType start();
   void stop();

   bool m_isAppStarted;

   boost::scoped_ptr<CPHW_GPRSHM_Server> m_pServer;
};

#endif /* _CPHW_GPRSHM_SERVICE_H */

