/*
 * syscon_service.h
 *
 *  Created on: Oct 21, 2015
 *      Author: xdtthng
 */

#ifndef SYSCON_SERVICE_H_
#define SYSCON_SERVICE_H_


#include "boost/shared_ptr.hpp"
#include "ACS_APGCC_ApplicationManager.h"
#include "syscon_server.h"
#include <string>

class Syscon_Service : public ACS_APGCC_ApplicationManager
{

public:

	// Construction and destruction according to ACS AGPCC Application Manager
    Syscon_Service();
    virtual ~Syscon_Service();

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

    // Start Application
    bool start();

    // Stop Application
    void stop();

    boost::shared_ptr<Syscon_Server>	m_syscon;

private:

};


#endif /* SYSCON_SERVICE_H_ */
