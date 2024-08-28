/*
 * SwMauOM.h
 *
 *  Created on: Mar 28, 2016
 *      Author: xdtthng
 */

#ifndef SWMAUOM_H_
#define SWMAUOM_H_

#include <string>
#include "acs_apgcc_omhandler.h"
#include "SwMauMom.h"


class SwMauOM
{
public:

	SwMauOM();
	~SwMauOM();


	// Create SwMau for CP1 and CP2
	bool createObject(int* admState);

	// Create SwMau for a specific CpType, i.e for CP1 or CP2
	bool createObject(SwMauMom::CpTypeT);

	// Create SwMau for a parent instance
	bool createObject(const std::string& parent, bool& objExist, int& adminState);

	// Create SwMau for a specific CpType, i.e for CP1 or CP2
	bool deleteObject(SwMauMom::CpTypeT);

	// Read administrativeSate for a specified CpTypeT object
	bool readAdminState(int* state);

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

	// Create object
	// For initial testing
	bool createObject_old(SwMauMom::CpTypeT);

	// Create SwMau for a parent instance
	//bool createObject(const std::string& parent);


};


#endif /* SWMAUOM_H_ */
