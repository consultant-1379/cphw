/*
 * MausCpTableObserver.h
 *
 *  Created on: Mar 5, 2015
 *      Author: xdtthng
 */

/*
NAME
    File_name: MausCpTableObserver.h

Ericsson AB

    COPYRIGHT Ericsson AB, Sweden 2000. All rights reserved.

    The Copyright to the computer program(s) herein is the property of
    Ericsson AB, Sweden. The program(s) may be used and/or
    copied only with the written permission from Ericsson AB
    or in accordance with the terms and conditions stipulated in the
    agreement/contract under which the program(s) have been supplied.

DESCRIPTION

	The extension of ACS_CS_API_CpTableObserver to check for the changes of mauType
	in Native environment.

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

#ifndef MAUSCCPTABLEOBSERVER_H_
#define MAUSCCPTABLEOBSERVER_H_

#include <ACS_CS_API.h>
#include "maus_event.h"
#include "MausSysInfo.h"

#include <string>

typedef Maus_Event MauTypeChangeT[4];

class CpTableObserver : public ACS_CS_API_CpTableObserver
{
public:
	enum {
		MAUB = 1,
		MAUS = 2
	};

	CpTableObserver(int, int*, SysInfo::EventT&);
	~CpTableObserver();

	// Setup subscription to ACS CS CpTableChange
	void init();

	// The callback function handle changing of CP table
	void update(const ACS_CS_API_CpTableChange& observee);
	void updateAdd(const ACS_CS_API_CpTableChange& observee);
	void updateDelete(const ACS_CS_API_CpTableChange& observee);

private:
	bool						m_initialized;
	ACS_CS_API_SubscriptionMgr *m_subMgrInstance;
	int							m_architecture;
	SysInfo::EventT&			m_change;

	// Should leave this as the last data member
	int		m_mauType[2];
	int		m_cpId[2];

	// The following can be in a separate module for common functions manipulating files
	static const std::string		s_dataPath;
	static const std::string		s_ncdSide[2];
	static const std::string		s_mcpSide[2];
	static const std::string		s_cpIDs[2];
	static const std::string		s_default[3];
	static const std::string		s_pxelinux;

	bool isMultipleCP();
	bool fileExists(const std::string& name);
	bool deleteFile(const std::string& name);
	bool copyFile(const std::string& src, const std::string& dest);
	void handlePxe(int cpId, int mauType);
	void handlePxe(const std::string& dir, int mauType);
};

#endif // MAUSCCPTABLEOBSERVER_H_
