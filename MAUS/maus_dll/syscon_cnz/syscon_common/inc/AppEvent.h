/*
 * AppEvent.h
 *
 *  Created on: Oct 22, 2015
 *      Author: xdtthng
 */

#ifndef APPEVENT_H_
#define APPEVENT_H_

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/eventfd.h>

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>


class AppEvent : private boost::noncopyable {

public:

    enum STATE {
        INVALID,
        IDLE,
        WRITE
    };

    AppEvent() : mState(INVALID), mError(0), mFd(-1) {
        int ret = eventfd(0, 0);
        if ( ret == -1) {
            mError = errno;
        }
        else {

            boost::lock_guard<boost::mutex> alock(mCs);
            mFd = ret;
            fcntl(mFd, F_SETFL, O_NONBLOCK);
            mState = IDLE;
        }
    }

    ~AppEvent() {
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


#endif /* APPEVENT_H_ */
