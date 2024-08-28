/*
 * CpMauData.cpp
 *
 *  Created on: Apr 5, 2016
 *      Author: xdtthng
 */

#include "boost/assign/list_of.hpp"
#include "boost/algorithm/string.hpp"

#include "maus_trace.h"
#include "CpMauData.h"
#include "MAUFunx.h"
#include "FileFunx.h"
#include "ParseIni.h"

#define Every_Ntimes	if (++s_traceCount%150 == 0)

using namespace std;

static unsigned int s_traceCount = 0;

map<CpMauData::CpTypeT, std::string> CpMauData::s_cpNames = boost::assign::map_list_of
    (CpMauData::Ncd, "cp1")
    (CpMauData::Cp1, "cp1")
    (CpMauData::Cp2, "cp2")
    (CpMauData::CpInvalid, "");


bool CpMauData::readFexRequest(int& fexreq, CpTypeT cpType)
{
	Every_Ntimes
	TRACE(("CpMauData::readFexRequest() cpType <%d>", cpType));

	string iniFile = CpMauData::getFexIniFileName(cpType);
	fexreq = ParseIni::getPrivateProfile("fex", "fexRequest", 0, iniFile.c_str());

	Every_Ntimes
    TRACE(("fexRequest from ini file  <%d>", fexreq));
	return true;
}

bool CpMauData::setFexRequest(int fexreq, CpTypeT cpType)
{
	//Every_Ntimes
	TRACE(("CpMauData::readFexRequest() fexreq<%d> cpType <%d>", fexreq, cpType));

	string dataPath;
	string fileName = CpMauData::getFexIniFileName(cpType, dataPath);
	if (FileFunx::createDirectories(dataPath))
	{
		ParseIni::writePrivateProfile("fex", "fexRequest", fexreq, fileName.c_str());
	}
	else
	{
		//Every_Ntimes
		TRACE(("CpMauData::readFexRequest() failed to write to ini file"));
	}

	return true;
}

const string& CpMauData::getCpName(CpTypeT type)
{
	return s_cpNames[type];
}

string CpMauData::getFexIniFileName(CpTypeT type)
{
	string dataPath = MAUFunx::getCphwDataPath();
	dataPath += CpMauData::getCpName(type);
	dataPath += '/';
	dataPath += MAUFunx::getCphwFexIniFileName();
	return dataPath;
}

string CpMauData::getFexIniFileName(CpTypeT type, string& path)
{
	string dataPath = MAUFunx::getCphwDataPath();
	dataPath += CpMauData::getCpName(type);
	dataPath += '/';
	path = dataPath;
	dataPath += MAUFunx::getCphwFexIniFileName();
	return dataPath;
}
