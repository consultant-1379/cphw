/*
NAME
    File_name: maus_event.h

Ericsson AB

    COPYRIGHT Ericsson AB, Sweden 2014. All rights reserved.

    The Copyright to the computer program(s) herein is the property of
    Ericsson AB, Sweden. The program(s) may be used and/or copied only
    with the written permission from Ericsson AB or in accordance with
    the terms and conditions stipulated in the agreement/contract under
    which the program(s) have been supplied.

DESCRIPTION
    This class wraps linux eventfd() function

DOCUMENT NO
    190 89-CAA 109 xxxx TBD

AUTHOR
    2014-05-15 by XDT/DEK/XDTTHNG

SEE ALSO
    -

Revision history
----------------
2014-05-15 xdtthng Created

*/

#ifndef MAUS_EVENT_FD_H_
#define MAUS_EVENT_FD_H_


#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <boost/thread/lock_guard.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>


class Maus_Event : private boost::noncopyable {

public:

    enum STATE {
        INVALID,
        IDLE,
        WRITE
    };

    Maus_Event() : mState(INVALID), mError(0), mFd(-1) {
        int ret = eventfd(0, 0);
        if ( ret == -1) {
            mError = errno;
        }
        else {
            //boost::mutex::scoped_lock lock(mCs);
            boost::lock_guard<boost::mutex> alock(mCs);
            mFd = ret;
            fcntl(mFd, F_SETFL, O_NONBLOCK);
            mState = IDLE;
        }
    }

    ~Maus_Event() {
        close(mFd);
        mState = INVALID;
    }

    int get() const {
        return mFd;
    }

    bool set() {

        uint64_t u = 1ULL;

        //boost::mutex::scoped_lock lock(mCs);
        boost::lock_guard<boost::mutex> alock(mCs);
        if (mState != IDLE) {
            return false;
        }

        bool res = write(mFd, &u, sizeof(uint64_t)) == sizeof(uint64_t);
        mError = errno;

        mState = res? WRITE : IDLE;
        return res;
    }

    bool reset() {

        uint64_t u = 1ULL;
        //boost::mutex::scoped_lock lock(mCs);
        boost::lock_guard<boost::mutex> alock(mCs);
        if (mState != WRITE) {
            return false;
        }

        int ret;
        do {
            ret = read(mFd, &u, sizeof(uint64_t));
            mError = errno;
            u = 0ULL;
        } while (!(ret == -1 && mError == EAGAIN));

        mState = IDLE;
        return true;
    }

private:

    STATE           mState;
    int             mError;
    int             mFd;
    boost::mutex  	mCs;
};

#endif
