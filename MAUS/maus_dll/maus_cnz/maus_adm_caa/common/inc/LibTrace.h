/*
 * LibTrace.h
 *
 *  Created on: May 17, 2015
 *      Author: xdtthng
 */

#ifndef MAUS_LIB_TRACE_H_
#define MAUS_LIB_TRACE_H_

#include "maus_trace.h"

#ifdef USE_ACS_TRACE_DEBUG

extern void maus_log(int32_t type, int32_t level, const char *string);

#define LIB_TRACE(_p1) \
		MausTrace(__FILE__,__LINE__, LOG_LEVEL_TRACE).LibTrace _p1

#else

#define LIB_TRACE(_p1)

#endif

#endif /* MAUS_LIB_TRACE_H_ */
