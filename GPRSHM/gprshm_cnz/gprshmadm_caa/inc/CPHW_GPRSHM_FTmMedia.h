/****************************************************************************/
/**
 *  CPHW_GPRSHM_FTmMedia.h
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
 *  This is class declaration for CPHW_GPRSHM_FTmMedia class.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-08  xjoschu  Clean-up after internal review.
*/

#ifndef _CPHW_GPRSHM_FTMMEDIA_H
#define _CPHW_GPRSHM_FTMMEDIA_H

#include "FTmMedia.hxx"

class CPHW_GPRSHM_FTmMedia : public FTmMedia
{
public:
   struct Vlan
   {
      char* name;
      char ip[16];
   };

   /**
    * @brief Constructor
    *
    * @param[in] ftm_name Name of FTm endpoint
    */
   CPHW_GPRSHM_FTmMedia(const char* ftm_name);

   /**
    * @brief Destructor
    */
   virtual ~CPHW_GPRSHM_FTmMedia() {}

   /**
    * @brief This unction is used by FTm to log its events
    */
   static int logEvent(void*, const char*, ...);

   /**
    * @brief This function is used by FTm to log its errors
    */
   static int logError(void*, const char*, ...);

   /**
    * @brief This function is used by FTm to allocate the memory for its usage
    */
   static void* allocMem(void*, size_t);

   /**
    * @brief This function is used by FTm to free the memory that has been allocated.
    */
   static void freeMem(void*, void*);

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
   virtual bool joinGroup(const char*      endpoint_name,
                          const char*      group_name,
                          FTmEventHandler* ftm_event_handler,
                          bool             is_msg_ftm);

   /**
    * @brief Set endpoint format, which help to convert an endpoint ID to endpoint name.
    *
    * @param endpoint_format    Format of the endpoint name.
    * @return true if set successfully, false otherwise.
    */
   virtual bool setEndpointFormat(const char* endpoint_format);

protected:
   enum
   {
      MaxLogLen = 1024
   };

private:
   /**
    * @brief The copy constructor is declared but not implemented to prohibit
    * copying of instantiated CPHW_GPRSHM_FTmMedia.
    */
   CPHW_GPRSHM_FTmMedia(const CPHW_GPRSHM_FTmMedia&);

   /**
    * @brief The assignment operator is defined but not implemented in order to
    * prevent copying of FTmMedia members
    */
   CPHW_GPRSHM_FTmMedia& operator=(const CPHW_GPRSHM_FTmMedia*);

   /* Format string to form endpoint name from endpoint ID */
   char m_ftmEndpointFormat[MaxStringLen];
};

#endif /* _CPHW_GPRSHM_FTMMEDIA_H */

