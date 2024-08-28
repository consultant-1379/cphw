/*
 * CptSwMauMsg.cpp
 *
 *  Created on: May 9, 2016
 *      Author: xdtthng
 */


#include <iostream>
#include "CptSwMauMsg.h"

using namespace std;

const CptSwMauMsg::PrimitiveT CptSwMauMsg::GetPtcoiReqMsg::s_primitive = CptSwMauMsg::GetPtcoiReq;
const CptSwMauMsg::PrimitiveT CptSwMauMsg::GetPtcoiRspMsg::s_primitive = CptSwMauMsg::GetPtcoiRsp;

ostream& operator<<(ostream& os, const CptSwMauMsg::Version& v)
{
	return os << dec << "<" << v.major() << "><" << v.minor() << ">";
}

ostream& operator<<(ostream& os, const CptSwMauMsg::MsgHeader& msg)
{
	os << hex << "<"<< dec
			<< msg.version() << "<"
			<< msg.primitive() << "><"
			<< msg.seqNo()  << "><"
			<< msg.reserved()  << ">>";
	return os;
}

ostream& operator<<(std::ostream& os, const CptSwMauMsg::GetPtcoiReqMsg& msg)
{
	os << "GetPtcoiReqMsg "
			<< static_cast<const CptSwMauMsg::MsgHeader&>(msg)
			<< "<" << msg.serviceIdCode() << ">";

	return os;
}

ostream& operator<<(std::ostream& os, const CptSwMauMsg::GetPtcoiRspMsg& msg)
{
	os << "GetPtcoiRspMsg "
			<< static_cast<const CptSwMauMsg::MsgHeader&>(msg)
			<< "<" << msg.error()
			<< "><" << msg.ptcoiStatus() << ">";
	return os;
}
