/*
 * CpMauMsg.cpp
 *
 *  Created on: Jul 6, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include "CpMauMsg.h"

using namespace std;

const CpMauMsg::PrimitiveT CpMauMsg::GetFexReqMsg::s_primitive = CpMauMsg::GetFexRq;
const CpMauMsg::PrimitiveT CpMauMsg::GetFexRspMsg::s_primitive = CpMauMsg::GetFexRq_Rsp;
const CpMauMsg::PrimitiveT CpMauMsg::SetFexReqMsg::s_primitive = CpMauMsg::SetFexRq;
const CpMauMsg::PrimitiveT CpMauMsg::SetFexRspMsg::s_primitive = CpMauMsg::SetFexRq_Rsp;
const CpMauMsg::PrimitiveT CpMauMsg::GetAdminStateReqMsg::s_primitive = CpMauMsg::GetAdminState;
const CpMauMsg::PrimitiveT CpMauMsg::GetAdminStateRspMsg::s_primitive = CpMauMsg::GetAdminState_Rsp;

const CpMauMsg::PrimitiveT CpMauMsg::GetApgStateReqMsg::s_primitive = CpMauMsg::GetApgStateRq;
const CpMauMsg::PrimitiveT CpMauMsg::GetApgStateRspMsg::s_primitive = CpMauMsg::GetApgStateRq_Rsp;


ostream& operator<<(ostream& os, const CpMauMsg::Version& v)
{
	return os << dec << "<" << v.major() << "><" << v.minor() << ">";
}

ostream& operator<<(ostream& os, const CpMauMsg::MsgHeader& msg)
{
	os << hex << "<"<< dec
			<< msg.version() << "<"
			<< msg.primitive() << "><"
			<< msg.seqNo()  << "><"
			<< msg.reserved()  << ">>";
	return os;
}

ostream& operator<<(std::ostream& os, const CpMauMsg::GetFexReqMsg& msg)
{
	os << "GetFexReqMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.serviceIdCode() << ">";
	return os;
}

ostream& operator<<(std::ostream& os, const CpMauMsg::GetFexRspMsg& msg)
{
	os << "GetFexRspMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.error()
			<< "><" << msg.fexReqState() << ">";
	return os;
}

ostream& operator<<(std::ostream& os, const CpMauMsg::SetFexReqMsg& msg)
{
	os << "SetFexReqMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.serviceIdCode() << "><"
			<< msg.fexReqState()
			<< ">";
	return os;
}

ostream& operator<<(std::ostream& os, const CpMauMsg::SetFexRspMsg& msg)
{
	os << "SetFexRspMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.error() << ">";
	return os;
}

ostream& operator<<(std::ostream& os, const CpMauMsg::GetAdminStateReqMsg& msg)
{
	os << "GetAdminStateReqMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.serviceIdCode() << ">";
	return os;

}

ostream& operator<<(std::ostream& os, const CpMauMsg::GetAdminStateRspMsg& msg)
{
	os << "GetAdminStateRspMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.error()
			<< "><" << msg.adminState() << ">";
	return os;

}

ostream& operator<<(std::ostream& os, const CpMauMsg::GetApgStateReqMsg& msg)
{
	os << "GetApgStateReqMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			;
	return os;

}

ostream& operator<<(std::ostream& os, const CpMauMsg::GetApgStateRspMsg& msg)
{
	os << "GetApgStateRspMsg "
			<< static_cast<const CpMauMsg::MsgHeader&>(msg)
			<< "<" << msg.error()
			<< "><" << msg.apaState()
			<< "><" << msg.apbState()
			<< ">"
			;
	return os;

}
