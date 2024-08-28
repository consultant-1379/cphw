/*
 * SwMauOI.h
 *
 *  Created on: Apr 11, 2016
 *      Author: xdtthng
 */

#ifndef SWMAUOI_H_
#define SWMAUOI_H_

#include <vector>
#include <boost/thread/thread.hpp>

#include "acs_apgcc_oihandler_V3.h"
#include "acs_apgcc_objectimplementerinterface_V3.h"
#include "SwMauOM.h"
#include "MausSysInfo.h"

class CpTableObserver;

class SwMauOI : public acs_apgcc_objectimplementerinterface_V3
{
public:

	struct ModAttr
	{
		enum {MOD_ATTR_SUCCESS = 0, MOD_ATTR_ERROR = 1};

		ModAttr()  { reset(); }
		void reset() { m_cpType = m_attr = m_attrName = -1; m_completeStatus = MOD_ATTR_SUCCESS;}

		~ModAttr() {}

		int m_cpType;
		int m_attr;
		int m_attrName;		// SwMauOM::AdministrativeState for now
		int m_completeStatus;
	};


	SwMauOI(int architecture, int* mauType, SysInfo::EventT&);
	~SwMauOI();

    virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **attr);
    virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);
    virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);
    virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

    virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);
    virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);
    virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);
    virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle,ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);


    void run();
    int start();
    int stop();
    void join() { m_oiThread.join(); }

private:
    bool					m_running;

    // OI Handler
    acs_apgcc_oihandler_V3 	m_oiHandler;

    // Cmw event
	Maus_Event				m_endEvent;

	// OI main thread
	boost::thread			m_oiThread;

	// Only interested in Virtual or not Virtual
	int						m_architecture;

	// Changing of mauType from MAUB to MAUS and vice versa
	SysInfo::EventT&		m_mauChangeEvents;

	std::vector<ModAttr>	m_modAttr;

	// Object Manager implementation
	SwMauOM					m_om;

	// should be the last data member ???
	int						m_mauType[2];

};

#endif /* SWMAUOI_H_ */
