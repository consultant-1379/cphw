/*
 * CpMauM.h
 *
 *  Created on: Feb 23, 2015
 *      Author: xdtthng
 */

/*
NAME
    File_name: CpMauM.h

Ericsson AB

    COPYRIGHT Ericsson AB, Sweden 2000. All rights reserved.

    The Copyright to the computer program(s) herein is the property of
    Ericsson AB, Sweden. The program(s) may be used and/or
    copied only with the written permission from Ericsson AB
    or in accordance with the terms and conditions stipulated in the
    agreement/contract under which the program(s) have been supplied.

DESCRIPTION

	Object Implementer MausOI uses this class to carry out its actions towards
	MAUS1 and MAUS2.

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

#ifndef CPMAUM_H_
#define CPMAUM_H_

#include "boost/thread/mutex.hpp"
#include "GMClient.h"

class CpMauM
{
public:

	enum OperationalStateT
	{
		NotAvailable = 0,	// DISABLE
		Available = 1		// ENABLE

	};

	// Ctor and Dtor.
	CpMauM(int instance);
	~CpMauM() {};

	// Read operationalState attribute of CpMauM MO
	OperationalStateT getOperationalState();

	// Disable MAUS1 or MAUS2, i.e. remove the endpoints from connectivity view
	bool disable();

	// Enable MAUS1 or MAUS2, i.e. start the endpoint to join connectivity view
	bool enable();

	// Set function change state
	//bool setFCState(int state);

private:
	int			m_instance;		// talking to CP1MAU or CP2MAU
	bool		m_activeConn;	// connect to both Active
	bool		m_passiveConn;	// and Passive nodes

	GMClient	m_gmActive;
	GMClient	m_gmPassive;

	static const char*	s_gmServerDomain;
	static const char*  s_gmServerName[3];

    static boost::mutex	s_cs;

    // Check if there is a DSD connection to both Active node and Passive node
	bool isConnected() { return m_activeConn && m_passiveConn; }
	bool isAnyConnected() { return m_activeConn || m_passiveConn; }

	CpMauM(const CpMauM&);
	CpMauM& operator=(const CpMauM&);

};

#endif // CPMAUM_H_
