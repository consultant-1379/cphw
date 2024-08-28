/*
 * CSException.h
 *
 *  Created on: Jul 15, 2014
 *      Author: xdtthng
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_


#include <string>


//using namespace std;
//
// creates and Exception, and throws it
//#define THROW_X(msg, code) { Exception x((msg), (code), __FILE__, __LINE__); throw x; }

//
//
//----------------------------------------------------------------------------
class Exception
{

public:
	virtual ~Exception() { }
	virtual const char* name() const = 0;
	const std::string& what() const { return m_msg; }
	int errcode() const { return m_errcode; }
	const std::string& file() const { return m_file; }
	int line() const { return m_line; }

	Exception(const std::string& msg, int errcode, const char* file, int line);

	std::string m_msg;
	int m_errcode;
	std::string m_file; // source file
	int m_line; // source line
};


#define THROW_XCODE(code) { \
	CodeException x((code), __FILE__, __LINE__); \
	throw x; }


class CodeException: public Exception
{
public:
	enum CE_ERROR_CODE
	{
		CE_FIRST_ERROR_CODE = 0,
		CE_NO_ERROR = CE_FIRST_ERROR_CODE,
		CE_CSUNREACHABLE,
		CE_CS_CPNAME_UNDEFINED,
		CE_LAST_ERROR_CODE
	};

	enum { NO_CE_ERROR_CODES = CE_LAST_ERROR_CODE - CE_FIRST_ERROR_CODE };
	static const char* CE_ERROR_CODE_MSG[NO_CE_ERROR_CODES];

	CodeException(CE_ERROR_CODE code, const char* file, int line);
	~CodeException() { }
	virtual const char* name() const { return CE_ERROR_CODE_MSG[m_errcode]; }

private:

};


#endif // CSCSException_H_
