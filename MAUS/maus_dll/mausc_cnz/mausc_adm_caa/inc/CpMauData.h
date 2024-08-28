/*
 * CpMauData.h
 *
 *  Created on: Apr 5, 2016
 *      Author: xdtthng
 */

#ifndef CPMAUDATA_H_
#define CPMAUDATA_H_

#include <string>
#include <map>

class CpMauData
{
public:

	enum CpTypeT
	{
		Ncd = 0,
		Cp1,
		Cp2,
		CpInvalid
	};

	// Read fexRequest for a specified CpTypeT
	static bool readFexRequest(int& fexreq, CpTypeT cpType);

	// Set fexRequest for a specified CpTypeT
	static bool setFexRequest(int fexreq, CpTypeT cpType);

	static const std::string&	getCpName(CpTypeT);
	static std::string			getFexIniFileName(CpTypeT);
	static std::string			getFexIniFileName(CpTypeT, std::string&);

private:

	static std::map<CpTypeT, std::string>		s_cpNames;

};

#endif /* CPMAUDATA_H_ */
