/*
 * MausOI.h
 *
 *  Created on: Jan 17, 2015
 *      Author: xdtthng
 */

#ifndef MAUSOI_H_
#define MAUSOI_H_

#include <vector>
#include <boost/thread/thread.hpp>

#include "acs_apgcc_oihandler_V3.h"
#include "acs_apgcc_objectimplementerinterface_V3.h"
#include "MausOM.h"
#include "MausSysInfo.h"

class CpTableObserver;

class MausOI : public acs_apgcc_objectimplementerinterface_V3
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
		int m_attrName;		// MausModel::AdministrativeState
							// MausModel::ChangeState
		int m_completeStatus;
	};

	MausOI(int, int*, SysInfo::EventT&);
	~MausOI();

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
    ACS_CC_ReturnType updateRuntime(MausModel::CpTypeT cpType);

    ACS_CC_ReturnType updateRuntimeAttr(const std::string& objectName,
                               const std::string& attrName, int value, bool useUnsigned = true);

    void join() { m_oiThread.join(); }

private:
    bool					m_running;
    // OI Handler
    acs_apgcc_oihandler_V3 	m_oiHandler;
	Maus_Event				m_endEvent;
	boost::thread			m_oiThread;
	int						m_architecture;
	SysInfo::EventT&		m_mauChangeEvents;
	std::vector<ModAttr>	m_modAttr;
	MausOM					m_om;

	// should be the last data member ???
	int						m_mauType[2];


};

#endif // MAUSOI_H_
