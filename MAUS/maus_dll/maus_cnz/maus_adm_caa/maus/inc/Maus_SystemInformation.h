/*
 * Maus_MauTypeMonitor.h
 *
 *  Created on: Oct 16, 2014
 *      Author: xdtthng
 */

#ifndef MAUS_SYSTEM_INFORMATION_H_
#define MAUS_SYSTEM_INFORMATION_H_

#include <boost/thread/thread.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#include "ACS_CS_API.h"
#include "Maus_Information.h"

class Maus_Application;

class SystemInformation
{
public:
	// Only meant to be used by Maus_Application
	friend class Maus_Application;

	enum MauTypeT
	{
		MAUB = 1,
		MAUS = 2
	};

	typedef ACS_CS_API_CommonBasedArchitecture Acs_ArchT;
	typedef Acs_ArchT::ArchitectureValue ArchValueT;

	SystemInformation(int instance, int endEvent);
	~SystemInformation();
	void start();
	void run();

	//bool getMultiCpSystem()		{ return m_multiCpSystem; }
	//unsigned int getCpCount()	{ return m_cpCount; }
	//int getMauType()			{ return m_mauType; }
	//MAUBase::MauArg& getMauArg() { return m_mauArg;}
	//int	getSlotId()				{ return m_slotId; }

private:
	int				m_instance;
	int				m_cmwEndEvent;
	bool			m_multiCpSystem;
	unsigned int	m_cpCount;
	int				m_mauType;
	bool			m_classicCpSystem;
	int				m_infrastructure;
	int				m_sysNo;
	boost::thread	m_thread;
	int				m_slotId;
	std::string		m_ftmName;
	bool			m_initialised;
    int				m_adminState;
    int				m_fcState;
    bool			m_shutdown;
    int				m_apgState;

    // Should always be the last
	MausInformation	m_mausInformation;


	int init();

	static const char*	s_slotFileName;
	static const char*	s_interfaces[2];
	static const char* 	s_logRoots[3];

};

#endif // MAUS_SYSTEM_INFORMATION_H_
