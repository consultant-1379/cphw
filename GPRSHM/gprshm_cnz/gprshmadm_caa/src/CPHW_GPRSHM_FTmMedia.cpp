/****************************************************************************/
/**
 *  CPHW_GPRSHM_FTmMedia.cpp
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
 *  This is class implementation for CPHW_GPRSHM_FTmMedia class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-02  xjoschu  Clean-up following internal review.
 */

#include "CPHW_GPRSHM_FTmMedia.h"
#include "CPHW_GPRSHM_Trace.h"

#include <stdarg.h>
#include <string.h>
#include <cstdlib>


/**
 * @brief The constructor for the CPHW_GPRSHM_FTmMedia class
 * @param ftm_name Name of this FTm media
 */
CPHW_GPRSHM_FTmMedia::CPHW_GPRSHM_FTmMedia(const char* ftm_name) : FTmMedia(ftm_name)
{
   if (!isEnvironmentRegistered())
   {
      registerEnvironment(4 * 1024 * 1024,
                          1 * 1024 * 1024, // 1MB additional each mem alloc
                          allocMem, NULL,
                          freeMem,  NULL,
                          logEvent, NULL,
                          logError, NULL);
   }
   memset(m_ftmEndpointFormat, 0, MaxStringLen);
}

/**
 * @brief Join the GPR FTm group, and register the callback object. As this function
 * creates the Endpoint handle it should always be called only once and
 * prior to the other methods.
 *
 * @param endpoint_name      Own endpoint name.
 * @param group_name         FTm group name for GPR
 * @param ftm_event_handler  Handler who handles FTm events.
 * @param is_msg_ftm         Whether or not this endpoint is optimized for
 *                           exchanging messages between endpoints.
 *
 * @return  true if this endpoint joins successfully to a FTm group, false otherwise.
 */
bool CPHW_GPRSHM_FTmMedia::joinGroup(const char*      endpoint_name,
                                          const char*      group_name,
                                          FTmEventHandler* ftm_event_handler,
                                          bool             is_msg_ftm)
{
   TRACE((LOG_LEVEL_INFO, "%s %s", 0, FILENAME, __FUNCTION__));

   const char* group    = (group_name == NULL) ? "" : group_name;
   const char* endpoint = (endpoint_name == NULL) ? "" : endpoint_name;
   char msg[MaxLogLen];
   memset(msg, 0, MaxLogLen);
   TRACE((LOG_LEVEL_INFO, "endpoint = %s, group= %s", 0, endpoint, group));
   return FTmMedia::joinGroup(endpoint_name, group_name,
                              ftm_event_handler, is_msg_ftm);
}

/**
 * @brief Set endpoint format, which help to convert an endpoint ID to endpoint name.
 *
 * @param endpoint_format    Format of the endpoint name.
 * @return true if set successfully, false otherwise.
 */
bool CPHW_GPRSHM_FTmMedia::setEndpointFormat(const char* endpoint_format)
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   size_t len = 0;
   char msg[MaxLogLen];
   memset(msg, 0, MaxLogLen);

   if ((endpoint_format == NULL) ||
       ((len = strlen(endpoint_format)) < 1))
   {
      snprintf(msg, MaxLogLen - 1,
               "FTm[%s].endpoint[%s].state[%s].%s(): "
               "Empty/non-existing endpoint format is not valid",
               m_ftmName, m_ftmEndpointName, stateName(), __FUNCTION__);
      TRACE((LOG_LEVEL_ERROR, "%s", 0, msg));
      return false;
   }
   else if (len >= MaxStringLen)
   {
      snprintf(msg, MaxLogLen - 1,
               "FTm[%s].endpoint[%s].state[%s].%s(): "
               "endpoint format '%s' is too long",
               m_ftmName, m_ftmEndpointName, stateName(),
               __FUNCTION__, endpoint_format);
      TRACE((LOG_LEVEL_ERROR, "%s", 0, msg));
      return false;
   }

   memset(m_ftmEndpointFormat, 0, MaxStringLen);
   strncpy(m_ftmEndpointFormat, endpoint_format, MaxStringLen - 1);
   return true;
}

/**
 * @brief Write an entry to FTm event log.
 *
 * @param usr_ptr   User pointer.
 * @param format    Format string (in plain text) to form log entry.
 * @param ...       Other parameters to form a log entry.
 *
 * @return see printf() definition for reference of the return value.
 */
int CPHW_GPRSHM_FTmMedia::logEvent(void* usr_ptr, const char* format, ...)
{
   SUPPRESS_UNUSED_MESSAGE(usr_ptr);

   static unsigned long long suppress = 0;
   static char last_msg[MaxLogLen] = "";
   char curr_msg[MaxLogLen];
   int len = 0;

   va_list args;
   va_start(args, format);
   memset(curr_msg, 0, MaxLogLen);
   len = vsnprintf(curr_msg, MaxLogLen - 1, format, args);
   va_end(args);

   if (strcmp(last_msg, curr_msg) == 0)
   {
      suppress += 1;
   }
   else
   {
      if (suppress > 0)
      {
         /**
          * TODO: add event log, for temporary using TRACE
          */
         TRACE((LOG_LEVEL_DEBUG, "FTmEvent: [SUPPRESSED %llu TIME(S)]: %s", 0, suppress, last_msg));
         suppress = 0;
      }
      TRACE((LOG_LEVEL_INFO, "FTmEvent: %s", 0, curr_msg));
      strcpy(last_msg, curr_msg);
   }

   return len;
}

/**
 * @brief Write an entry to FTm error log.
 *
 * @param usr_ptr   User pointer.
 * @param format    Format string (in plain text) to form log entry.
 * @param ...       Other parameters to form a log entry.
 *
 * @return see printf() definition for reference of the return value.
 */
int CPHW_GPRSHM_FTmMedia::logError(void* usr_ptr, const char* format, ...)
{
   SUPPRESS_UNUSED_MESSAGE(usr_ptr);

   char msg[MaxLogLen];

   va_list args;
   va_start(args, format);
   memset(msg, 0, MaxLogLen);
   int len = vsnprintf(msg, MaxLogLen - 1, format, args);
   va_end(args);

   TRACE((LOG_LEVEL_ERROR, "FTmError: %s", 0, msg));

   return len;
}

/**
 * @brief Allocate FTm memory.
 *
 * @param  usr_ptr    User pointer.
 * @param mem_size    Size of a memory needs to be allocated.
 *
 * @return   NON-NULL: Pointer to the allocated memory
 *           NULL:     Could not allocate memory
 */
void* CPHW_GPRSHM_FTmMedia::allocMem(void* usr_ptr, size_t mem_size)
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   SUPPRESS_UNUSED_MESSAGE(usr_ptr);
   void* mem = malloc(mem_size);

   if (mem == NULL)
   {
      char msg[MaxLogLen];
      memset(msg, 0, MaxLogLen);
      snprintf(msg, MaxLogLen - 1,
               "Could not allocate %lu bytes of memory", mem_size);

      TRACE((LOG_LEVEL_ERROR, "%s", 0, msg));
   }

   return mem;
}

/**
 * @brief Release allocated FTm memory.
 *
 * @param usr_ptr    User pointer.
 * @param mem_ptr    Pointer points to the memory that needs to be released.
 *
 */
void CPHW_GPRSHM_FTmMedia::freeMem(void* usr_ptr, void* mem_ptr)
{
   TRACE((LOG_LEVEL_DEBUG, "%s %s", 0, FILENAME, __FUNCTION__));

   SUPPRESS_UNUSED_MESSAGE(usr_ptr);

   if (mem_ptr != NULL)
   {
      free(mem_ptr);
   }
   else
   {
      TRACE((LOG_LEVEL_ERROR, "CPHW_GPRSHM_FTmMedia::freeMem. mem_ptr is null", 0));
   }
}

