/****************************************************************************/
/**
 *  CPHW_GPRSHM_Server.h
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
 *  This is class declaration for CPHW_GPRSHM_Server class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 */

#ifndef _CPHW_GPRSHM_SERVER_H
#define _CPHW_GPRSHM_SERVER_H

#include "CPHW_GPRSHM_AP2CPPH.h"
#include "Event.h"

#include "ACS_CS_API.h"

#include <boost/thread.hpp>
#include <map>

class CPHW_GPRSHM_Server
{
public:
   /**
    * @Brief Constructor
    * @Param [in]: N/A
    * @Param [out]: N/A
    * @Return: error code
    */
   CPHW_GPRSHM_Server();

   /**
    * @Brief Destructor
    * Param [in]: N/A
    * Param [out]: N/A
    * Return: error code
    */
   ~CPHW_GPRSHM_Server();

   /**
    * @Brief  Start the service and its services
    * @Param [in]: N/A
    * @Param [out]: N/A
    * @Return error code
    */
   int start();

   /**
    * @Brief  Run the services
    * @Param [in]: N/A
    * @Param [out]: N/A
    * @Return error code
    */
   int run();

   /**
    * @Brief  Stop the server and its services
    * @Param [in]: N/A
    * @Param [out]: N/A
    * @Return error code
    */
   void stop();

private:
   Event m_stopEvent;
   boost::thread   m_serverThread;
   boost::scoped_ptr<CPHW_GPRSHM_AP2CPPH> m_ap2cpPH;
};

#endif /* _CPHW_GPRSHM_SERVER_H */

