/*
 * AppTrace.cxx
 *
 *  Created on: Oct 22, 2015
 *      Author: xdtthng
 */

#include <stdarg.h>
#include <cstring>
#include "AppTrace.h"


ACS_TRA_trace*		AppTrace::s_trace;
ACS_TRA_Logging*	AppTrace::s_logging;

void AppTrace::initialise(const char* name)
{
	const char *traceType = "C1024";

	AppTrace::s_trace = new ACS_TRA_trace(name, traceType);
	AppTrace::s_logging = new ACS_TRA_Logging();

	AppTrace::s_logging->ACS_TRA_Log_Open(name);
}

void AppTrace::finalise()
{
	delete AppTrace::s_trace;
	delete AppTrace::s_logging;
	AppTrace::s_trace = 0;
	AppTrace::s_logging = 0;
}

void AppTrace::Trace(const char* messageFormat, ...)
{
	const unsigned int TRACE_BUF_SIZE = 1024;
	char traceBuffer[TRACE_BUF_SIZE];
	char traceOutput[TRACE_BUF_SIZE];
	if ( messageFormat && *messageFormat )
	{
		try {
			va_list params;
			va_start(params, messageFormat);
			vsnprintf(traceBuffer, TRACE_BUF_SIZE - 1, messageFormat, params);
			traceBuffer[TRACE_BUF_SIZE-1]='\0';
			traceOutput[TRACE_BUF_SIZE-1]='\0';
			sprintf(traceOutput,"| %lx | %s | %d | %s",pthread_self(),fileWithoutPath(file),line,traceBuffer);

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

const char* AppTrace::fileWithoutPath(const char* fileName)
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
