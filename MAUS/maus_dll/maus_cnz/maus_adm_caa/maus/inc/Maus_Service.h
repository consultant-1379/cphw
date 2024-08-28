/*
 * AP_MAUS.h
 *
 *  Created on: May 9, 2014
 *      Author: xdtthng
 */

#ifndef MAUS_SERVICE_H_
#define MAUS_SERVICE_H_

#include "boost/shared_ptr.hpp"
#include "ACS_APGCC_ApplicationManager.h"
//#include "Maus_ApplicationManager.h"
#include "Maus_Application.h"
#include <string>

class Maus_Service: public ACS_APGCC_ApplicationManager
{
public:
	Maus_Service(const std::string& daemonName, int instance = 1);
	virtual ~Maus_Service();
	void start();

	ACS_APGCC_ReturnType performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT);
	ACS_APGCC_ReturnType performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT);
	ACS_APGCC_ReturnType performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT);
	ACS_APGCC_ReturnType performComponentTerminateJobs(void);
	ACS_APGCC_ReturnType performComponentRemoveJobs(void);
	ACS_APGCC_ReturnType performApplicationShutdownJobs(void);
	ACS_APGCC_ReturnType performComponentHealthCheck(void);

private:
	int					m_instance;
	const std::string&	m_daemonName;

	boost::scoped_ptr<Maus_Application> m_maus;

};


#endif /* MAUS_SERVICE_H_ */
