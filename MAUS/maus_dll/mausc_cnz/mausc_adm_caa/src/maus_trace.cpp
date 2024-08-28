//******************************************************************************
//
//
//******************************************************************************
#include <stdarg.h>
#include <cstring>
#include "maus_trace.h"


ACS_TRA_trace*		MausTrace::s_trace;
ACS_TRA_Logging*	MausTrace::s_logging;

void MausTrace::initialise(const char* name)
{
	const char *traceType = "C1024";
	//MausTrace::s_trace = new ACS_TRA_trace("MAUS", traceType);
	MausTrace::s_trace = new ACS_TRA_trace(name, traceType);
	MausTrace::s_logging = new ACS_TRA_Logging();

	MausTrace::s_logging->ACS_TRA_Log_Open(name);
}

void MausTrace::finalise()
{
	delete MausTrace::s_trace;
	delete MausTrace::s_logging;
	MausTrace::s_trace = 0;
	MausTrace::s_logging = 0;
}

void MausTrace::Trace(const char* messageFormat, ...)
{
	const unsigned int TRACE_OUTPUT_SIZE = 1024;
	const unsigned int TRACE_BUF_SIZE = TRACE_OUTPUT_SIZE - 128;

	char traceBuffer[TRACE_BUF_SIZE];
	char traceOutput[TRACE_OUTPUT_SIZE];
	memset(traceOutput, 0, TRACE_OUTPUT_SIZE);
	memset(traceBuffer, 0, TRACE_BUF_SIZE);

	if ( messageFormat && *messageFormat )
	{
		try {
			va_list params;
			va_start(params, messageFormat);
			vsnprintf(traceBuffer, TRACE_BUF_SIZE - 1, messageFormat, params);
			//traceBuffer[TRACE_BUF_SIZE-1]='\0';
			//traceOutput[TRACE_OUT_BUF_SIZE-1]='\0';
			sprintf(traceOutput,"| %lx | %s | %d | %s",pthread_self(),fileWithoutPath(file),line,traceBuffer);

			traceOutput[TRACE_OUTPUT_SIZE-1]='\0';
			if (s_trace && s_trace->isOn())
			{
				s_trace->ACS_TRA_event(1,traceOutput);
			}
			if (s_logging) {
				s_logging->ACS_TRA_Log(traceOutput,mLevel);
			}
			va_end(params);
		}
		catch (...) {
			// What can we say?
		}
	}
}


const char* MausTrace::fileWithoutPath(const char* fileName)
{
	if(fileName ==0)
	{
		return "Unknown";
	}

	const char* fileNameStart = strrchr(fileName, '/');

	if(fileNameStart != NULL && (fileNameStart[1] != '\0'))
	{
		return fileNameStart + 1;
	}
	else
	{
		return fileName;
	}
}


