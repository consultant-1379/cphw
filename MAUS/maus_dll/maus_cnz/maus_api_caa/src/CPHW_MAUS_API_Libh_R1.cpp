/*
 * CPHW_MAUS_API_Libh_R1.cpp
 *
 *  Created on: May 4, 2015
 *      Author: xdtthng
 */

#include "CPHW_MAUS_API_Libh_R1.h"
#include "CPHW_MAUS_API_Libh_Impl.h"

CPHW_MAUS_API_Libh_R1::CPHW_MAUS_API_Libh_R1(int mauIdCode):
		m_impl(new CPHW_MAUS_API_Libh_Impl((mauIdCode < 1 || mauIdCode > 2)? 1 : mauIdCode))
{

}

CPHW_MAUS_API_Libh_R1::~CPHW_MAUS_API_Libh_R1()
{

}

CPHW_MAUS_API_Libh_R1::Result CPHW_MAUS_API_Libh_R1::loadLib(int side)
{
	return m_impl->loadLib(side);
}

CPHW_MAUS_API_Libh_R1::Result CPHW_MAUS_API_Libh_R1::unloadLib(int side)
{
	return m_impl->unloadLib(side);
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_R1::getOperationalState(int side, int& opState)
{
	return m_impl->getOperationalState(side, opState);
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_R1::performChecksum(int side, int config)
{
	return m_impl->performChecksum(side, config);
}

CPHW_MAUS_API_Libh::Result CPHW_MAUS_API_Libh_R1::installMibs(int side, int config)
{
	return m_impl->installMibs(side, config);
}
