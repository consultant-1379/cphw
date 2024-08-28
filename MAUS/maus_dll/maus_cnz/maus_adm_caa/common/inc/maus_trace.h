//******************************************************************************
//
//  NAME
//
//******************************************************************************
#ifndef MAUS_TRACE_TRACE_H_
#define MAUS_TRACE_TRACE_H_

// Turn off or on tracing for core dump investigation purpose
//

#define USE_ACS_TRACE_DEBUG
//#undef USE_ACS_TRACE_DEBUG

#ifdef USE_ACS_TRACE_DEBUG

#include <stdarg.h>
#include <ACS_TRA_trace.h>
#include <ACS_TRA_Logging.h>

#include <boost/thread.hpp>


class MausTrace
{
public:
	static ACS_TRA_trace* 	s_trace;
	static ACS_TRA_Logging* s_logging;

	// Not allowed to instantiate an object of this class
	MausTrace(const char* fileName, int lineNumb, ACS_TRA_LogLevel level):
		file(MausTrace::fileWithoutPath(fileName)),line(lineNumb), mLevel(level) { };

	void Trace(const char* messageFormat, ...);

	// Trace function used by mau core
	//void LibTrace(int level, const char* messageFormat, ...);
	void LibTrace(const char* messageFormat, ...);

	// This is the service function for both Trace() and LibTrace()
	void trace(const char* messageFormat, va_list args);

	static const char* fileWithoutPath(const char* file);

	static void initialise(const char* name);
	static void finalise();

private:
	const char* file;
	const int line;
	ACS_TRA_LogLevel mLevel;
	boost::mutex m_cs;


};


#define TRACE(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_TRACE).Trace _p1

#define TRACE_DEBUG(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_DEBUG).Trace _p1

#define TRACE_INFO(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_INFO).Trace _p1

#define TRACE_WARNING(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_WARN).Trace _p1

#define TRACE_ERROR(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_ERROR).Trace _p1

#define TRACE_FATAL(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_FATAL).Trace _p1

#else
#define TRACE(_p1)

#define TRACE_DEBUG(_p1)

#define TRACE_INFO(_p1)

#define TRACE_WARNING(_p1)

#define TRACE_ERROR(_p1)

#define TRACE_FATAL(_p1)


#endif // end of USE_ACS_TRACE_DEBUG


#endif // end of ifndef MAUS_TRACE_TRACE_H_
