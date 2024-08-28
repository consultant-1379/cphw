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

	enum { MSG_SIZE_IN_BYTES = 1024 };

	enum PrimitiveT
	{
		FirstPrimitive = 0,
		GetOperationState = FirstPrimitive,
		GetOperatoinState_Rsp,
		Enable,
		Enable_Rsp,
		Disable,
		Disable_Rsp
	};

	enum ErrorCodeT
	{
		FirstErrorCode = 0,
		Success = FirstErrorCode,
		Protocol_Version_Unsupported,
		Internal_Error,
		Unknown_Primitive,
		Already_Enabled,
		Already_Disabled
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
		GetOperationStateRspMsg(int32_t error, int32_t state, int32_t pid, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error), m_state(state), m_pid(pid)
			{};
		int32_t state() const { return m_state; }
		int32_t	error() const { return m_error; }
		int32_t	pid()	const { return m_pid;	}
	private:
		int32_t		m_error;
		int32_t		m_state;
		int32_t		m_pid;
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
		EnableRspMsg(int32_t error, int32_t pid, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error), m_pid(pid)
			{};
		int32_t	error() const { return m_error; }
		int32_t	pid()	const { return m_pid;	}
	private:
		int32_t		m_error;
		int32_t		m_pid;
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
		DisableRspMsg(int32_t error, int32_t pid, int maj = VER_MAJOR, int min = VER_MINOR):
			MsgHeader(s_primitive, maj, min), m_error(error), m_pid(pid)
			{};
		int32_t	error() const { return m_error; }
		int32_t	pid()	const { return m_pid;	}
	private:
		int32_t		m_error;
		int32_t		m_pid;
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

#endif // MAUMSG_H_
