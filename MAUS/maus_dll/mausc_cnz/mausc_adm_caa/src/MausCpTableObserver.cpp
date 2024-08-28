/*
 * MauscTableObserver.cpp
 *
 *  Created on: Mar 5, 2015
 *      Author: xdtthng
 */

#include "MausCpTableObserver.h"
#include "maus_trace.h"
#include "MAUFunx.h"
#include "CpMauM.h"
#include "SwMauOM.h"
#include "boost/filesystem.hpp"

using namespace std;
namespace fs = boost::filesystem;

const string CpTableObserver::s_dataPath =	"/data/apz/data/";
const string CpTableObserver::s_ncdSide[2] = {"boot/image/a/", "boot/image/b/"};
const string CpTableObserver::s_mcpSide[2] = {"cpa/", "cpb/"};
const string CpTableObserver::s_cpIDs[2] = {"cp1/", "cp2/"};
const string CpTableObserver::s_default[3] =	{"default", "default_1", "default_2"};
const string CpTableObserver::s_pxelinux = "pxelinux.cfg/";

void CpTableObserver::handlePxe(const string& dir, int mauType)
{
	TRACE(("CpTableObserver::handlePxeMau(dir, mauType)"));
	TRACE(("dir <%s> mauType <%d>", dir.c_str(), mauType));

	string sname = dir;
	sname += s_default[mauType];
	string dname = dir;
	dname += s_default[0];
	if (fileExists(sname))
	{
		TRACE(("copyFile source sname <%s>", sname.c_str()));
		TRACE(("copyFile destination dname <%s>", dname.c_str()));
		copyFile(sname, dname);
	}
	else
	{
		TRACE(("Non exist source sname <%s>", sname.c_str()));
	}
	TRACE(("CpTableObserver::handlePxeMau() returns"));
}

void CpTableObserver::handlePxe(int cpId, int mauType)
{
	TRACE(("CpTableObserver::handlePxe() cpId <%d> mauType <%d>", cpId, mauType));

	bool mcpSystem = isMultipleCP();
	TRACE(("CpTableObserver::handlePxe() mcpSystem <%d>", mcpSystem));

	string dir, dircp, sname, dname;
	int index = cpId - 1001;
	if (mcpSystem)
	{
		dir = s_dataPath;
		dir += s_cpIDs[index];
		for (int i = 0; i < 2; ++i)
		{
			dircp = dir;
			dircp += s_mcpSide[i];
			dircp += s_pxelinux;
			handlePxe(dircp, mauType);
		}
	}
	else
	{
		for (int i = 0; i < 2; ++i)
		{
			dir = s_dataPath;
			dir += s_ncdSide[i];
			dir += s_pxelinux;
			handlePxe(dir, mauType);
		}
	}

	TRACE(("CpTableObserver::handlePxe() returns"));
}

bool CpTableObserver::copyFile(const std::string& src, const std::string& dest)
{
	bool result = true;
    if (fileExists(dest))
    {
        deleteFile(dest);
    }

    try
    {
        fs::path psrc(src.c_str());
        fs::path pdest(dest.c_str());
        fs::copy_file(psrc, pdest);
    }
    catch (fs::filesystem_error)
    {
    	result = false;
    }
    return result;
}

bool CpTableObserver::deleteFile(const std::string& name)
{
    bool result;
    try
    {
        fs::path p(name.c_str());
        result = fs::remove(p);
    }
    catch (fs::filesystem_error)
    {
    	result = false;
    }
    return result;
}


bool CpTableObserver::fileExists(const std::string& name)
{
    string tName(name);
    bool result;

    // Remove trailing backslash
    string::iterator it = tName.end();
    if (*--it == '/')
    {
        tName.erase(it);
    }

    try
        {
            fs::path p(tName.c_str());
            result = fs::exists(p);
        }
        catch (fs::filesystem_error)
        {
        	result = false;
        }
        return result;
}


bool CpTableObserver::isMultipleCP()
{
	bool multCPSys;

	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::isMultipleCPSystem(multCPSys);
	return res != ACS_CS_API_NS::Result_Success? false : multCPSys;
}

CpTableObserver::CpTableObserver(int a, int* mauType, SysInfo::EventT& e): m_initialized(false),
		m_subMgrInstance(0),
		m_architecture(a), m_change(e)
{
	TRACE(("CpTableObserver::CpTableObserver() ctor"));
	m_mauType[0] = mauType[0];
	m_mauType[1] = mauType[1];

	// Remove the trace after basic test
	// ---------------------------------
	TRACE(("Event file descriptor [0] <%d>", m_change[0].get()));
	TRACE(("Event file descriptor [1] <%d>", m_change[1].get()));
	TRACE(("Event file descriptor [2] <%d>", m_change[2].get()));
	TRACE(("Event file descriptor [3] <%d>", m_change[3].get()));
}

CpTableObserver::~CpTableObserver()
{
	TRACE(("CpTableObserver::~CpTableObserver() dtor"));

	if (m_initialized)
	{
		ACS_CS_API_NS::CS_API_Result result = m_subMgrInstance->unsubscribeCpTableChanges(*this);

		if (result != ACS_CS_API_NS::Result_Success)
		{
			// AEH logging
			TRACE(("CpTableObserver::~CpTableObserver() failed to unsubscribe to Cp Table Change"));
		}
		else
		{
			TRACE(("CpTableObserver::~CpTableObserver() successfully unsubscribe to Cp Table Change"));
		}
	}
}

void CpTableObserver::init()
{
	TRACE(("CpTableObserver::init()"));

	// The following if is for testing the leak

#if 1


	SysInfo::getCpId(m_cpId);
	TRACE(("CpTableObserver::init(), m_cpId[0] <%d>", m_cpId[0]));
	TRACE(("CpTableObserver::init(), m_cpId[1] <%d>", m_cpId[1]));


	// Skip this in virtual
	// --------------------
	//if (m_architecture == ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED)
	//{
		// Trace out something
	//	TRACE(("CpTableObserver::init(), CEE detected, no need to subscribe to mauType changes"));
	//	return;
	//}

	// MAUSC acts upon the following change
	// - Add or Delete DualSided CP in both Native and Virtual
	// - Change of mauType which only occurs in Native
	TRACE(("CpTableObserver::init(), subscribe to mauType changes"));
	m_subMgrInstance = ACS_CS_API_SubscriptionMgr::getInstance();
	if (!m_subMgrInstance)
	{
		TRACE(("CpTableObserver::init() failed to get a subscription instance"));
		// EHA logging
		return;
	}

	// Subscribe for CpTableChanges
	ACS_CS_API_NS::CS_API_Result cs_result = m_subMgrInstance->subscribeCpTableChanges(*this);

	if (cs_result != ACS_CS_API_NS::Result_Success)
	{
		TRACE(("CpTableObserver::init() failed to subscribe to the CP Table Change"));
		// EAH logging
		return;
	}

	m_initialized = true;

#endif

#if 0

	int mauType = MAUFunx::getMauType(1);
	if (mauType == -1)
	{
		TRACE(("CpTableObserver::init() failed to get mauType for CP1"));
		// EAH logging
		return;

	}
	m_mauType[1] = mauType;

	mauType = MAUFunx::getMauType(2);
	if (mauType == -1)
	{
		m_mauType[1] = -1;
		TRACE(("CpTableObserver::init() failed to get mauType for CP2"));
		// EAH logging
		return;

	}
	m_mauType[2] = mauType;

#endif

}

void CpTableObserver::updateAdd(const ACS_CS_API_CpTableChange& observee)
{
	TRACE(("CpTableObserver::updateAdd()"));

	ACS_CS_API_CpTableData *data = observee.cpData;
	TRACE(("CpTableObserver::updateAdd(), observee.dataSize <%d>", observee.dataSize));
	TRACE(("CpTableObserver::updateAdd(), data->operationType <%d>", data->operationType));

	do
	{
		int cpId = data->cpId;
		int instance = cpId - 1001;
		if (instance != 0 && instance != 1)
		{
			TRACE(("CpTableObserver::updateDelete() out of scope cpId <%d>", cpId));
			break;
		}

		if (cpId == m_cpId[instance])
		{
			TRACE(("No change in Add operation for cpid <%d>, m_cpId[%d] <%d>", cpId, instance, m_cpId[instance]));
			break;
		}

		// Populate data for observe table
		m_mauType[instance] = MAUFunx::getMauType(cpId - 1000);
		m_cpId[instance] = cpId;
		TRACE(("Adding m_cpId[%d] = <%d> mauType <%d>", instance, cpId, m_mauType[instance]));

		// Check mau type is MAUS then create object
		if (m_mauType[instance] != MAUS)
		{
			TRACE(("No action; mauType is <%d>", m_mauType[instance]));
			break;
		}

		SwMauOM om;
		om.createObject(SwMauMom::CpTypeT(cpId - 1000));

#if 0

		// Check admin state; for now the default state at creation is LOCKED
		// This might change in the MOM
		int adminState[2];
		om.readAdminState(adminState);
		if (adminState[instance] == SwMauMom::UNLOCKED)
		{
			// Disable MAUS and delete SwMauM object
			TRACE(("updateAdd(), disable MAUS when UNLOCKED"));
			CpMauM cpMau(instance + 1);
			cpMau.enable();
		}

#endif

	}
	while (false);

	TRACE(("CpTableObserver::updateAdd() return"));
}

void CpTableObserver::updateDelete(const ACS_CS_API_CpTableChange& observee)
{
	TRACE(("CpTableObserver::updateDelete()"));

	ACS_CS_API_CpTableData *data = observee.cpData;
	TRACE(("CpTableObserver::updateDelete(), observee.dataSize <%d>", observee.dataSize));
	TRACE(("CpTableObserver::updateDelete(), data->operationType <%d>", data->operationType));

	do
	{
		int cpId = data->cpId;
		int instance = cpId - 1001;
		if (instance != 0 && instance != 1)
		{
			TRACE(("CpTableObserver::updateDelete() out of scope cpId <%d>", cpId));
			break;
		}

		if (cpId != m_cpId[instance])
		{
			TRACE(("Delete cpId <%d>, no operation; cached m_cpId[%d] <%d>", cpId, instance,m_cpId[instance]));
			break;
		}


		TRACE(("Delete operation for cpid <%d>, m_cpId[%d] <%d>", cpId, instance, m_cpId[instance]));

		//SwMauOM om;

#if 0

		int adminState[2];
		om.readAdminState(adminState);
		if (adminState[instance] == SwMauMom::UNLOCKED)
		{
			// Disable MAUS and delete SwMauM object
			TRACE(("updateDelete(), disable MAUS when UNLOCKED"));
			CpMauM cpMau(instance + 1);
			cpMau.disable();
		}

#endif

		// Delete the object
		//TRACE(("updateDelete(), delete object for cpId <%d> cpType <%d>", cpId, cpId - 1000));
		//om.deleteObject(SwMauMom::CpTypeT(cpId - 1000));

		if (m_mauType[instance] == MAUS)
		{
			CpMauM cpMau(instance + 1);
			TRACE(("update(), disable MAUS"));
			cpMau.disable();

		}
		m_mauType[instance] = -1;
		m_cpId[instance] = -1;

	}
	while (false);

	TRACE(("CpTableObserver::updateDelete() return"));
}

void CpTableObserver::update(const ACS_CS_API_CpTableChange& observee)
{
	//TRACE(("CpTableObserver::update()"));
	TRACE(("CpTableObserver::update() for cpId <%d> op type <%d>", observee.cpData->cpId, observee.cpData->operationType));

	if (observee.cpData->operationType == ACS_CS_API_TableChangeOperation::Add)
	{

		updateAdd(observee);

#if 0


		TRACE(("CpTableObserver::update() Add operation"));
		TRACE(("CpTableChange.dataSize <%d>", observee.dataSize));
		ACS_CS_API_CpTableData *cpData = observee.cpData;
		TRACE(("cpData size <%d>", cpData->dataSize));
		TRACE(("cpData cpId <%d>", cpData->cpId));

		int cpId = cpData->cpId;
		int instance = cpId - 1001;

		if (cpId == m_cpId[instance])
		{
			TRACE(("No change in Add operation for cpid <%d>, m_cpId[%d] <%d>", cpId, instance, m_cpId[instance]));
		}
		else
		{
			TRACE(("Adding cpId <%d>", cpId));
			m_mauType[instance] = -1;
			m_cpId[instance] = cpId;

			// at adding
			// check mau type; if maus the create object
		}

#endif

		return;
	}

	if (observee.cpData->operationType == ACS_CS_API_TableChangeOperation::Delete)
	{
		updateDelete(observee);

#if 0

		TRACE(("CpTableObserver::update() Delete operation"));
		TRACE(("CpTableChange.dataSize <%d>", observee.dataSize));
		ACS_CS_API_CpTableData *cpData = observee.cpData;
		TRACE(("cpData size <%d>", cpData->dataSize));
		TRACE(("cpData cpId <%d>", cpData->cpId));

		int cpId = cpData->cpId;
		int instance = cpId - 1001;

		if (cpId == m_cpId[instance])
		{
			TRACE(("Delete operation for cpid <%d>, m_cpId[%d] <%d>", cpId, instance, m_cpId[instance]));
			m_mauType[instance] = -1;
			m_cpId[instance] = -1;
		}
		else
		{
			TRACE(("Delete cpId <%d>, no operation", cpId));
		}

		int adminState[2];
		SwMauOM om;
		om.readAdminState(adminState);

		CpMauM cpMau(instance + 1);
		// Disable MAUS and delete SwMauM object
		TRACE(("update(), disable MAUS"));
		cpMau.disable();

#endif

		return;
	}

	if (observee.cpData->operationType != ACS_CS_API_TableChangeOperation::Change)
	{
		// Nothing to be done
		TRACE(("NonOp since not Change Op"));
		return;
	}

	ACS_CS_API_CpTableData *data = observee.cpData;
	TRACE(("CpTableObserver::update(), observee.dataSize <%d>", observee.dataSize));
	TRACE(("CpTableObserver::update(), data->operationType <%d>", data->operationType));

	int cpId = data->cpId;
	int index = cpId - 1001;
	TRACE(("CpTableObserver::update(),cpId <%d>, index <%d>", cpId, index));

	if (index != 0 && index != 1)
	{
		TRACE(("CpTableObserver::update() out of scope cpId <%d>", cpId));
		return;
	}

	int mauType = data->mauType;
	if (mauType == m_mauType[index])
	{
		// No change
		// Should it trace or ...
		TRACE(("CpTableObserver::update() wihtout changes; current mauType <%d>, new mauType <%d>", m_mauType[index], m_mauType[index]));
		return;
	}

	handlePxe(cpId, mauType);

	// CS notifies mauType changes. The changes is acted on when CpMauM.administrativeState == Enabled.
	// If administrativeState is Disabled, it is not possible to turns MAUS1 or MAUS2 on or off
	int adminState[2];
	SwMauOM om;
	om.readAdminState(adminState);
	TRACE(("adminiState[0] <%d> adminState[1] <%d>", adminState[0], adminState[1]));

	TRACE(("****** CpTableObserver::update(), mauType change ****"));
	TRACE(("Current mauType <%d>, changed mauType <%d>", m_mauType[index], mauType));
	TRACE(("******"));
	m_mauType[index] = mauType;
    bool opResult = true;

    // Changing from MAUB to MAUS
    // Create SwMau=1 object
	if (mauType == MAUS)
	{
		TRACE(("update(), enable MAUS with index <%d>", index));

		om.createObject(SwMauMom::CpTypeT(cpId - 1000));

		if (index == 0)
		{
			m_change[0].set();
		}
		else
		{
			m_change[2].set();
		}
	}
	else if (mauType == MAUB)
	{
		TRACE(("update(), attempt disable MAUS"));

		CpMauM cpMau(index + 1);
		// Only operate when administrativeState == Enable

		if (adminState[index] == SwMauMom::UNLOCKED)
		{
			// Disable MAUS and delete SwMauM object
			TRACE(("update(), disable MAUS when UNLOCKED"));
			opResult = cpMau.disable();
		}
		else
		{
			TRACE(("update(), skips disable MAUS when LOCKED"));
		}

		// Delete the object
		TRACE(("update(), delete object for cpId <%d> cpType <%d>", cpId, cpId - 1000));
		om.deleteObject(SwMauMom::CpTypeT(cpId - 1000));

		// Should always update mauType change table
		if (index == 0)
		{
			m_change[1].set();
		}
		else
		{
			m_change[3].set();
		}

	}
	TRACE(("update(), result of changing <%d>", opResult));
}
