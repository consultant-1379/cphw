/*
 * CPHW_MAUS_API_Libh_Impl.h
 *
 *  Created on: May 5, 2015
 *      Author: xdtthng
 */

#ifndef CPHW_MAUS_API_LIBH_IMPL_H_
#define CPHW_MAUS_API_LIBH_IMPL_H_

#include "boost/thread/mutex.hpp"
#include "CPHW_MAUS_API_Libh.h"
#include "GMClient.h"


class CPHW_MAUS_API_Libh_Impl
{
public:

	enum OperationalStateT
	{
		NotAvailable = 0,	// DISABLED
		Available = 1		// ENABLED
	};

	enum SideT
	{
		SideA = 0,
		SideB = 1,
		BothSides =2
	};


	CPHW_MAUS_API_Libh_Impl(int);
	virtual ~CPHW_MAUS_API_Libh_Impl();

	CPHW_MAUS_API_Libh::Result loadLib(int side);
	CPHW_MAUS_API_Libh::Result unloadLib(int side);
	CPHW_MAUS_API_Libh::Result setMask(int mask = 0);
	CPHW_MAUS_API_Libh::Result getOperationalState(int side, int& opState);
	CPHW_MAUS_API_Libh::Result performChecksum(int side, int config);
	CPHW_MAUS_API_Libh::Result installMibs(int side, int config);

private:

	int			m_instance;		// talking to CP1MAU or CP2MAU
	bool		m_activeConn;	// connect to Active node
	bool		m_passiveConn;	// connect to Passive node

	GMClient	m_gmActive;
	GMClient	m_gmPassive;
	GMClient*	m_gmClient[2];
	bool		m_conn[2];

	static const char*	s_gmServerDomain;
	static const char*  s_gmServerName[3];
    static boost::mutex	s_cs;

    // Check if there is a DSD connection to both Active node and Passive node
	bool isConnected(int side ) { return side == 2? (m_activeConn && m_passiveConn): m_conn[side]; }

	CPHW_MAUS_API_Libh::Result unload(int side);
	CPHW_MAUS_API_Libh::Result load(int side);
	CPHW_MAUS_API_Libh::Result performChecksum(int side);

};


#endif /* CPHW_MAUS_API_LIBH_IMPL_H_ */
