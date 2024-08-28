/*
 * Maus_MauTypeMonitor.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: xdtthng
 */

//#include "halPlatformDefinitions.hxx"

#include <boost/bind.hpp>
#include <sys/poll.h>
#include <fstream>
#include <sstream>

#include "acs_prc_api.h"

#include "Maus_SystemInformation.h"
#include "maus_trace.h"
#include "MAUFunx.h"
#include "Maus_AehEvent.h"
#include "AtomicFlag.h"

const char*	SystemInformation::s_slotFileName = "/etc/opensaf/slot_id";
const char* SystemInformation::s_interfaces[2] = { "eth3", "eth4" };
const char* SystemInformation::s_logRoots[3] = {"APZ/data/mau/", "APZ/logs/cp1/mau/", "APZ/logs/cp2/mau/" };

using namespace std;
extern BoolFlagT s_applicationAborting;

#define Every_Nsec	if (loopCount % 5 == 0)

SystemInformation::SystemInformation(int instance, int endEvent):
		m_instance(instance),
		m_cmwEndEvent(endEvent),
		m_multiCpSystem(false),
		m_cpCount(0),
		m_mauType(-1),
		m_classicCpSystem(false),
		m_infrastructure(-1),
		m_sysNo(-1),
		m_thread(),
		m_slotId(-1),
		m_ftmName(),
		m_initialised(false),
		m_adminState(-1),
		m_fcState(-1),
		m_shutdown(false),
		m_apgState(-1),
		m_mausInformation()
{
	TRACE_DEBUG(("SystemInformation::SystemInformation()"));
	//memset(&m_mauArg, 0, sizeof(m_mauArg));
}

SystemInformation::~SystemInformation()
{
	TRACE_DEBUG(("SystemInformation::~SystemInformation()"));

	if (m_thread.joinable())
	{
		TRACE_DEBUG(("SystemInformation::~SystemInformation() joining main thread"));
		m_thread.join();
	}
}

void SystemInformation::start()
{
	TRACE_DEBUG(("SystemInformation::start()"));
	m_thread = boost::thread(boost::bind(&SystemInformation::run, this));
	TRACE_DEBUG(("SystemInformation::start() returns"));
}

void SystemInformation::run()
{
	TRACE_DEBUG(("SystemInformation::run()"));

	// Try to get information from CS
	// This will block until CS is contacted

	struct pollfd fds[1];
	fds[0].fd = m_cmwEndEvent;
	fds[0].events = POLLIN;
	int pollResult = 0;

	bool stop = false;
	bool multCPSys = false;
	int classicCp = -1;
	unsigned int cpCount = 0;
	int mauType = 0;
	//ArchValueT infra;
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue infra;

	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NS::Result_Failure;
	ACS_PRC_API prcApi;
	int timeInMilliseconds = 100;
	int loopCount = 0;
	bool classicCpRead = false;
	bool nodeArchRead = false;
	bool isMultiCpRead = false;

	while (!stop)
	{
		// lopping at 200 ms interval
		pollResult = poll(fds, 1, timeInMilliseconds);
		// after the fist timeout, set elapse time to 100ms
		timeInMilliseconds = 1000;
		++loopCount;

		if (pollResult == -1)
		{
			// Just try again for now
			continue;
		}
		else if (pollResult)
		{
			//shut down event
			if (fds[0].revents & POLLIN )
			{
				fds[0].revents = 0;
				stop = true;
				m_shutdown = true;
				TRACE_DEBUG(("SystemInformation::run() shutdown event"));
				continue;
			}
		}

		if(classicCpRead != true)
		{
			m_classicCpSystem = false;
			classicCp = -1;
			classicCp = MAUFunx::getClassicCp();
			if (classicCp == -1)
			{
				Every_Nsec
				TRACE(("MAUFunx::getClassicCp() failed"));
				continue;
			}

			TRACE_DEBUG(("MAUFunx::getClassicCp() returned classicCp = <%d>", classicCp));

			classicCpRead = true;
			m_classicCpSystem = classicCp == 1;

			if (m_classicCpSystem)
			{
				TRACE(("MAUFunx::getClassicCp() ok, return. Function not supported"));
				break;
			}
		}

		if (s_applicationAborting)
		{
			stop = true;
			m_shutdown = true;
			continue;
		}

		if(isMultiCpRead != true)
		{

			m_multiCpSystem = multCPSys = false;
			res = ACS_CS_API_NetworkElement::isMultipleCPSystem(multCPSys);
			if (res != ACS_CS_API_NS::Result_Success)
			{
				Every_Nsec
				TRACE(("NetworkElement::isMultipleCPSystem() failed"));
				continue;
			}
			m_multiCpSystem = multCPSys;
			isMultiCpRead = true;
			TRACE_DEBUG(("NetworkElement::isMultipleCPSystem() returned multCPSys = <%d>",
					multCPSys));
		}

		if (s_applicationAborting)
		{
			stop = true;
			m_shutdown = true;
			continue;
		}


		if(nodeArchRead !=true)
		{
			m_infrastructure = -1;
			res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);
			if (ACS_CS_API_NS::Result_Success != res)
			{
				Every_Nsec
				TRACE(("NetworkElement::getNodeArchitecture() failed"));
				continue;
			}
			m_infrastructure = infra;
			nodeArchRead = true;
			TRACE_DEBUG(("NetworkElement::getNodeArchitecture() returned infra = <%d>",
					infra));
		}


		// TR
		// Should remove the following
		// --------------------------------------------------------
		//if (m_instance == 2 && (!m_multiCpSystem || m_cpCount != 2))
		//	break;

		if (s_applicationAborting)
		{
			stop = true;
			m_shutdown = true;
			continue;
		}

		mauType = 0;
		if (Acs_ArchT::VIRTUALIZED == m_infrastructure)
		{
			m_mauType = MAUS;
		}
		else
		{
			mauType = MAUFunx::getMauType(m_instance);
			if (mauType <= 0)
			{
				Every_Nsec
				TRACE(("MAUFunx::getMauType() failed"));
				continue;
			}

			m_mauType = mauType;
			TRACE_DEBUG(("MAUFunx::getMauType() returned  mauType = <%d>", mauType));
		}


		// Remove later
		//m_fcState = MAUFunx::getCpMauMFCState(m_instance);
		m_fcState = -1;

		m_adminState = -1;
		if (m_mauType == MAUS)
		{
			m_adminState = MAUFunx::getSwMauAdminState(m_instance);

			if (m_adminState < 0)
			{
				Every_Nsec
				TRACE(("MAUFunx::getSwMauAdminState() failed"));
				continue;
			}
		}
		TRACE_DEBUG(("MAUFunx::getSwMauAdminState() returned m_adminState = <%d>",
				m_adminState));

		m_cpCount = cpCount = 0;
		res = ACS_CS_API_NetworkElement::getDoubleSidedCPCount(cpCount);
		if (ACS_CS_API_NS::Result_Success != res)
		{
			Every_Nsec
			TRACE(("NetworkElement::getDoubleSidedCPCount() failed"));
			continue;
		}

		m_cpCount = cpCount;
		TRACE_DEBUG(("NetworkElement::getDoubleSidedCPCount() returned cpCount =  <%d>",
				cpCount));

		m_apgState = prcApi.askForNodeState();
		if (m_apgState < 0)
		{
			Every_Nsec
			TRACE(("prcApi.askForNodeState() failed"));
			continue;
		}

		if (!init())
			continue;

		stop = true;
		m_initialised = true;
		TRACE_DEBUG(("SystemInformation::run() CS OK, multiCpSystem <%d>, cpCount <%d>"
				" mauType <%d>", m_multiCpSystem, m_cpCount, m_mauType));
		TRACE_DEBUG(("SystemInformation::run() infrastructure <%d>, adminState <%d>",
				m_infrastructure, m_adminState));
		TRACE_DEBUG(("SystemInformation::run() m_fcState <%d>", m_fcState));
	}
	TRACE_DEBUG(("SystemInformation::run() returns OK"));
}

int SystemInformation::init()
{
	TRACE_DEBUG(("SystemInformation::init()"));

	if (m_slotId == -1)
	{
		ifstream slotFile(s_slotFileName);

		if (slotFile.good()) {
			int slot = -1;
			slotFile >> slot;
			m_slotId = slotFile.good() ? slot : -1;
			slotFile.close();
			if (m_slotId == -1) {
				TRACE_DEBUG(("SystemInformation::initMauArg(); not yet read slotId"));
				EventReporter::instance().report(__LINE__,
						"Fail to read slot Id",
						"SystemInformation::initMauArg()");

			}
			else {
				TRACE_DEBUG(("SystemInformation::initMauArg(), m_slotId <%d>", m_slotId));
			}
		}
	}
	else {
		TRACE_DEBUG(("SystemInformation::initMauArg(), m_slotId <%d>", m_slotId));
	}

	memset(&m_mausInformation, 0, sizeof(m_mausInformation));	// Might not need this line
	m_mausInformation.side = m_slotId == 1 ? 0 :1;

	m_ftmName = "MAU_";
	m_ftmName += dynamic_cast<ostringstream &>(ostringstream() << m_instance).str();
	m_mausInformation.viewName = const_cast<char*>(m_ftmName.c_str());
	m_mausInformation.interface[0] = const_cast<char*>(s_interfaces[0]);
	m_mausInformation.interface[1] = const_cast<char*>(s_interfaces[1]);
	m_mausInformation.logRoot = const_cast<char*>(!m_multiCpSystem ? s_logRoots[0] : s_logRoots[m_instance]);
	m_mausInformation.instance = m_instance;
	m_mausInformation.shlf = m_infrastructure;
	m_mausInformation.fcState = m_fcState;
	m_mausInformation.apgState = m_apgState;

	// Fetching system number, CS should be up at this stage
	m_sysNo = MAUFunx::getSysNo(m_instance, m_slotId);
	m_mausInformation.sysNo = m_sysNo;

	TRACE_DEBUG(("m_mausInformation, log root <%s>", m_mausInformation.logRoot));
	TRACE_DEBUG(("m_mausInformation, view name <%s>", m_mausInformation.viewName));
	TRACE_DEBUG(("m_mausInformation, instance <%d>", m_mausInformation.instance));
	TRACE_DEBUG(("m_mausInformation, shlf <%d>", m_mausInformation.shlf));
	TRACE_DEBUG(("m_mausInformation, side <%d>", m_mausInformation.side));
	TRACE_DEBUG(("m_mausInformation, sysNo <%d>", m_mausInformation.sysNo));
	TRACE_DEBUG(("m_mausInformation, fcState <%d>", m_mausInformation.fcState));
	TRACE_DEBUG(("m_mausInformation, apgState <%d>", m_mausInformation.apgState));

	TRACE_DEBUG(("SystemInformation::init() returns"));
	return 1;

}

