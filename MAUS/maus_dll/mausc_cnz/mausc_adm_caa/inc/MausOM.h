/*
 * MausOM.h
 *
 *  Created on: Jan 16, 2015
 *      Author: xdtthng
 */
/*
NAME
   File_name: MausOM.h

COPYRIGHT Ericsson AB, Sweden 2015. All rights reserved.

   The Copyright to the computer program(s) herein is the property of Ericsson AB, Sweden.
   The program(s) may be used and/or copied only with the written permission from
   Ericsson AB or in accordance with the terms and conditions
   stipulated in the agreement/contract under which the program(s) have been
   supplied.

DESCRIPTION

   This class provide the wrapper for interfacing to APG CC OMHandler

DOCUMENT NO
   190 89-CAA 109

AUTHOR
   2015-01-16 by DEK/XDTTHNG Thanh Nguyen

SEE ALSO


Revision history
----------------
2014-06-05 Thanh Nguyen Created

*/

#ifndef MAUSOM_H_
#define MAUSOM_H_

#include <string>
#include "acs_apgcc_omhandler.h"
#include "MausModel.h"

class MausOM
{
public:
	MausOM();
	virtual ~MausOM();

	// Create all objects based on system information
	bool createAndSyncObject();

	// Create object specified by CpTypeT
	bool createObject(MausModel::CpTypeT);

	// Read administrativeSate for a specified CpTypeT object
	bool readAdminState(int& adminState, MausModel::CpTypeT cpType);

	// Read fexRequest for a specified CpTypeT object
	bool readFexRequest(int& fexreq, MausModel::CpTypeT cpType);

	// Set fexRequest for a specified CpTypeT object
	bool setFexRequest(int fexreq, MausModel::CpTypeT cpType);

	// Read administrativeSate for a specified CpTypeT object
	bool readChangeState(int& changeState, MausModel::CpTypeT cpType);

	// Read attribute
	bool readAttribute(int& attribute, const std::string& name, MausModel::CpTypeT cpType);

	// Modify attribute
	bool setAttribute(int attribute, const std::string& name, MausModel::CpTypeT cpType);

	// Modify string value of an attribute
	//bool modifyAttrs(const std::string& objectName, const std::string& attrName,
	//                 const std::string& value, const std::string& transName);

	// Modify int value of an attribute
    //bool modifyAttrs(const std::string& objectName, const std::string& attrName,
    //                    int value, const std::string& transName, bool unsignedIntUsed = false);

private:

    // Initialized indication
    bool m_initialized;

    // OM Handler
    OmHandler m_omHandler;

    // Error code & string
    int m_lastErrorCode;
    string m_lastErrorText;

    // Initialize OmHandler object
	bool init();
};


#endif // MAUSOM_H_ //
