/*
 * CptSwMau.h
 *
 *  Created on: May 9, 2016
 *      Author: xdtthng
 */

#ifndef CPTSWMAU_H_
#define CPTSWMAU_H_

#include "boost/thread/mutex.hpp"
#include "GMClient.h"

class CptSwMau
{
public:

	enum PtcoiStatusT
	{
		NoPtcoiSession = 0,
		PtcoiSessionExists = 1
	};

	CptSwMau(int instance);
	~CptSwMau() {};

	// Check if there is PTCOI session towards CP instance
	bool ptcoiExist();

private:
	int			m_instance;		// talking to CP1MAU or CP2MAU
	bool		m_conn;			// connect to Active side

	GMClient	m_client;

	static const char*	s_gmServerDomain;
	static const char*  s_gmServerName;

    static boost::mutex	s_cs;

    // Check if there is a DSD connection to Active node
	bool isConnected() { return m_conn; }
};

#endif /* CPTSWMAU_H_ */
