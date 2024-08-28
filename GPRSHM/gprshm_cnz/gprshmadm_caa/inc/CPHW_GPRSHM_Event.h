/****************************************************************************/
/**
 *  CPHW_GPRSHM_Event.h
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
 *  This is class declaration for CPHW_GPRSHM_Event class.
 *  Used by gprshmd for all event logging  
 */

/**
 *  Revision History:
 *  -----------------
 *  2017-02-03  xmarkho	First revision created.
 */

#ifndef _CPHW_GPRSHM_EVENT_H
#define _CPHW_GPRSHM_EVENT_H

#include <string.h>
#include "acs_aeh_evreport.h"

// #define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/')+1 : __FILE__)

class CPHW_GPRSHM_Event
{
public:
   /**
    * Constructor
    */
   CPHW_GPRSHM_Event();

   /**
    * Destructor
    */
   ~CPHW_GPRSHM_Event();

   /**
    * @brief This method is called by users via MACRO EVENT. It sends the event report to 
    * acs_aeh_evreport. It handles any errors encountered during the event reporting 
    *
    * @param line The line number where the event was discovered
    * @param file The file where the event was discovered
    * @param problemData Free text description to be logged.
    * @param problemText Free text description to be printed 
    * @param eventNumber An event identity number if needed (optional) 
   */
   void reportEvent(const int line,
                    const char* file,
                    const char* problemData,
                    const char* problemText,
                    long eventNumber = 0);

private:
   static acs_aeh_evreport s_eventLog;

};

inline CPHW_GPRSHM_Event::CPHW_GPRSHM_Event() 
{
}

/**
    * Macro to make it easy for users to log events
*/
#define EVENT(p1) \
   do { \
         CPHW_GPRSHM_Event().reportEvent p1; \
   } while (0)

#endif /* _CPHW_GPRSHM_EVENT_H */

