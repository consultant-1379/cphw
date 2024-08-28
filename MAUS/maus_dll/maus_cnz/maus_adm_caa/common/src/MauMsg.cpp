/*
 * MauMsg.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: xdtthng
 */


#include <iostream>
#include "MauMsg.h"

using namespace std;

const MauMsg::PrimitiveT MauMsg::GetOperationStateMsg::s_primitive = MauMsg::GetOperationState;
const MauMsg::PrimitiveT MauMsg::GetOperationStateRspMsg::s_primitive = MauMsg::GetOperatoinState_Rsp;
const MauMsg::PrimitiveT MauMsg::EnableMsg::s_primitive = MauMsg::Enable;
const MauMsg::PrimitiveT MauMsg::EnableRspMsg::s_primitive = MauMsg::Enable_Rsp;
const MauMsg::PrimitiveT MauMsg::DisableMsg::s_primitive = MauMsg::Disable;
const MauMsg::PrimitiveT MauMsg::DisableRspMsg::s_primitive = MauMsg::Disable_Rsp;
const MauMsg::PrimitiveT MauMsg::GetQuorumInfomationMsg::s_primitive = MauMsg::GetQuorumInformation;
const MauMsg::PrimitiveT MauMsg::GetQuorumInfomationRspMsg::s_primitive = MauMsg::GetQuorumInformation_Rsp;

const MauMsg::PrimitiveT MauMsg::SetActiveMaskMsg::s_primitive = MauMsg::SetActiveMask;
const MauMsg::PrimitiveT MauMsg::SetActiveMasRspMsg::s_primitive = MauMsg::SetActiveMask_Rsp;

const MauMsg::PrimitiveT MauMsg::PerformChecksumMsg::s_primitive = MauMsg::PerformChecksum;
const MauMsg::PrimitiveT MauMsg::PerformChecksumRspMsg::s_primitive = MauMsg::PerformChecksum_Rsp;

const MauMsg::PrimitiveT MauMsg::SetFCStateMsg::s_primitive = MauMsg::SetFCState;
const MauMsg::PrimitiveT MauMsg::SetFCStateRspMsg::s_primitive = MauMsg::SetFCstate_Rsp;

const MauMsg::PrimitiveT MauMsg::InstallMibsMsg::s_primitive = MauMsg::InstallMibs;
const MauMsg::PrimitiveT MauMsg::InstallMibsRspMsg::s_primitive = MauMsg::InstallMibs_Rsp;


MauMsg::InstallMibsMsg::InstallMibsMsg(int opid, int config, int maj, int min):
	MsgHeader(s_primitive, maj, min), m_opInd(opid), m_confSection(config)
{
	m_opInd = (opid < 0 || opid > 1)? 0: opid;
	m_confSection = (opid == 0)? 0: (config < 0 || config > 2)? 1: config;
};

MauMsg::PerformChecksumMsg::PerformChecksumMsg(int csid, int config, int maj, int min):
	MsgHeader(s_primitive, maj, min), m_checksumInd(csid), m_confSection(config)
{
	m_checksumInd = (csid < 0 || csid > 1)? 0: csid;
	m_confSection = (m_checksumInd == 0)? 0: (config < 0 || config > 2)? 1: config;
};


ostream& operator<<(ostream& os, const Version& v)
{
	return os << dec << "<" << v.major() << "><" << v.minor() << ">";
}

ostream& operator<<(ostream& os, const MauMsg::MsgHeader& msg)
{
	os << hex << "<" << msg.protId() << "><" << dec
			<< msg.version() << "><"
			<< msg.primitive() << "><"
			<< msg.reserved() << ">>";
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::GetOperationStateMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::GetOperationStateRspMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg)
			<< "<" << msg.error()
			<< "><" << msg.state() << ">";
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::GetQuorumInfomationMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::GetQuorumInfomationRspMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	if (msg.error())
	{
		os << "<" << msg.error() << ">";
	}
	else
	{
		os << "<" << msg.error()
			<< "><" << msg.connView()
			<< "><" << msg.quorumView()
			<< "><" << msg.activeMau()
			<< "><" << msg.activeMask()
			<< ">";
	}
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::SetActiveMaskMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	os << "<" << msg.mask() << ">";
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::SetActiveMasRspMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	os << "<" << msg.error() << ">";
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::PerformChecksumMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	os << "<" << msg.checksumIndicator() << ">";
	os << "<" << msg.confSection() << ">";
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::PerformChecksumRspMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	os << "<" << msg.error() << ">";	return os;
}

ostream& operator<<(ostream& os, const MauMsg::InstallMibsMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	os << "<" << msg.operationIndicator() << ">";
	os << "<" << msg.confSection() << ">";
	return os;
}

ostream& operator<<(ostream& os, const MauMsg::InstallMibsRspMsg& msg)
{
	os << static_cast<const MauMsg::MsgHeader&>(msg);
	os << "<" << msg.error() << ">";	return os;
}

