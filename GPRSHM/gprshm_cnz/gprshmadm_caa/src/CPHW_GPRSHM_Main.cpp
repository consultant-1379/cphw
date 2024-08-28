/****************************************************************************/
/**
 *  CPHW_GPRSHM_Main.cpp
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
 *  This file implements the main entry for the GPRSHM.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-06  xdargas  Remove SetLogFilterLevel and GetLogFilterLevel methods.
 */

#include "CPHW_GPRSHM_Trace.h"
#include "CPHW_GPRSHM_Service.h"
#include "CPHW_GPRSHM_Server.h"
#include "ACS_APGCC_AmfTypes.h"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <string>
#include <signal.h>


/**
 * @brief Service entry point
 *
 * @@param argc Number of arguments
 * @@param argv Array o arguments
 * @@return 0 means successful, other are the error codes
 */

using namespace std;

const char* NOSERVICE = "noservice";
const int FAILED = -100;

void help();
int run_as_console();
int run_as_service();

/**
 * @brief  main()
 * @param [in]:
 * @param [out]:
 * @return: error code
 */
int main(int argc, char* argv[])
{
   int retVal = 0;

   // ------------------------------------------------------------------------
   // Parsing command line
   // ------------------------------------------------------------------------

   int opt;
   bool helpOpt = false;
   opterr = 0;
   while ((opt = getopt(argc, argv, "h")) != -1)
   {
      switch (opt)
      {
      case 'h':
      case '?':
         helpOpt = true;
         break;
      default:;
      }
   }

   if (helpOpt)
   {
      help();
      return FAILED;
   }

   if (argc == 1)
   {
      retVal = run_as_service();
   }
   else if (argc == 2)
   {
      string t(argv[1]);
      transform(t.begin(), t.end(), t.begin(), (int (*)(int)) ::tolower);
      if (t == NOSERVICE)
      {
         retVal = run_as_console();
      }
      else
      {
         help();
         retVal = FAILED;
      }
   }
   else
   {
      help();
      retVal = FAILED;
   }

   return retVal;
}

/**
 * @brief  run_as_console()
 * @param [in]: N/A
 * @param [out]: N/A
 * @return: error code
 */
int run_as_console()
{
   // No TRACE shall be invoked before this point
   initTRACE();
   TRACE((LOG_LEVEL_INFO, "main() starts as console", 0));

   try
   {
      // Block all signals for background thread.
      sigset_t new_mask;
      sigfillset(&new_mask);
      sigset_t old_mask;
      pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

      TRACE((LOG_LEVEL_INFO, "Server started.", 0));


      CPHW_GPRSHM_Server gprshmServer;

      //gprshmServer.setRunningMode(2); // run with noservice

      gprshmServer.start();

      // Restore previous signals.
      pthread_sigmask(SIG_SETMASK, &old_mask, 0);

      // Wait for signal indicating time to shut down.
      sigset_t wait_mask;
      sigemptyset(&wait_mask);
      sigaddset(&wait_mask, SIGINT);
      sigaddset(&wait_mask, SIGQUIT);
      sigaddset(&wait_mask, SIGTERM);
      pthread_sigmask(SIG_BLOCK, &wait_mask, 0);

      int sig = 0;
      sigwait(&wait_mask, &sig);

      gprshmServer.stop();

      TRACE((LOG_LEVEL_INFO, "Server stopped.", 0));

   }
   catch (std::exception& e)
   {
      TRACE((LOG_LEVEL_INFO, "%s", 0, e.what()));
   }

   return 0;
}

/****************************************************************************
 * @brief  run_as_service()
 * Description: run server in AMF service mode
 * @param [in]: N/A
 * @param [out]: N/A
 * @return: error code
 *****************************************************************************
 */
int run_as_service()
{
   ACS_APGCC_HA_ReturnType retCode = ACS_APGCC_HA_SUCCESS;

   /** Tracing instantiation must be after this point */
   CPHW_GPRSHM_Service vgprslfmgrService("cphw_gprshmd", "root");

   /** No TRACE shall be invoked before this point */
   initTRACE();
   TRACE((LOG_LEVEL_INFO, "main() starts", 0));

   try
   {
      retCode = vgprslfmgrService.init();

   }
   catch (...)
   {
      TRACE((LOG_LEVEL_INFO, "Exception. Exit GPRSHM server.", 0));
      return FAILED;
   }

   TRACE((LOG_LEVEL_INFO, "Exit  GPRSHM server.", 0));

   return retCode;
}


/****************************************************************************
 * @brief  help()
 * @param [in]:
 * @param [out]:
 * @return: error code
 *****************************************************************************
 */
void help()
{
   std::cout << "Usage: " << "cphw_gprshmd" << " [noservice]" << std::endl;
}
