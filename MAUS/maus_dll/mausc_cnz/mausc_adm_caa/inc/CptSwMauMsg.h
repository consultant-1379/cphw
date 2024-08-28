/*
 * CptSwMauMsg.h
 *
 *  Created on: May 9, 2016
 *      Author: xdtthng
 */

#ifndef CPTSWMAUMSG_H_
#define CPTSWMAUMSG_H_

#include <stdint.h>
#include <cstring>

#pragma pack(push, 1)


class CptSwMauMsg
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
		GetPtcoiReq = FirstPrimitive,
		GetPtcoiRsp
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

	class GetPtcoiReqMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetPtcoiReqMsg(int32_t code, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_serviceIdCode(code)
			{};
		int32_t serviceIdCode() const { return m_serviceIdCode; }
	private:
		int32_t		m_serviceIdCode;
	};

	class GetPtcoiRspMsg: public MsgHeader
	{
	public:
		static const PrimitiveT s_primitive;
		GetPtcoiRspMsg(int32_t error, int32_t state, int seqno, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, seqno, maj, min), m_error(error), m_ptcoiStatus(state)
			{};
		int32_t ptcoiStatus() const { return m_ptcoiStatus; }
		int32_t	error() const { return m_error; }
	private:
		int32_t		m_error;
		int32_t		m_ptcoiStatus;
	};

	CptSwMauMsg()	{ reset(); }
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

std::ostream& operator<<(std::ostream& os, const CptSwMauMsg::Version& v);
std::ostream& operator<<(std::ostream& os, const CptSwMauMsg::MsgHeader& msg);
std::ostream& operator<<(std::ostream& os, const CptSwMauMsg::GetPtcoiReqMsg& msg);
std::ostream& operator<<(std::ostream& os, const CptSwMauMsg::GetPtcoiRspMsg& msg);


#endif /* CPTSWMAUMSG_H_ */
