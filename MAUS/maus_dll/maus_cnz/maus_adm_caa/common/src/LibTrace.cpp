/*
 * maus_lib_trace.cpp
 *
 *  Created on: May 17, 2015
 *      Author: xdtthng
 */

#include "LibTrace.h"

#ifdef USE_ACS_TRACE_DEBUG

void maus_log(int32_t level, int32_t type, const char *str)
{
	// Set level to TRACE for now
	//LIB_TRACE((level, "%d, %s", type, string));
	//LIB_TRACE((0, "%d, %d, %s", level, type, string));
	LIB_TRACE(("%d, %d, %s", level, type, str));
}

#endif
