/*
 * Text_Code.h
 *
 *  Created on: Aug 2, 2014
 *      Author: xdtthng
 */

#ifndef TEXT_CODE_H_
#define TEXT_CODE_H_

#include <string>
#include <map>

enum Text_Code
{
	cStart				= 0,		// Hello string
	cMaus1CompName		= 1,		// MAUS service component name 1
	cMaus2CompName		= 2,		// MAUS service component name 2
	cNoService			= 3,		// used to run the daemon as console service for debugging
	cLockPath			= 4,		// Path to lock file
	cForOption			= 5,		// Text for composing error message used in commands
	cLockFileExtention	= 6,		// Extension of lock file name
	cSiNumber			= 7,		// with service instance number
	cEnd				= cSiNumber	// Must change this when adding new code
};

extern std::map<Text_Code, const std::string> text_string;

#endif // TEXT_CODE_H_
