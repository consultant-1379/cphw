/****************************************************************************/
/**
 *  Event.h
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
 *  This file declares and defines the event which is used for select() function.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 */

#ifndef _EVENT_H
#define _EVENT_H

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <boost/noncopyable.hpp>
#include "CriticalSection.h"

class Event : private boost::noncopyable
{
public:
   enum STATE
   {
      INVALID,
      IDLE,
      WRITE,
      READ
   };

   Event() :
      mState(INVALID),
      mError(0),
      mFd(0)
   {
      int ret = eventfd(0, 0);
      if (ret == -1)
      {
         mError = errno;

      }
      else
      {
         AutoCS a(mCs);
         mFd = ret;
         fcntl(mFd, F_SETFL, O_NONBLOCK);
         mState = IDLE;
      }
   }

   ~Event()
   {
      close(mFd);
      mState = INVALID;
   }

   int getFd()
   {
      return mFd;
   }

   bool setEvent()
   {
      uint64_t u = 1ULL;

      mCs.enter();
      if (mState != IDLE)
      {
         mCs.leave();
         return false;
      }
      mCs.leave();

      bool res = write(mFd, &u, sizeof(uint64_t)) == sizeof(uint64_t);
      mError = errno;

      AutoCS a(mCs);
      mState = res ? WRITE : IDLE;
      return res;
   }

   bool resetEvent()
   {
      uint64_t u = 1ULL;

      mCs.enter();
      if (mState != WRITE)
      {
         mCs.leave();
         return false;
      }
      mCs.leave();

      int ret;
      do {
         ret = read(mFd, &u, sizeof(uint64_t));
         mError = errno;
         u = 0ULL;
         //memset(&u, '0', sizeof(uint64_t));
      }
      while (!(ret == -1 && mError == EAGAIN));

      AutoCS a(mCs);
      mState = IDLE;
      return true;
   }

private:
   STATE           mState;
   int             mError;
   int             mFd;
   CriticalSection mCs;
};

#endif /* _EVENT_H */

