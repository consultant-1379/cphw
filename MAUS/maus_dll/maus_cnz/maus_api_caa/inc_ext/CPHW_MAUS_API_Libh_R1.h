/*
 * CPHW_MAUS_API_Libh_R1.h
 *
 *  Created on: May 4, 2015
 *      Author: xdtthng
 */

#ifndef CPHW_MAUS_API_LIBH_R1_H_
#define CPHW_MAUS_API_LIBH_R1_H_

#include <boost/shared_ptr.hpp>
#include <string>

class CPHW_MAUS_API_Libh_Impl;

class CPHW_MAUS_API_Libh_R1
{
public:

	// Result
	// ------
	// Please note that this is under development. it is expected to have more codes added
	//
	enum Result
	{
		RC_SUCCESS 						= 0,	// Request was executed successfully
		RC_NOCONTACT					= 1, 	// No contact to all MAUS instances on AP-A and AP-B
		RC_FILE_NOT_FOUND				= 2,	// The requested file does not exist
		RC_FAILED_TO_LOAD_LIB			= 3,	// The requested lib failed to load
		RC_MASK_VALUE_OUT_OF_RANGE		= 4,	// The specified mask value is out of range
		RC_OPERATION_NOT_IMPLEMENTED	= 5,	// This is mainly used during development of the API
		RC_FAILED_TO_GET_OPER_STATE		= 6, 	// Failed to get operational state
		RC_FAILED_TO_UNLOAD_LIB			= 7,	// Failed to unload lib
		RC_LIB_ALREADY_UNLOADED			= 8,
		RC_LIB_ALREADY_LOADED			= 9,
		RC_LIB_ALREADY_LOADED_NODEA		= 10,
		RC_LIB_ALREADY_LOADED_NODEB		= 11,
		RC_LIB_ALREADY_UNLOADED_NODEA	= 12,
		RC_LIB_ALREADY_UNLOADED_NODEB	= 13,
		RC_FAILED_TO_UNLOAD_LIB_NODEA	= 14,
		RC_FAILED_TO_UNLOAD_LIB_NODEB	= 15,
		RC_FAILED_TO_LOAD_LIB_NODEA		= 16,
		RC_FAILED_TO_LOAD_LIB_NODEB		= 17,
		RC_CHECK_SUM_FAILED				= 18,	// Checksum calculation failed
		RC_CHECK_SUM_FAILED_NODEA		= 19,	// Checksum calculation failed for AP-A
		RC_CHECK_SUM_FAILED_NODEB		= 20,	// Checksum calculation failed for AP-B

		RC_INSTALL_MIBS_FAILED			= 21,	// Install Mibs failed
		RC_INSTALL_MIBS_FAILED_NODEA	= 22,	// Install Mibs failed for AP-A
		RC_INSTALL_MIBS_FAILED_NODEB	= 23,	// Install Mibs failed for AP-B

		RC_NOCONTACT_NODEA				= 24, 	// No contact to MAUS instances on AP-A
		RC_NOCONTACT_NODEB				= 25, 	// No contact to MAUS instances on AP-B

		RC_INTERNAL_ERROR 				= 255	// We need to work on this. Errors we don't know about
	};

	// Constructor
	//
	// mauIdCode = 1, for Nored MAUS1 on One CP Sysem or Multip[le CP System
	// mauIdCode = 2 for Nored MAUS2 on Multip[le CP System
	// if mauIdCode < 1 or mauIdCode > 2, mauIdcode will have the default value of 1

	CPHW_MAUS_API_Libh_R1(int mauIdCode);


	// Destructor
	//
	virtual ~CPHW_MAUS_API_Libh_R1();

	// Interface methods
	// -----------------

	// Load the current libs as edited in the config file; if load is successful, start running the
	// CPHW main loop. It is expected the config file is updated by cfeted prior to the API is used

	// Parameters
	// ----------
	// when side is not given, the library is loaded on both AP-A and AP-B
	// side = 0, load the library on AP-A; side = 1, load the library on AP-B

	Result loadLib(int side = 2);

	// The lib will be unloaded from memory on the specified sides

	// Parameters
	// ----------
	// when side is not given, the library is unloaded on the specified side of the AP node
	// side = 0, operation on AP-A; side = 1, operation on AP-B
	// side = 2, default value, operation on both sides AP-A and AP-B
	Result unloadLib(int side = 2);

	// Setting or clearing of the MAU Active Mask

	// Parameters
	// ----------
	// mask = 0 ... 15
	Result setMask(int mask = 0);

	// Reading the operational state of MAUS
	// side = 0, for AP-A; side = 1 for AP-B. Other values are not accepted
	//
	// Return values
	// opState = 0, libMauCore is loaded and executed;
	// opState = 1, libMauCore is unloaded
	Result getOperationalState(int side, int& opState);

	// Order a checksum calculation of file specified in maus[12].conf
	// config = 0, [new] section in maus*.conf
	//        = 1, [cur] section
	//        = 2, [bak] section
	Result performChecksum(int side, int config);

	// Unzip mibsCXC*.zip specified in maus[12].conf
	// config = 0, [new] section in maus*.conf
	//        = 1, [cur] section
	//        = 2, [bak] section
	//
	// Usually installMibs() follows performChecksum()
	Result installMibs(int side, int config);

private:

   boost::shared_ptr<CPHW_MAUS_API_Libh_Impl> m_impl;

};


#endif /* CPHW_MAUS_API_LIBH_R1_H_ */
