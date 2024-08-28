/****************************************************************************/
/**
 *  CriticalSection.h
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
 *  This file declares the mutex object and also implements it.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 */

#ifndef _CRITICALSECTION_H
#define _CRITICALSECTION_H

#include "boost/thread/recursive_mutex.hpp"

class CriticalSection
{
public:
   CriticalSection() : m_cs() {}
   ~CriticalSection() {}
   void enter() { m_cs.lock(); }
   void leave() { m_cs.unlock(); }

private:
   boost::recursive_mutex  m_cs;
};

class AutoCS
{
public:
   AutoCS(CriticalSection& cs) : m_cs(cs) { m_cs.enter(); }
   ~AutoCS() { m_cs.leave(); }

private:
   CriticalSection& m_cs;
};

#endif /* _CRITICALSECTION_H */

