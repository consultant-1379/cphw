//******************************************************************************
//
//  NAME
//
//******************************************************************************
#ifndef MAUS_TRACE_TRACE_H_
#define MAUS_TRACE_TRACE_H_


#include <stdarg.h>
#include <ACS_TRA_trace.h>
#include <ACS_TRA_Logging.h>


class MausTrace
{
public:
	static ACS_TRA_trace* 	s_trace;
	static ACS_TRA_Logging* s_logging;

	// Not allowed to instantiate an object of this class
	MausTrace(const char* fileName, int lineNumb, ACS_TRA_LogLevel level):
		file(MausTrace::fileWithoutPath(fileName)),line(lineNumb), mLevel(level) { };
	void Trace(const char* messageFormat, ...);
	static const char* fileWithoutPath(const char* file);

	static void initialise(const char* name);
	static void finalise();

private:
	const char* file;
	const int line;
	ACS_TRA_LogLevel mLevel;

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

#define TRACE_FATA(_p1) \
        MausTrace(__FILE__,__LINE__, LOG_LEVEL_FATAL).Trace _p1


#endif // end of ifndef MAUS_TRACE_TRACE_H_
