/*
 * MausModel.h
 *
 *  Created on: Jan 16, 2015
 *      Author: xdtthng
 */

#ifndef MAUSMODEL_H_
#define MAUSMODEL_H_

#include <string>
#include <map>

class MausModel
{
public:

	enum CpTypeT
	{
		Ncd = 0,
		Cp1,
		Cp2,
		CpInvalid
	};

	enum AttributeT
	{
		AttributeStart = 0,
		AdministrativeState,
		ChangeState,
		FexRequest
	};

	static const std::string	immImpl_OiName;
	static const std::string	immCln_CpMauM;

	//static const std::string	immCln_CpMau;
	static const std::string	immCln_DualSidedCp;

	//static const std::string	immAttr_cpMauId;
	static const std::string	immAttr_dualSidedCpId;

	static const std::string	immAttr_operationalState;
	static const std::string	immAttr_administrativeState;
	static const std::string 	immAttr_changeState;
	static const std::string 	immAttr_fexRequest;

	static const std::string&	getParentInstance();
	static const std::string&	getCpInstance(CpTypeT);
	static std::string			getCpDn(CpTypeT);
	static const std::string&	getCpName(CpTypeT);
	static CpTypeT				getCpType(const std::string& cpDn);
	static std::string			getFexIniFileName(CpTypeT);
	static std::string			getFexIniFileName(CpTypeT, std::string&);

	static std::map<CpTypeT, std::string>		s_cpInstances;
	static std::map<CpTypeT, std::string>		s_cpNames;
};

#endif // MAUSMODEL_H_
