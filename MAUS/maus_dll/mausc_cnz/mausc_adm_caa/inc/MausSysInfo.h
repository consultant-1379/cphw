/*
 * MausSysInfo.h
 *
 *  Created on: Mar 14, 2015
 *      Author: xdtthng
 */

/*
NAME
   File_name: MausSysInfo.h

COPYRIGHT Ericsson AB, Sweden 2015. All rights reserved.

   The Copyright to the computer program(s) herein is the property of Ericsson AB, Sweden.
   The program(s) may be used and/or copied only with the written permission from
   Ericsson AB or in accordance with the terms and conditions
   stipulated in the agreement/contract under which the program(s) have been
   supplied.

DESCRIPTION

   Provide appropriate system information

DOCUMENT NO
   190 89-CAA 109

AUTHOR
   2015-01-16 by DEK/XDTTHNG Thanh Nguyen

SEE ALSO


Revision history
----------------
2014-06-05 Thanh Nguyen Created

*/

#ifndef MAUS_SYS_INFORMATION_H_
#define MAUS_SYS_INFORMATION_H_

#include "maus_event.h"

class SysInfo
{
public:

	enum {
		MaxEvent = 4
	};
	typedef Maus_Event EventT[MaxEvent];

	static void getSysInfo(int& architecture, int mauType[]);
	static void getCpId(int cpId[]);

private:
	static int s_cpId[2];
};

#endif // MAUS_SYS_INFORMATION_H_
