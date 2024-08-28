/*
 * MauMsg.h
 *
 *  Created on: Feb 14, 2015
 *      Author: xdtthng
 */

#ifndef MAUMSG_H_
#define MAUMSG_H_

#include <stdint.h>
#include <cstring>

#pragma pack(push, 1)

class Version
{

public:
	Version(int16_t major = 0, int16_t minor = 0) { set(major, minor); }

	int16_t major() const { return m_major; }
	int16_t minor() const { return m_minor; }
	void set(int16_t major, int16_t minor) { m_major = major; m_minor = minor; }
	void major(int16_t val) { m_major = val; }
	void minor(int16_t val) { m_minor = val; }

	bool operator==(Version rhs) const { return m_major == rhs.m_major && m_minor == rhs.m_minor; }
	bool operator!=(Version rhs) const { return !(*this == rhs); }
	bool operator>(Version rhs) const { return m_major > rhs.m_major || m_minor > rhs.m_minor; }
	bool operator>=(Version rhs) const { return ((*this > rhs) || (*this == rhs)); }
	bool operator<=(Version rhs) const { return !(*this > rhs); }
	bool operator<(Version rhs) const { return !(*this >= rhs); }

private:
	int16_t m_major;
	int16_t m_minor;
};
//
// inlines
//===========================================================================
#pragma pack(pop)

class MauMsg
{
public:
	enum MauProtT
	{
		MAU_PROT_ID = 0xdead
	};

	enum {
		VER_MAJOR = 1,
		VER_MINOR = 0,
	};

	enum { MSG_SIZE_IN_BYTES = 512 };

	enum PrimitiveT
	{
		FirstPrimitive = 0,
		GetOperationState = FirstPrimitive,
		GetOperatoinState_Rsp,
		Enable,
		Enable_Rsp,
		Disable,
		Disable_Rsp,
		GetQuorumInformation,
		GetQuorumInformation_Rsp,
		SetActiveMask,
		SetActiveMask_Rsp,
		PerformChecksum,
		PerformChecksum_Rsp,
		SetFCState,
		SetFCstate_Rsp
	};

	enum ErrorCodeT
	{
		FirstErrorCode 						= 0,
		Success 							= FirstErrorCode,
		Protocol_Version_Unsupported 		= 1,
		Internal_Error						= 2,
		Unknown_Primitive					= 3,
		Already_Enabled						= 4,
		Already_Disabled					= 5,
		GeneralError 						= 6,
		MauCoreBusy							= 7,
		MauCoreNotRunning					= 8,
		GetQuorumInfoFailed					= 9,
		SetActiveMaskFailed					= 10,
		SystemShutdown						= 11,
		ApplicationBusy						= 12,
		ChecksumFailed						= 13,
		MauCoreRunning						= 14,
		SetFCStateFailed					= 15
	};

	class MsgHeader
	{
	protected:
		MsgHeader(PrimitiveT prim, int maj, int min) :
				m_protId(MAU_PROT_ID), m_version(maj, min), m_primitive(prim),
				m_reserved(0)
				{ }
	public:

		PrimitiveT primitive() const { return m_primitive; }
		MauProtT protId() const { return m_protId; }
		Version version() const { return m_version; }
		int32_t	reserved() const { return m_reserved; }
	private:
		MauProtT	m_protId;
		Version		m_version;
		PrimitiveT	m_primitive;
		int32_t		m_reserved;
	};

	// Checksum indicator; m_checksumInd
	// m_checksumInd == 0; perform checksum in the [cur] section of maus[12].conf
	// m_checksumInd == 1; perform checksum in the [section] specified in m_confSection
	// m_checksumInd == other values; reserved
	//
	// Configuration Section; m_confSection
	// m_confSection == 0; [new] section
	// m_confSection == 1; [cur] section
	// m_confSection == 2; [bak] section
	class PerformChecksumMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
#if 0
		PerformChecksumMsg(int csid, int config, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_checksumInd(csid), m_confSection(config)
		{
			m_checksumInd = (csid < 0 || csid > 1)? 0: csid;
			m_confSection = (m_checksumInd == 0)? 0: (config < 0 || config > 2)? 1: config;
		};
#endif
		PerformChecksumMsg(int csid, int config, int maj = VER_MAJOR, int min = VER_MINOR);

		int32_t	checksumIndicator() const { return m_checksumInd; }
		int32_t	confSection() const { return m_confSection; }
	private:
		int32_t	m_checksumInd;
		int32_t	m_confSection;
	};

	class PerformChecksumRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		PerformChecksumRspMsg(int32_t error, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error)
			{};
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
	};

	class GetOperationStateMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetOperationStateMsg(int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min)
			{};
	};

	class GetOperationStateRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetOperationStateRspMsg(int32_t error, int32_t state, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error), m_state(state)
			{};
		int32_t state() const { return m_state; }
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
		int32_t		m_state;
	};

	class EnableMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		EnableMsg(int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min)
			{};
	};

	class EnableRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		EnableRspMsg(int32_t error, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error)
			{};
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
	};

	class DisableMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		DisableMsg(int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min)
			{};
	};

	class DisableRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		DisableRspMsg(int32_t error, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error)
			{};
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
	};

	class GetQuorumInfomationMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetQuorumInfomationMsg(int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min)
			{};
	};

	class GetQuorumInfomationRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;

		// For error case
		GetQuorumInfomationRspMsg(int32_t error, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min),
			m_error(error),
			m_connectivityView(-1),
			m_quorumView(-1),
			m_activeMau(-1),
			m_activeMask(-1)
		{
		};

		// For successfull case
		GetQuorumInfomationRspMsg(int32_t error, int32_t connView,
			int32_t quorumView, int32_t active, int32_t mask,
			int maj = VER_MAJOR, int min = VER_MINOR):
				MsgHeader(s_primitive, maj, min),
				m_error(error),
				m_connectivityView(connView),
				m_quorumView(quorumView),
				m_activeMau(active),
				m_activeMask(mask)
		{
		};

		int32_t	error() const 	{ return m_error; }
		int32_t connView() const { return m_connectivityView; }
		int32_t quorumView() const { return m_quorumView; }
		int32_t activeMau() const { return m_activeMau; }
		int32_t activeMask() const { return m_activeMask; }

	private:
		int32_t	m_error;			// When error == 0, the following is valid
        int32_t m_connectivityView;	// Bit 0 AP-A, bit 1 AP-B, bit 2 CP-A, bit 3 CP-B
        int32_t m_quorumView;       // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-A, bit 3 CP-B
        int32_t m_activeMau;        // 0 = AP-A, 1 = AP-B, 2 = CP-A, 3 = CP-B
        int32_t m_activeMask;       // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-SB, bit 3 CP-EX

	};


	class SetActiveMaskMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		SetActiveMaskMsg(int32_t mask, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_mask(mask)
			{};

		int32_t mask() const { return m_mask; }
	private:
		int32_t		m_mask;
	};

	class SetActiveMasRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		SetActiveMasRspMsg(int32_t error, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error)
			{};
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
	};

	class SetFCStateMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		SetFCStateMsg(int32_t fcState, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_fcState(fcState)
			{};

		int32_t fcState() const { return m_fcState; }
	private:
		int32_t		m_fcState;
	};

	class SetFCStateRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		SetFCStateRspMsg(int32_t error, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error)
			{};
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
	};

public:
	MauMsg()	{ reset(); }
	char* addr() { return reinterpret_cast<char*>(m_buffer); }
	const char* addr() const { return reinterpret_cast<const char*>(m_buffer); }
	MsgHeader* msgHeader() { return reinterpret_cast<MsgHeader*>(addr()); }
	const MsgHeader* msgHeader() const { return reinterpret_cast<const MsgHeader*>(addr()); }

	void reset()
	{
		memset(m_buffer, 0, MSG_SIZE_IN_BYTES);
	}

private:
	char		m_buffer[MSG_SIZE_IN_BYTES];

};

std::ostream& operator<<(std::ostream& os, const Version& v);
std::ostream& operator<<(std::ostream& os, const MauMsg::MsgHeader& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::GetOperationStateMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::GetOperationStateRspMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::GetQuorumInfomationMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::GetQuorumInfomationRspMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::SetActiveMaskMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::SetActiveMasRspMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::PerformChecksumMsg& msg);
std::ostream& operator<<(std::ostream& os, const MauMsg::PerformChecksumRspMsg& msg);

#endif // MAUMSG_H_
