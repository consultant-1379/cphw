/*
 * ApMaus.cpp
 *
 *  Created on: Jun 5, 2014
 *      Author: xdtthng
 */


#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <fstream>
#include <sstream>
#include <dlfcn.h>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "MAUFunx.h"
#include "ApMaus.h"

#include "maus_trace.h"
#include "LibTrace.h"
#include "maus_event.h"
#include "Maus_AehEvent.h"
#include "maus_sha.h"


using namespace std;

MausIntT ApMaus::s_objCount;

namespace MausDllApi
{
	typedef MausCore* (*CreateMausCoreT)(void);
}


bool ApMaus::isMauCoreRunning()
{
	bool run = m_mcRunning;
	TRACE_DEBUG(("ApMaus::isMauCoreRunning(), <%d>", run));
	return run;
}

ApMaus::ApMaus(MausInformation& mauInfo, int instance, int slotId) : //MAUBase(mauArg, 16, true),
	//m_termEvent(),
	m_mausThread(),
	m_slotId(slotId),
	m_instance(instance),
	m_mcRunning(false),
	m_mcAborting(false),
	m_lib(0),
	m_maus(0),
	m_mausInfo(mauInfo)

{
	TRACE_DEBUG(("ApMaus::ApMaus() ctor"));

	// ApMaus is destroyed more than once
	// This now has no effect using the so lib
	// Leave it here to get a distinctive tracing in log file
	int objCount = s_objCount;
	if (objCount >=1)
	{
		TRACE_DEBUG((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
		TRACE_DEBUG(("ApMaus::ApMaus() ctor, about to crash, throw TooManyApMaus(), ApMaus obj count is <%d>", objCount));
		TRACE_DEBUG((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));

		EventReporter::instance().report(__LINE__,
				"More than one instance of ApMaus were instantiated",
				"ApMaus::ApMaus() throw TooManyApMaus()");

		// Description
		//
		// This is the last line of defence against the claim that there exists more
		// than one instance of MAUBase running
		// Note: this is only used for BT.
		//
		// Usage:
		//
		// 1) Un-comment the throw line below
		// 2) Compile this binary with the throw line
		// 3) Only in the Active node, replace maus1/maus2 with this version
		// 4) Run the test, if ap mau crashes there it is likely that
		//    possible mutltiple instance of MAUBase occurs
		//
		//throw TooManyApMaus();


	}
	objCount = ++s_objCount;

	TRACE_DEBUG((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
	TRACE_DEBUG(("ApMaus::ApMaus() ctor returns; ApMaus obj count is <%d>", objCount));
	TRACE_DEBUG((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
}

ApMaus::~ApMaus()
{
	TRACE_DEBUG(("ApMaus::~ApMaus() dtor"));

	// ApMaus is destroyed more than once
	// This now has no effect using the so lib
	// Leave it here to get a distinctive tracing in log file
	if (s_objCount == 0)
	{
		EventReporter::instance().report(__LINE__,
				"ApMaus is destructed more than once",
				"ApMaus::~ApMaus() throw TooManyClosing()");

		TRACE_DEBUG(("ApMaus::~ApMaus() dtor error; destruct more than once"));

	}

	int objCount = --s_objCount;

	TRACE_DEBUG((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
	TRACE_DEBUG(("ApMaus::~ApMaus() dtor returns, ApMaus obj count is <%d>", objCount));
	TRACE_DEBUG((">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));

	if (m_maus)
	{
		TRACE_DEBUG(("ApMaus::~ApMaus() deleting mau core instance"));
		delete m_maus;
		m_maus = 0;
	}

	if (m_lib)
	{
		TRACE_DEBUG(("ApMaus::~ApMaus() closes lib file"));
		dlclose(m_lib);
		m_lib = 0;
	}

}

int ApMaus::getQuorumdata(MausDllApi::MausCore::MausQuorum& qinfo)
{
	TRACE_DEBUG(("ApMaus::getQuorumdata()"));

	if (!m_mcRunning || m_mcAborting)
	{
		TRACE_DEBUG(("ApMaus::getQuorumdata() returns APMAUS_NOT_RUNNNG <%d>", APMAUS_NOT_RUNNNG));
		return APMAUS_NOT_RUNNNG;
	}

	// Try to get a lock
	boost::interprocess::scoped_lock<boost::mutex> alock(m_cs, boost::interprocess::try_to_lock);
	if (!alock)
	{
		TRACE_DEBUG(("ApMaus::getQuorumdata() failed to lock api returns BUSY <%d>", BUSY));
		return BUSY;
	}

	int result = APMAUS_NOT_RUNNNG;
	using namespace MausDllApi;
	MausCore::ResultCode res = m_maus->maus_get_quorum_info(qinfo);
	TRACE_DEBUG(("ApMaus::getQuorumdata() maus_get_quorum_info() returns <%d>", res));

	TRACE_DEBUG(("ApMaus::getQuorumdata() connView <%d>", qinfo.connView));
	TRACE_DEBUG(("ApMaus::getQuorumdata() quorumView <%d>", qinfo.quorumView));
	TRACE_DEBUG(("ApMaus::getQuorumdata() activeMau <%d>", qinfo.activeMau));
	TRACE_DEBUG(("ApMaus::getQuorumdata() activeMask <%d>", qinfo.activeMask));
	TRACE_DEBUG(("ApMaus::getQuorumdata() returns <%d>", res));

	switch (res)
	{
		case 	MausCore::OK:		result = SUCCESS; 	break;
		case 	MausCore::Busy:		result = BUSY;		break;
		default:					result = FAILURE;	break;
	}

	TRACE_DEBUG(("ApMaus::getQuorumdata() mask returns <%d>", result));
	return result;
}

int ApMaus::setActiveMask(int32_t mask)
{
	TRACE_DEBUG(("ApMaus::setActiveMask() mask to set <%d>", mask));

	if (!m_mcRunning || m_mcAborting)
	{
		TRACE_DEBUG(("ApMaus::setActiveMask() returns APMAUS_NOT_RUNNNG <%d>", APMAUS_NOT_RUNNNG));
		return APMAUS_NOT_RUNNNG;
	}

	// Try to get a lock
	boost::interprocess::scoped_lock<boost::mutex> alock(m_cs, boost::interprocess::try_to_lock);
	if (!alock)
	{
		TRACE_DEBUG(("ApMaus::setActiveMask() failed to lock api returns BUSY <%d>", BUSY));
		return BUSY;
	}

	int result = APMAUS_NOT_RUNNNG;
	using namespace MausDllApi;
	MausCore::ResultCode res = m_maus->maus_set_mask(mask);
	TRACE_DEBUG(("ApMaus::setActiveMask() maus_set_mask() returns <%d>", res));
	switch (res)
	{
		case 	MausCore::OK:		result = SUCCESS; 	break;
		case 	MausCore::Busy:		result = BUSY;		break;
		default:					result = FAILURE;	break;
	}

	TRACE_DEBUG(("ApMaus::setActiveMask() mask returns <%d>", result));
	return result;
}

int ApMaus::setFCState(int32_t fcState)
{
	TRACE_DEBUG(("ApMaus::setFCState() fcState to set <%d>", fcState));

	if (!m_mcRunning || m_mcAborting)
	{
		TRACE_DEBUG(("ApMaus::setFCState() returns APMAUS_NOT_RUNNNG <%d>", APMAUS_NOT_RUNNNG));
		return APMAUS_NOT_RUNNNG;
	}

	// Try to get a lock
	boost::interprocess::scoped_lock<boost::mutex> alock(m_cs, boost::interprocess::try_to_lock);
	if (!alock)
	{
		TRACE_DEBUG(("ApMaus::setFCState() failed to lock api returns BUSY <%d>", BUSY));
		return BUSY;
	}

	int result = APMAUS_NOT_RUNNNG;
	using namespace MausDllApi;
	MausCore::ResultCode res = m_maus->maus_set_fc_state(fcState);
	TRACE_DEBUG(("ApMaus::setFCState() maus_set_fc_state(<%d>) returns <%d>", fcState, res));
	switch (res)
	{
		case 	MausCore::OK:		result = SUCCESS; 	break;
		case 	MausCore::Busy:		result = BUSY;		break;
		default:					result = FAILURE;	break;
	}

	TRACE_DEBUG(("ApMaus::setFCState() returns <%d>", result));
	return result;
}


bool ApMaus::loadSo(bool checksum)
{
	TRACE_DEBUG(("ApMaus::loadSo() with checksum <%d>", checksum));

	if (checksum && MAUFunx::performChecksum(m_instance, m_slotId) > 0)
	{
		TRACE_ERROR(("ApMaus::loadSo() failed checksum"));
		return false;
	}

	string lib = MAUFunx::getMausLibName(m_instance, m_slotId);
	TRACE_DEBUG(("ApMaus::loadSo() attempt loading lib <%s>", lib.c_str()));
	if (m_lib)
	{
		dlclose(m_lib);
		m_lib = 0;
	}

	m_lib = dlopen(lib.c_str(), RTLD_LAZY);
	if (!m_lib)
	{
		ostringstream os;
		os << "Failed to open shared lib " << lib << " " << flush;
		//EventReporter::instance().report(__LINE__,
		//		os.str().c_str(),
		//		"ApMaus::loadSo(()");

		TRACE_ERROR(("%s", os.str().c_str()));
		TRACE_ERROR(("dlopen() returns <%s>", dlerror()));
		return false;
	}
	else
	{
		TRACE_DEBUG(("ApMaus::loadSo() ok load lib <%s>", lib.c_str()));
	}

	// Clear any existing error
	dlerror();

	MausDllApi::CreateMausCoreT const makeMausCore =
			reinterpret_cast<MausDllApi::CreateMausCoreT>(dlsym(m_lib, CREATE_MAU_INFO_SYMBOL));


	if (!makeMausCore)
	{
		ostringstream os;
		os << "Failed to lookup <" << CREATE_MAU_INFO_SYMBOL << ">" << flush;
		//EventReporter::instance().report(__LINE__,
		//		os.str().c_str(),
		//		"ApMaus::loadSo(()");

		TRACE_ERROR(("%s", os.str().c_str()));
		TRACE_ERROR(("dlsym() returns <%s>", dlerror()));
		return false;
	}
	else
	{
		TRACE_DEBUG(("ApMaus::loadSo() ok load symbol <%s>", CREATE_MAU_INFO_SYMBOL));
	}

	//m_maus.reset(reinterpret_cast<MausDllApi::MausCore*>(makeMausCore()));
	m_maus = makeMausCore();

	// Populate arg with values from SystemInformation
	MausDllApi::MausCore::MausInfo arg;
	memset(&arg, 0, sizeof(arg));
	//m_mausInfo
	arg.side = m_mausInfo.side;
	arg.shlf = m_mausInfo.shlf;
	arg.sysNo = m_mausInfo.sysNo;
	strcpy(arg.interface[0], m_mausInfo.interface[0]);
	strcpy(arg.interface[1], m_mausInfo.interface[1]);
	strcpy(arg.logRoot, m_mausInfo.logRoot);
	arg.mausServiceIdCode = m_mausInfo.instance;
	arg.fcState = m_mausInfo.fcState;
	arg.apgState = m_mausInfo.apgState;

	using namespace MausDllApi;
	MausCore::Version abiVer;
	abiVer.version = 1;
	arg.version = abiVer;

	MausCore::ResultCode resc = m_maus->maus_create(arg, maus_log);
	//MausCore::ResultCode resc = m_maus->maus_create(arg);
	TRACE_DEBUG(("ApMaus::loadSo() maus_create() returns <%d>", resc));

	if (resc != MausCore::OK)
	{
		TRACE_ERROR(("ApMaus::loadSo() maus_create() failed with result <%d>", resc));
		return false;
	}
	else
	{
		TRACE_DEBUG(("ApMaus::loadSo() maus_create() ok"));
	}

	return true;
}


int ApMaus::run()
{
	TRACE_DEBUG(("ApMaus::run()"));

	try
	{

		// Initialise before the run loop
		TRACE_DEBUG(("ApMaus::run() check for initialised"));

		{
			// This is fanatic.
			// Run does not have this lock to start with, best effort no guarantee.
			// However at the very beginning, it will get the lock
			//
			//boost::interprocess::scoped_lock<boost::mutex> alock(m_cs, boost::interprocess::try_to_lock);
                      bool isInitTrue=false;
                      for( int i=0; i<5; i++ )
                      {
			if (m_maus->isInitialized())
			{
				TRACE_DEBUG(("ApMaus::run() initialised done"));
                                isInitTrue=true;
                                break;
			}
			else
			{
				// AEH event goes here
				  TRACE_ERROR(("ApMaus::run() initialised failed-- Doing retry"));
                                  usleep(1000);
                                  continue;
			}
                     }

                     if(isInitTrue ==false)
                     {
				// AEH event goes here
                       TRACE_ERROR(("ApMaus::run() initialised failed --- Fatal MAx retry done"));
				  throw;
                     }
      
		}

		TRACE_DEBUG(("ApMaus::run() starts polling"));

		m_mcRunning(true);
		while (!m_mcAborting)
		{
			m_maus->maus_poll();
		}
		TRACE_DEBUG(("ApMaus::run() calling maus_destroy()"));
		m_maus->maus_destroy();
	}
	catch (...)
	{
	}

	m_mcRunning(false);
	TRACE_DEBUG(("ApMaus::run() returns"));

	return 0;
}

int ApMaus::start()
{
	TRACE_DEBUG(("ApMaus::start()"));

	m_mausThread = boost::thread(&ApMaus::run, this);

	TRACE_DEBUG(("ApMaus::start() returns <%d>", 0));
	return 0;
}

int ApMaus::stop()
{
	TRACE_DEBUG(("ApMaus::stop()"));

	// Waiting for api if being executed
	boost::interprocess::scoped_lock<boost::mutex> alock(m_cs);

	// This is set once. It is expected that destruction of ApMaus follows
	m_mcAborting(true);
	if (m_mausThread.joinable())
	{
		TRACE_DEBUG(("ApMaus::stop() before thread join"));
		m_mausThread.join();
		TRACE_DEBUG(("ApMaus::stop() after thread join"));
	}
	TRACE_DEBUG(("ApMaus::stop() returns"));

	return 0;
}
