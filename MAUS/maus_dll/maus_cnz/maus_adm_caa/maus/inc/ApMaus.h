/*
 * ApMaus.h
 *
 *  Created on: Jun 5, 2014
 *      Author: xdtthng
 */

/*
NAME
   File_name: ApMaus.h

COPYRIGHT Ericsson AB, Sweden 2015. All rights reserved.

   The Copyright to the computer program(s) herein is the property of Ericsson AB, Sweden.
   The program(s) may be used and/or copied only with the written permission from
   Ericsson AB or in accordance with the terms and conditions
   stipulated in the agreement/contract under which the program(s) have been
   supplied.

DESCRIPTION
   MAUS functionality for Dual Sided CP

DOCUMENT NO
   190 89-CAA 109

AUTHOR
   2014-06-05 by DEK/XDTTHNG Thanh Nguyen

SEE ALSO


Revision history
----------------
2014-06-05 Thanh Nguyen Created

*/


#ifndef APMAUS_H_
#define APMAUS_H_

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "boost/scoped_ptr.hpp"

#include "MausDllApi.hxx"
#include "maus_event.h"
#include "AtomicFlag.h"
#include "Maus_Atomic.h"
#include "Maus_Information.h"
#include "AtomicFlag.h"

class ApMaus
{
public:


	enum ResultCode
	{
		SUCCESS 			= 0,
		BUSY				= 1,
		FAILURE				= 2,
		FATALERROR			= 3,
		APMAUS_NOT_RUNNNG	= 4,
		INTERNAL_ERROR		= 255
	};

	// Constructor passing configuration information in to MAUBase
	ApMaus(MausInformation& mauInfo, int instance, int slotId);
	~ApMaus();

	// Run the main polling loop in MAUBase
	int run();

	// Start the thread that executing the main polling loop in the MAUBase
	int start();

	// If possible, abort all current tasks, then stop
	int stop();

	// Check if MAUBase is running
	bool isMauCoreRunning();

	// Loading the shared object
	bool loadSo(bool checksum = false);

	// Read quorum data
	int getQuorumdata(MausDllApi::MausCore::MausQuorum& qinfo);

	// Set active mask
	int setActiveMask(int32_t mask);

	// Set function change state
	int setFCState(int32_t fcState);

private:

	//Maus_Event			m_termEvent;
	boost::thread		m_mausThread;
	int					m_slotId;
	int					m_instance;
	BoolFlagT			m_mcRunning;		// Write by run loop thread
	BoolFlagT			m_mcAborting;		// Read by run loop thread, write by cmw thread
	void*				m_lib;

	//boost::scoped_ptr<MausDllApi::MausCore> m_maus;
	MausDllApi::MausCore* m_maus;

	MausInformation&	m_mausInfo;
	boost::mutex		m_cs;

	ApMaus(const ApMaus&);
	ApMaus& operator=(const ApMaus&);

	static MausIntT			s_objCount;

};

#endif // APMAUS_H_
