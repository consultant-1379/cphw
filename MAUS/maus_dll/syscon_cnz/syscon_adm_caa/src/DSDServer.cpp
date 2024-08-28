#include <algorithm>
#include <utility>

#include "DSDServer.h"
#include "AppTrace.h"

using namespace std;

const int DSDServer::MAX_NO_OF_SERVER_HANDLES;
const int DSDServer::MAX_NO_OF_SESSION_HANDLES;
const int DSDServer::BUFFER_SIZE;


bool DSDServer::running()
{
	boost::lock_guard<boost::mutex> alock(m_cs);
	return mThreadRunning;
}

void DSDServer::running(bool val)
{
	boost::lock_guard<boost::mutex> alock(m_cs);
	mThreadRunning = val;
}

DSDServer::DSDServer(const char* name, const char* domain): mError(NO_DSD_ERROR),
		mPublishing(false),
		mThreadRunning(false),
		mAbortFD(),
		mDSDsrvName(name),
		mDSDsrvDomain(domain),
		mDSDServer(new ACS_DSD_Server(acs_dsd::SERVICE_MODE_INET_SOCKET)),
		m_cs(),
		mAborting(false),
		mMsgHandling(false),
		mMasterFDs(),
		mVH(),
		mSessions(),
		mDSDThread()

{
	TRACE_DEBUG(("DSDServer::DSDServer()"));
}

DSDServer::~DSDServer()
{
	try {
		TRACE_DEBUG(("DSDServer::~DSDServer()"));

		// Stop the running thread first
		// -----------------------------

		// Must wait for DSD thread return
		if (running()) {
			TRACE_DEBUG(("DSDServer::~DSDServer() while thread is still running"));
			mDSDThread.join();
		}
		this->unregister();
	}
	catch (...) {

	}
	mAborting(false);
	TRACE_DEBUG(("DSDServer::~DSDServer() returns"));

}

void DSDServer::cleanup()
{
	TRACE_DEBUG(("DSDServer::cleanup()"));
	mVH.clear();
	for (SessionMapIter it = mSessions.begin(); it != mSessions.end();)
	{
		it->second->close();
		mSessions.erase(it++);
	}
	mSessions.clear();
	running(false);
	TRACE_DEBUG(("DSDServer::cleanup() returns"));
}

void DSDServer::unregister()
{
	TRACE_DEBUG(("DSDServer::unresigter()"));
	if (mDSDServer.get()) {
		mDSDServer->unregister();
		mDSDServer->close();
	}
	TRACE_DEBUG(("DSDServer::unresigter() returns"));
}

void DSDServer::publish(void)
{
	TRACE_DEBUG(("DSDServer::publish()"));

	// Try indefinitely until successfull or get aborted
	// Setting up infinite trial loop

	fd_set masterFDs;       // Master file descriptor list or event list
	fd_set readFDs;         // Temp file descriptor list
	struct timeval tv;      // Sleeping time between each trial

	// Reset the FD list
	FD_ZERO(&masterFDs);
	FD_ZERO(&readFDs);
	FD_SET(mAbortFD.get(), &masterFDs);
	int maxFD = mAbortFD.get();   // The range of FDs; wait for single object
	mPublishing = true;
	int retval = 0;
	bool running = true;

	while (running) {

		readFDs = masterFDs;
		tv.tv_sec = 0;				// Sleep time in micro second
		tv.tv_usec = 100 * 1000L;   // Sleeps 100 ms before each trial

		if (retval == 0) {  // Time out, attempt publishing
			if (mDSDServer->open(acs_dsd::SERVICE_MODE_INET_SOCKET) < 0) {
				mError = PUBLISH_ERROR;
				TRACE_DEBUG(("DSDServer::publish() fails to open DSD server"));
				TRACE_DEBUG(("%s", mDSDServer->last_error_text()));
			}
			//else if (mDSDServer->publish("BUSRVDSD","SBC") < 0) {
			else if (mDSDServer->publish(mDSDsrvName, mDSDsrvDomain, acs_dsd::SERVICE_VISIBILITY_GLOBAL_SCOPE) < 0) {
				mError = PUBLISH_ERROR;
				mDSDServer->close();
				TRACE_DEBUG(("DSDServer::publish() fails to publish DSD server"));
				TRACE_DEBUG(("%s", mDSDServer->last_error_text()));
			}
			else {
				// Successfully publish DSD sever; stop this infinite loop
				mAbortFD.reset();
				running = false;
				mError = NO_DSD_ERROR;
				TRACE_DEBUG(("DSDServer::publish() OK; Ready for use"));
				mPublishing = false;
				break;
			}
		}
		else if (FD_ISSET(mAbortFD.get(), &readFDs)) {
			mAbortFD.reset();
			running = false;
			mError = ABORTING;
			TRACE_DEBUG(("DSDServer::publish() is aborted"));
			mPublishing = false;
			break;
		}
		retval = select(maxFD + 1, &readFDs, NULL, NULL, &tv);
	}
}

void DSDServer::run()
{
	TRACE_DEBUG(("DSDServer::run()"));

#if 0
	mDSDServer.reset(new (nothrow) ACS_DSD_Server(acs_dsd::SERVICE_MODE_INET_SOCKET));
	if (mDSDServer.get()) {
		TRACE_DEBUG((LOG_LEVEL_INFO, "DSDServer::publish() gets ACS_DSD_Server; attempt to publish", 0));
	}
	else {
		TRACE_DEBUG((LOG_LEVEL_ERROR, "DSDServer::publish() fails to get a new ACS_DSD_Server", 0));
		mError = PUBLISH_ERROR;
		mPublishing = false;
		return;				
	}
#endif

	this->running(true);
	publish();
	if (mError != NO_DSD_ERROR) {
		TRACE_DEBUG(("Error publishing DSD server"));
		TRACE_DEBUG(("DSDServer::run() returns"));
		cleanup();
		this->running(false);
		return;
	}

	// ------------------------------------------------------------------------
	// Server infinite loop
	//-------------------------------------------------------------------------
	//fd_set masterFDs;       // Master file descriptor list or event list
	fd_set readFDs;         // Temp file descriptor list
	struct timeval tv;      // Sleeping time between each trial
	//vector<acs_dsd::HANDLE> vH;

	// Prepare the FD list
	FD_ZERO(&mMasterFDs);
	FD_ZERO(&readFDs);
	mVH.push_back(mAbortFD.get());
	FD_SET(mAbortFD.get(), &mMasterFDs);
	int maxFD = mAbortFD.get();
	TRACE_DEBUG(("Before loop, maxFD is <%d>", maxFD));

	// Set FD list with DSD listening handles
	acs_dsd::HANDLE listenH[MAX_NO_OF_SERVER_HANDLES];
	acs_dsd::HANDLE sessionH[MAX_NO_OF_SESSION_HANDLES];
	int noListenH = MAX_NO_OF_SERVER_HANDLES;
	int noSessionH = MAX_NO_OF_SESSION_HANDLES;
	if (mDSDServer->get_handles(listenH, noListenH) >= 0) {
		TRACE_DEBUG(("DSDServer::run(); get <%d> dsd listening handles", noListenH));
		for (int i = 0; i < noListenH; ++i) {
			FD_SET(listenH[i], &mMasterFDs);
			mVH.push_back(listenH[i]);
			TRACE_DEBUG(("%d %d", i, listenH[i]));
			maxFD = max(maxFD, listenH[i]);
			TRACE_DEBUG(("%d %d %d", i, listenH[i], maxFD));
		}
		//sort(vH.begin(), vH.end());
		//maxFD = vH.back();
		//maxFD = *max_element(vH.begin(), vH.end());
		TRACE_DEBUG(("DSDServer::run(); maxFD <%d>", maxFD));
	}
	else {
		TRACE_DEBUG(("DSDServer::run() returns; Error when getting listening handles"));
		TRACE_DEBUG(("%s", mDSDServer->last_error_text() ));
		cleanup();
		this->running(false);
		return;
	}

	int retval = 0;
	bool running = true;
	acs_dsd::HANDLE aHandle, bHandle;
	while (running) {

		readFDs = mMasterFDs;
		tv.tv_sec = 0;      // Sleeps 100 ms
		tv.tv_usec =  100L * 1000L;    // Example 100 ms = 100 000 micro seconds
		//retval = select(maxFD + 1, &readFDs, NULL, NULL, &tv);
		retval = select(maxFD + 1, &readFDs, NULL, NULL, NULL);

		// Timeout; at the moment, take no action
		if (retval == 0) {
			continue;
		}


		// Not recoverable error; exit the loop
		if (retval == -1 ) {
			// A signal was caught; at the moment, take no action
			if (errno == EINTR) {
				continue;
			}

			TRACE_DEBUG(("DSDServer::run(); select() returns negative result; exiting .."));
			running = false;
			break;
		}

		// Abort event signaled
		//TRACE_DEBUG((LOG_LEVEL_INFO, "Checking for abort event", 0));
		if (FD_ISSET(mAbortFD.get(), &readFDs)) {
			mAbortFD.reset();
			closing();
			running = false;
			TRACE_DEBUG(("DSDServer::run() is aborted"));
			break;
		}

		// New connection signaled from remote end
		// Accept new connection and setup new session
		bool newConnection = false;
		for (int i = 0; i < noListenH; ++i)
		{

			if (mAborting)
				break;

			aHandle = listenH[i];
			if (FD_ISSET(aHandle, &readFDs))
			{
				TRACE_DEBUG(("listenH[%d] == <%d> signaled", i, aHandle));
				SessionPtr aSession = SessionPtr(new ACS_DSD_Session);
				if(mDSDServer->accept(*aSession) < 0)
				{
					TRACE_DEBUG(("Failed to accept signaled handle"));
					newConnection |= false;
					continue;
				}
				noSessionH = MAX_NO_OF_SESSION_HANDLES;
				if (aSession->get_handles(sessionH, noSessionH) < 0)
				{
					TRACE_DEBUG(("Failed to get session handles"));
					newConnection |= false;
					continue;
				}
				TRACE_DEBUG(("No of Sessions per connection <%d>", noSessionH));
				newConnection |= true;
				for (int j = 0; j < noSessionH; ++j)		// Store all session handles
				{
					bHandle = sessionH[j];
					FD_SET(bHandle, &mMasterFDs);
					maxFD = max(maxFD, bHandle);
					TRACE_DEBUG(("Session Handle <%d>", bHandle));
					mSessions.insert(pair<acs_dsd::HANDLE, SessionPtr>(bHandle, aSession));
					mVH.push_back(bHandle);
				}
				TRACE_DEBUG(("maxFD is <%d>", maxFD));
			}
		}

		if (mAborting)
			break;

		// Need this in case the session is erased
		//if (newConnection)
		//   continue;

		// New message from remote end signaled.
		// Receive and process new message
		SessionMapIter it = mSessions.begin();
		TRACE_DEBUG(("The size of mSessions is <%d>", mSessions.size()));

		// Should remove for released product ??
		{
			TRACE_DEBUG((">>>>>>> all handles >>>>>>>>>"));
			stringstream ss;
			copy(mVH.begin(), mVH.end(), ostream_iterator<acs_dsd::HANDLE>(ss, " "));
			TRACE_DEBUG(("Handles: <%s>", ss.str().c_str()));
		}

		while (it != mSessions.end())
		{

			if (mAborting)
				break;

			aHandle = it->first;
			if (FD_ISSET(aHandle, &readFDs))
			{
				char buf[BUFFER_SIZE];
				ssize_t nob = it->second->recv(buf, BUFFER_SIZE);
				TRACE_DEBUG(("Received message len <%d> on handle <%d>", nob, aHandle));
				if (nob <= 0)
				{
					it->second->close();
					mSessions.erase(it++);
					closeHandle(aHandle, maxFD);
					continue;
				}
				else
				{
					//ssize_t res = this->incomingData(aHandle, it->second, buf, BUFFER_SIZE);
					ssize_t res = this->handleData(aHandle, it->second, buf, BUFFER_SIZE);
					if (res <= 0 && res != -999)
					{
						it->second->close();
						mSessions.erase(it++);
						closeHandle(aHandle, maxFD);
						continue;
					}
				}
			}
			++it;
		}
		// ToDo: remember to remove this for final production
		stringstream ss;
		copy(mVH.begin(), mVH.end(), ostream_iterator<acs_dsd::HANDLE>(ss, " "));
		TRACE_DEBUG(("Handles: <%s>", ss.str().c_str()));
		TRACE_DEBUG(("maxFD is <%d>", maxFD));
		TRACE_DEBUG(("Capacity of mVH is <%d>", mVH.capacity()));
		TRACE_DEBUG(("value of FD_SETSIZE is <%d>", FD_SETSIZE));
	}

	cleanup();
	this->running(false);
	TRACE_DEBUG(("DSDServer::run() returns; DSD server thread exiting"));
}

ssize_t DSDServer::handleData(acs_dsd::HANDLE handle, SessionPtr session, char* buffer, ssize_t noOfBytes)
{
	TRACE_DEBUG(("DSDServer::handleData()"));

	mMsgHandling(true);
	ssize_t res = this->incomingData(handle, session, buffer, noOfBytes);
	mMsgHandling(false);
	TRACE_DEBUG(("DSDServer::handleData() returns"));
	return res;
}

void DSDServer::closeHandle(acs_dsd::HANDLE aHandle, int& maxFD)
{
	TRACE_DEBUG(("DSDServer::closeHandle()"));

	TRACE_DEBUG(("**** Remote connection closed; erase session"));
	FD_CLR(aHandle, &mMasterFDs);
	//TRACE_DEBUG((LOG_LEVEL_INFO, "The size of mSessions is <%d>", 0, mSessions.size()));
	mVH.erase(remove(mVH.begin(), mVH.end(), aHandle),
			mVH.end());

	if (aHandle == maxFD) {
		maxFD = *max_element(mVH.begin(), mVH.end());
		TRACE_DEBUG(("Handle <%d> removed from master list", aHandle));
		TRACE_DEBUG(("New maxFD is <%d>", maxFD));
	}
	else {
		TRACE_DEBUG(("maxFD <%d> remains unchanged", maxFD));
	}
	stringstream ss;
	copy(mVH.begin(), mVH.end(), ostream_iterator<acs_dsd::HANDLE>(ss, " "));
	TRACE_DEBUG(("Handles: <%s>", ss.str().c_str()));
	TRACE_DEBUG(("DSDServer::closeHandle() returns"));

	// Call back function
	this->closeConnection(aHandle);
}

void DSDServer::stop()
{
	TRACE_DEBUG(("DSDServer::stop()"));

	mAborting(true);
	mAbortFD.set();

	/*
	if (running()) {
		mAbortFD.set();

		while (mMsgHandling)
		{
			// Wait for the job completion
		}

		// Must wait for DSD thread return
		mDSDThread.join();
		unregister();
		//cleanup();
		//running(false);
		//}
	}
	else
	{
		TRACE(("DSDServer was already stopped."));
	}
	mAborting(false);
	*/

	TRACE_DEBUG(("DSDServer::stop(); DSD Server thread returns"));
}

void DSDServer::closing()
{
	//TRACE_DEBUG(("DSDServer::closing()"));
	//TRACE_DEBUG(("DSDServer::closing() returns"));
}

void DSDServer::start()
{
	TRACE_DEBUG(("DSDServer::start()"));

	mDSDThread = boost::thread(boost::bind(&DSDServer::run, this));
}


