/*
NAME
    File_name: MauscService.h

Ericsson AB

    COPYRIGHT Ericsson AB, Sweden 2000. All rights reserved.

    The Copyright to the computer program(s) herein is the property of
    Ericsson AB, Sweden. The program(s) may be used and/or
    copied only with the written permission from Ericsson AB
    or in accordance with the terms and conditions stipulated in the
    agreement/contract under which the program(s) have been supplied.

DESCRIPTION

	This is the interface to AMF via ACS APG CC Application wrapper.
	It starts the CpMauM OI and OM.
	An observer for changes of mauType in ACS CP Table is also started

DOCUMENT NO
    190 89-CAA 109

AUTHOR
    2015-02-11 by XDT/DEK xdtthng


SEE ALSO
    -

Revision history
----------------
2015-02-11 xdtthng Created

*/

#ifndef MAUS_CENTRAL_H_
#define MAUS_CENTRAL_H_

#include "ACS_APGCC_ApplicationManager.h"
#include "boost/thread.hpp"
#include "SwMauOI.h"
#include "MausCpTableObserver.h"
#include "MausSysInfo.h"
#include "MAServer.h"

class MauscService : public ACS_APGCC_ApplicationManager
{

public:

	// Construction and destruction according to ACS AGPCC Application Manager
    MauscService();
    virtual ~MauscService();

    // Activate AMF service event loop
    void run();

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performComponentHealthCheck(void);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performComponentTerminateJobs(void);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performComponentRemoveJobs (void);

    // Callback function according to ACS APGCC Application Manager
    ACS_APGCC_ReturnType performApplicationShutdownJobs(void);

    static const char *s_daemonName;

private:

    // Start CpMauM OM and OI and CS CP Table observer
    bool start();

    // Stop CpMauM OM and OI and CS CP Table observer
    void stop();

private:
    boost::scoped_ptr<SwMauOI> 			m_oi;
    boost::scoped_ptr<CpTableObserver>	m_observer;
    boost::scoped_ptr<MAServer> 		m_maServer;

    SysInfo::EventT						m_change;

};

#endif /* MAUS_CENTRAL_H_ */
