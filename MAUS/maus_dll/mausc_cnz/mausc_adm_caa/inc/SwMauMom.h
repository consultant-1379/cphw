/*
 * SwMauMom.h
 *
 *  Created on: Mar 27, 2016
 *      Author: xdtthng
 */

#ifndef SWMAUMOM_H_
#define SWMAUMOM_H_

#include <string>
#include <map>

class SwMauMom
{
public:

	enum AttributeT
	{
		AttributeStart = 0,
		AdministrativeState,
	};

	enum OperationT
	{
		LOCKED = 0,
		UNLOCKED = 1
	};

	enum CpTypeT
	{
		Ncd = 0,
		Cp1,
		Cp2,
		CpInvalid
	};

	static const std::string	immImpl_OiName;				// Object Implementer (OI) name
	static const std::string	immCln_EquipmentSwMau;		// Class name SwMau
	static const std::string	immCln_EquipmentDualSidedCp;

	static const std::string	immAttr_dualSidedCpId;
	static const std::string	immAttr_swMauId;
	static const std::string	s_swMauInstance;

	static const std::string	immAttr_operationalState;
	static const std::string	immAttr_administrativeState;

	//static const std::string&	getParentInstance();

	// The Cp Instance is one of the following
	// dualSidedCpId=CP1 or dualSidedCpId=CP2
	static const std::string&	getCpInstance(CpTypeT);

	//static std::string			getCpDn(CpTypeT);

	// Return cp1 or cp2 for the given CpTypeT
	static const std::string&	getCpName(CpTypeT);

	// Return CpType for the given cpName cp1 or cp2
	static CpTypeT				getCpType(const std::string& cpDn, int = 1);

	// get SWMau distinguish name
	bool getSwMauDn(std::vector<std::string>&);

	static std::map<CpTypeT, std::string>		s_cpInstances;
	static std::map<CpTypeT, std::string>		s_cpNames;


private:
};


#endif /* SWMAUMOM_H_ */
