#include <iostream>
#include <stdint.h>
#include <sstream>

#include "/vobs/mas/mas_anz/maus_dll/maus_cnz/maus_adm_caa/maus/inc/MausDllApi.hxx"

using namespace std;
using namespace MausDllApi;

MausCore::~MausCore()
{

}

MausCore::MausCore()
{

}

class MausCoreImpl: public MausCore
{
public:
	MausCoreImpl(): MausCore(), m_logFn(0)
	{

	}

	~MausCoreImpl() {};
	void maus_poll();
	ResultCode maus_create(const MausInfo&, LogFn logFn = 0);
	ResultCode maus_destroy();
	bool isInitialized();
    ResultCode maus_get_quorum_info(MausQuorum& quorum);
    ResultCode maus_set_mask(int32_t mask);
    ResultCode maus_property(int32_t property, void* value, uint32_t* vallen);


	LogFn m_logFn;
};

void MausCoreImpl::maus_poll()
{
	static size_t count = 0;
	static const size_t MAX_COUNT = 6000;

	uint32_t sleepTimeInMilliSeconds = 4;

	int32_t sec = sleepTimeInMilliSeconds / 1000;
	int32_t msec = sleepTimeInMilliSeconds % 1000;
	struct timespec req;
	req.tv_sec = sec;
	req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
	nanosleep(&req, NULL);


	if (++count % MAX_COUNT == 0)
	{
		if (m_logFn)
		{
			ostringstream os;
			os << "MausCoreImpl::maus_poll() traced out every " << MAX_COUNT << " polls" << flush;
			m_logFn(0, 9876, os.str().c_str());
		}
	}


}

MausCore::ResultCode MausCoreImpl::maus_create(const MausInfo&, LogFn logFn)
{
	m_logFn = logFn;
	return MausCore::OK;
}

MausCore::ResultCode MausCoreImpl::maus_destroy()
{
	return MausCore::OK;
}

bool MausCoreImpl::isInitialized()
{
	return true;
}

MausCore::ResultCode MausCoreImpl::maus_get_quorum_info(MausQuorum&)
{
	return MausCore::OK;
}

MausCore::ResultCode MausCoreImpl::maus_set_mask(int32_t) 
{
	return MausCore::OK;
}

MausCore::ResultCode MausCoreImpl::maus_property(int32_t, void*, uint32_t*)
{
	return MausCore::OK;
}

namespace MausDllApi
{
	extern "C" MausCore* createMausCoreInstance()
	{
	   // Create an instance of MausCore
	   return new MausCoreImpl();
	}
}


