/*
 * CpMauMsg.h
 *
 *  Created on: Jul 6, 2015
 *      Author: xdtthng
 */

#ifndef CPMAUMSG_H_
#define CPMAUMSG_H_

#include <stdint.h>
#include <cstring>

#pragma pack(push, 1)

class CpMauMsg
{
public:

	enum { MSG_SIZE_IN_BYTES = 512 };

	enum {
		VER_MAJOR = 1,
		VER_MINOR = 0,
	};

	enum PrimitiveT
	{
		FirstPrimitive = 0,
		GetFexRq = FirstPrimitive,
		GetFexRq_Rsp,
		SetFexRq,
		SetFexRq_Rsp,
		GetApgStateRq,
		GetApgStateRq_Rsp,
		GetAdminState,
		GetAdminState_Rsp
	};

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

	class MsgHeader
	{
	protected:
		MsgHeader(PrimitiveT prim, uint32_t seqno, int maj, int min) :
				m_version(maj, min), m_primitive(prim), m_seqNumber(seqno), m_reserved(0)
				{ }
	public:

		PrimitiveT primitive() const { return m_primitive; }
		Version version() const { return m_version; }
		uint32_t seqNo() const { return m_seqNumber; }
		void seqNo(int seqno) {m_seqNumber = seqno; }
		int32_t	reserved() const { return m_reserved; }
	private:
		Version		m_version;
		PrimitiveT	m_primitive;
		uint32_t	m_seqNumber;
		int32_t		m_reserved;
	};

	class GetFexReqMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetFexReqMsg(int32_t code, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_serviceIdCode(code)
			{};
		int32_t serviceIdCode() const { return m_serviceIdCode; }
	private:
		int32_t		m_serviceIdCode;
	};

	class GetFexRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetFexRspMsg(int32_t error, int32_t state, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_error(error), m_fexReqState(state)
			{};
		int32_t fexReqState() const { return m_fexReqState; }
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
		int32_t		m_fexReqState;
	};

	class SetFexReqMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		SetFexReqMsg(int32_t code, int32_t state, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_serviceIdCode(code), m_fexReqState(state)
			{};
		int32_t fexReqState() const { return m_fexReqState; }
		int32_t serviceIdCode() const { return m_serviceIdCode; }
	private:
		int32_t		m_serviceIdCode;
		int32_t		m_fexReqState;
	};

	class SetFexRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		SetFexRspMsg(int32_t error, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_error(error)
			{};
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
	};


	// GetAdminStateReqMsg
	class GetAdminStateReqMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetAdminStateReqMsg(int32_t code, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_serviceIdCode(code)
			{};
		int32_t serviceIdCode() const { return m_serviceIdCode; }
	private:
		int32_t		m_serviceIdCode;
	};

	// GetAdminStateRspMsg
	class GetAdminStateRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetAdminStateRspMsg(int32_t error, int32_t state, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_error(error), m_adminState(state)
			{};
		int32_t adminState() const { return m_adminState; }
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
		int32_t		m_adminState;
	};


	// GetAdminStateReqMsg
	class GetApgStateReqMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetApgStateReqMsg(int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min)
			{};
		//int32_t serviceIdCode() const { return m_serviceIdCode; }
	private:
		//int32_t		m_serviceIdCode;
	};

	class GetApgStateRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetApgStateRspMsg(int32_t error, int32_t aState, int32_t bState, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_error(error), m_apaState(aState), m_apbState(bState)
			{};
		int32_t apaState() const { return m_apaState; }
		int32_t apbState() const { return m_apbState; }
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
		int32_t		m_apaState;
		int32_t		m_apbState;
	};



	CpMauMsg()	{ reset(); }
	char* addr() { return reinterpret_cast<char*>(m_buffer); }
	const char* addr() const { return reinterpret_cast<const char*>(m_buffer); }
	MsgHeader* msgHeader() { return reinterpret_cast<MsgHeader*>(addr()); }
	const MsgHeader* msgHeader() const { return reinterpret_cast<const MsgHeader*>(addr()); }

	void reset()
	{
		memset(m_buffer, 0, MSG_SIZE_IN_BYTES);
	}

private:
	int32_t		m_buffer[MSG_SIZE_IN_BYTES/sizeof(int32_t)];

};

#pragma pack(pop)

std::ostream& operator<<(std::ostream& os, const CpMauMsg::Version& v);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::MsgHeader& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::GetFexReqMsg& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::GetFexRspMsg& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::SetFexReqMsg& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::SetFexRspMsg& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::GetAdminStateReqMsg& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::GetAdminStateRspMsg& msg);

std::ostream& operator<<(std::ostream& os, const CpMauMsg::GetApgStateReqMsg& msg);
std::ostream& operator<<(std::ostream& os, const CpMauMsg::GetApgStateRspMsg& msg);

#endif /* CPMAUMSG_H_ */
