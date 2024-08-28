/****************************************************************************/
/**
 *  CPHW_GPRSHM_Trace.h
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
 *  This is class declaration for CPHW_GPRSHM_Trace class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-01-03  xdtkebo  Improved the TRACE macros and inline functions.
 *  2017-02-06  xdargas  Removed SetLogFilterLevel and GetLogFilterLevel methods.
 *  2017-02-13  xdargas  Removed nest trace levels.
 */

#ifndef _CPHW_GPRSHM_TRACE_H
#define _CPHW_GPRSHM_TRACE_H

#include <ACS_TRA_trace.h>
#include <ACS_TRA_Logging.h>

#include "boost/scoped_ptr.hpp"

#include <pthread.h>

#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/')+1 : __FILE__)

class CPHW_GPRSHM_Trace
{
public:
   CPHW_GPRSHM_Trace();
   ~CPHW_GPRSHM_Trace();
   void newTrace(ACS_TRA_LogLevel log_level, const char* fmt, int dummy, ...);
   void Trace(ACS_TRA_LogLevel log_level, const char* fmt, int dummy, ...);
   void Printf(ACS_TRA_LogLevel log_level, const char* fmt, int dummy, ...);

   static bool isTraceable();

   static void Initialise();
   static void Terminate();

   static const char* String(ACS_TRA_LogLevel lvl);

private:
   CPHW_GPRSHM_Trace(const CPHW_GPRSHM_Trace&);
   CPHW_GPRSHM_Trace& operator=(const CPHW_GPRSHM_Trace&);

   static boost::scoped_ptr<ACS_TRA_trace> s_trcb;
   static boost::scoped_ptr<ACS_TRA_Logging> s_logcb;
};

inline
CPHW_GPRSHM_Trace::CPHW_GPRSHM_Trace()
{
}

inline
bool CPHW_GPRSHM_Trace::isTraceable()
{
   return (s_trcb->ACS_TRA_ON());
}

#define initTRACE() \
   do { \
      CPHW_GPRSHM_Trace::Initialise(); \
   } while (0)

#define isTRACEABLE() \
   do { \
      CPHW_GPRSHM_Trace::isTraceable(); \
   } while (0)

#ifndef CONSOLE_LOG

#define TRACE(p1) \
   do { \
      CPHW_GPRSHM_Trace().Trace p1; \
   } while (0)
#else
#define TRACE(p1) \
   do { \
      CPHW_GPRSHM_Trace().Printf p1; \
   } while (0)

#endif

#define termTRACE() \
   do { \
      CPHW_GPRSHM_Trace::Terminate(); \
   } while (0)

#endif /* _CPHW_GPRSHM_TRACE_H */

