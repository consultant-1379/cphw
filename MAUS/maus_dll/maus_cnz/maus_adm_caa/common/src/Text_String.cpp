/*
 * Text_Code.cpp
 *
 *  Created on: Aug 2, 2014
 *      Author: xdtthng
 */

#include "Text_Code.h"
#include "boost/assign/list_of.hpp"
#include "boost/algorithm/string.hpp"

std::map<Text_Code, const std::string> text_string = boost::assign::map_list_of
	(cStart, "maus text message starts here")
	(cMaus1CompName, "MAUS1")
	(cMaus2CompName, "MAUS2")
	(cNoService, "noservice")
	(cLockPath, "/var/run/")
	(cForOption, "for option")
	(cLockFileExtention, ".lock")
	(cSiNumber, "instance number");
