#ifndef _DSD_SERVER_MAUS_H_
#define _DSD_SERVER_MAUS_H_

#include <map>
#include <vector>
#include <string>
#include "ACS_DSD_Server.h"
#include "ACS_DSD_Session.h"
#include "maus_event.h"
#include "AtomicFlag.h"

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

typedef boost::scoped_ptr<ACS_DSD_Server>                   ServerPtr;
typedef boost::shared_ptr<ACS_DSD_Session>                  SessionPtr;
typedef std::map<acs_dsd::HANDLE, SessionPtr>               SessionMap;
typedef std::map<acs_dsd::HANDLE, SessionPtr>::iterator     SessionMapIter;

//class CriticalSection;

class DSDServer 
{
public:
    DSDServer(const char* name, const char* domain);
    virtual ~DSDServer();

    void virtual start();
    void virtual stop();
    void join();
    void unregister();
    virtual ssize_t incomingData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes) = 0;
    virtual ssize_t handleData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes);
    
	enum ERROR_CODE { 
		NO_DSD_ERROR, 
		PUBLISH_ERROR,
		SESSION_ERROR,
		ABORTING
	};
	
	static const int MAX_NO_OF_SERVER_HANDLES = 24;
	static const int MAX_NO_OF_SESSION_HANDLES = 64;
	static const int BUFFER_SIZE = 512;
    
protected:

	void publish(void);
    void run(void);
    void closeHandle(acs_dsd::HANDLE, int&);
    void cleanup();
    bool running();
    void running(bool);

	ERROR_CODE         	mError;
	bool               	mPublishing;
	bool				mThreadRunning;
	Maus_Event 			mAbortFD;
	const std::string	mDSDsrvName;
	const std::string	mDSDsrvDomain;
	ServerPtr          	mDSDServer;
	boost::mutex 		m_cs;
	BoolFlagT			mAborting;
	BoolFlagT			mMsgHandling;

	fd_set 				mMasterFDs;
	std::vector<acs_dsd::HANDLE> mVH;
	SessionMap         	mSessions;
	boost::thread      	mDSDThread;
	
private:

	DSDServer(const DSDServer&);			// Non copyable
	DSDServer& operator=(const DSDServer&);	// Non copyable
};

inline
void DSDServer::join()
{
    mDSDThread.join();
}


#endif
