/****************************************************************************/
/**
 *  CPHW_GPRSHM_Event.cpp
 *
 *  190 89-CAA 109 1638
 *
 *  COPYRIGHT Ericsson AB 2017. All rights reserved.
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
 *  This is class implementation for CPHW_GPRSHM_Event class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2017-02-03  xmarkho  Created.
 *  2017-02-15  xdargas  Correcting process name that I incorrectly modified.
 */

#include <sstream>
#include "CPHW_GPRSHM_Trace.h"
#include "CPHW_GPRSHM_Event.h"
acs_aeh_evreport CPHW_GPRSHM_Event::s_eventLog;       // Event object
using namespace std;

CPHW_GPRSHM_Event::~CPHW_GPRSHM_Event()
{
}

/**
 * @brief This method sends event report to acs_aeh_evreport. 
 *        It handles any errors encountered during the event reporting 
 *
 * @param line The line number where the event was discovered
 * @param file The file where the event was discovered
 * @param problemData Free text description to be logged.
   @param problemText Free text description to be printed 
 * @param eventNumber An event identity number if needed (optional) 
*/
void CPHW_GPRSHM_Event::reportEvent(const int line,
                                    const char* file,
                                    const char* problemData,
                                    const char* problemText,
                                    long eventNumber)
{
        using namespace std;

        ostringstream objRef;
        objRef << "File: " << file << ", Line: " << line;

        cout << FILENAME << endl;
        ACS_AEH_ReturnType ret = s_eventLog.sendEventMessage("cphw_gprshmd",
                                                              eventNumber,
                                                              "EVENT",
                                                              "-",
                                                              "APZ",
                                                              objRef.str().c_str(),
                                                              problemData,
                                                              problemText);

        TRACE((LOG_LEVEL_DEBUG, "AEH Event: cphw_gprshmd : %s, %s, %s", 0, objRef.str().c_str(), 
               problemData, problemText));
        if (ret == ACS_AEH_error)
        {
                TRACE((LOG_LEVEL_ERROR, "acs_aeh_EH_EvReport error, code: %d", 0, s_eventLog.getError()));
                TRACE((LOG_LEVEL_ERROR, "ACS error description: %s", 0, s_eventLog.getErrorText()));
        }

}

