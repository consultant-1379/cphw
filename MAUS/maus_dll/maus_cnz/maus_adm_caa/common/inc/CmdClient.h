/*
 * CmdClient.h
 *
 *  Created on: May 17, 2015
 *      Author: xdtthng
 */

#ifndef CMDCLIENT_H_
#define CMDCLIENT_H_

#include "boost/thread/mutex.hpp"
#include "GMClient.h"
#include "CmdErrMsg.h"

class CmdClient
{
public:

    struct QuorumInfo
    {
       int32_t connView;    // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-A, bit 3 CP-B
       int32_t quorumView;  // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-A, bit 3 CP-B
       int32_t activeMau;   // 0 = AP-A, 1 = AP-B, 2 = CP-A, 3 = CP-B
       int32_t activeMask;  // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-SB, bit 3 CP-EX
    };

	// Ctor and Dtor.
	CmdClient(int instance);
	~CmdClient();

	Cmd_Error_Code getQuorumInfo(QuorumInfo&);
	Cmd_Error_Code setActiveMask(int32_t = 0);

private:

	int			m_instance;		// talking to CP1MAU or CP2MAU
	int			m_thisSide;
	int			m_otherSide;
	GMClient	m_gmActive;
	GMClient	m_gmPassive;
	GMClient*	m_gmClient[2];
	int			m_conn[2];

	static const char*	s_gmServerDomain;
	static const char*  s_gmServerName[3];

    static boost::mutex	s_cs;

};

#endif /* CMDCLIENT_H_ */
