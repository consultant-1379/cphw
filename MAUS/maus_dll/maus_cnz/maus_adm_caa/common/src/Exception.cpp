/*
 * Exception.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: xdtthng
 */

#include "Exception.h"

using namespace std;

const char* CodeException::CE_ERROR_CODE_MSG[CodeException::NO_CE_ERROR_CODES] = {
	"No Error",
	"Cannot contact Configuration Service",
	"CP Name is not defined"
};

Exception::Exception(const string& msg, int errcode, const char* file, int line)
	                : m_msg(msg), m_errcode(errcode), m_file(file), m_line(line)
{
}

CodeException::CodeException(CE_ERROR_CODE errcode, const char* file, int line):
				Exception(CE_ERROR_CODE_MSG[errcode], errcode, file, line) {
}
