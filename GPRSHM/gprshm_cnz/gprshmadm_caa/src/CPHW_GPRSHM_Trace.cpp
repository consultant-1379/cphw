/****************************************************************************/
/**
 *  CPHW_GPRSHM_Trace.cpp
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
 *  This is class implementation for CPHW_GPRSHM_Trace class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-09  xdargas  Removed the log filtering.
 *  2017-02-13  xdargas  Removed the nest trace levels.
 */

#include "CPHW_GPRSHM_Trace.h"
#include "CPHW_GPRSHM_Definitions.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

boost::scoped_ptr<ACS_TRA_trace> CPHW_GPRSHM_Trace::s_trcb;       // Trace object, used to be known as trace control block
boost::scoped_ptr<ACS_TRA_Logging> CPHW_GPRSHM_Trace::s_logcb;    // Log object

CPHW_GPRSHM_Trace::~CPHW_GPRSHM_Trace() {}

const char* CPHW_GPRSHM_Trace::String(ACS_TRA_LogLevel lvl)
{
   static const char* TXT[] = {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};
   return TXT[lvl];
}

//******************************************************************************
//	Trace()
//******************************************************************************
void CPHW_GPRSHM_Trace::Trace(ACS_TRA_LogLevel logLevel, const char* fmt, int dummy, ...)
{
   char  l_outText[10240];
   char  l_outText_log[10240];  //trautil text
   char  l_lineout[10752];
   char* l_pLine;
   char* last;

   va_list l_args;
   va_start(l_args, dummy);
   vsnprintf(l_outText, 10239, fmt, l_args);
   l_outText[10239] = '\0';
   va_end(l_args);

   memcpy(l_outText_log, l_outText, 10239);

   // Only Trace if Trace is On
   if (isTraceable())
   {
      l_pLine = strtok_r(l_outText, "\n", &last);
      while (l_pLine)
      {
         sprintf(l_lineout, "%s: %s", String(logLevel), l_pLine);
         s_trcb->ACS_TRA_event(1, l_lineout);
         l_pLine = strtok_r(NULL, "\n", &last);
      }
   }

   l_pLine = strtok_r(l_outText_log, "\n", &last);
   while (l_pLine)
   {
      sprintf(l_lineout, "%s: %s", String(logLevel), l_pLine);
      s_logcb->Write(l_lineout, logLevel);
      l_pLine = strtok_r(NULL, "\n", &last);
   }
}


//******************************************************************************
//  Prinf()
//******************************************************************************
void CPHW_GPRSHM_Trace::Printf(ACS_TRA_LogLevel log_level, const char* fmt, int dummy, ...)
{
   SUPPRESS_UNUSED_MESSAGE(log_level);

   char  l_outText[10240];
   char  l_lineout[10752];
   char* l_pLine;
   char* last;

   // At this point, either or both Trace or Log is On
   // Collect all arguments once for either or both Trace or Log
   va_list l_args;
   va_start(l_args, dummy);
   vsnprintf(l_outText, 10239, fmt, l_args);
   l_outText[10239] = '\0';
   va_end(l_args);

   time_t rawtime;
   struct tm* timeinfo;
   struct timeval now;
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   gettimeofday(&now, NULL);

   l_pLine = strtok_r(l_outText, "\n", &last);
   while (l_pLine)
   {
      sprintf(l_lineout, "%s: %s", String(log_level), l_pLine);
      printf("%d %d:%d:%d.%.6lu %s\n", timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, now.tv_usec, l_lineout);
      l_pLine = strtok_r(NULL, "\n", &last);
   }
}

//******************************************************************************
//	Terminate()
//******************************************************************************
void CPHW_GPRSHM_Trace::Terminate()
{
   s_trcb.reset();
   s_logcb.reset();
}

//******************************************************************************
//	Initialise()
//******************************************************************************
void CPHW_GPRSHM_Trace::Initialise()
{
   s_trcb.reset(new ACS_TRA_trace("", "C180"));
   s_logcb.reset(new ACS_TRA_Logging());
   s_logcb->Open("GPRSHM");
}
