/*
 * ParseIni.h
 *
 *  Created on: Dec 8, 2015
 *      Author: xdtthng
 */

#ifndef PARSEINI_H_
#define PARSEINI_H_

#include <string>

namespace ParseIni {

int getPrivateProfile(const char* section, const char* key, int default_val, const char* filename);
bool writePrivateProfile(const char* section, const char* key, int value, const char* filename);

std::string getPrivateProfile(const char* section, const char* key, const char* defaultStr, const char* filename);

}

#endif /* PARSEINI_H_ */
