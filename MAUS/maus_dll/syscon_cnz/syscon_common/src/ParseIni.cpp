/*
 * ParseIni.cpp
 *
 *  Created on: Dec 8, 2015
 *      Author: xdtthng
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ParseIni.h"
#include "FileFunx.h"
#include "AppTrace.h"

using namespace std;

int ParseIni::getPrivateProfile(const char* section, const char* key, int default_val, const char* filename)
{
    using boost::property_tree::ptree;

    ptree pt;
    try {
        read_ini(filename, pt);
    }
    catch (exception& e) {
    	TRACE_DEBUG(("MAUFunx::getPrivateProfile() failed to open file <%s> error <%s>", filename, e.what()));
        return default_val;
    }
    std::string keystr = section;
    keystr += ".";
    keystr += key;
    boost::optional<int> value = pt.get_optional<int>(keystr);

    return value? *value : default_val;
}

bool ParseIni::writePrivateProfile(const char* section, const char* key, int value, const char* filename)
{
    using boost::property_tree::ptree;

    ptree pt;
    try {
        // Create empty file if it doesn't exist
        int fd = FileFunx::createFile(filename);
        FileFunx::closeFile(fd);

        read_ini(filename, pt);
        std::string keystr = section;
        keystr += ".";
        keystr += key;
        pt.put(keystr, value);
        write_ini(filename, pt);
    }
    catch (...) {
        return false;
    }

    return true;
}

string ParseIni::getPrivateProfile(const char* section, const char* key,
    const char* defaultStr, const char* filename)

{
    using boost::property_tree::ptree;

    ptree pt;
    try {
        read_ini(filename, pt);
    }
    catch (...) {
        return string(defaultStr);
    }
    std::string keystr = section;
    keystr += ".";
    keystr += key;
    boost::optional<string> value = pt.get_optional<string>(keystr);

    return value? *value : defaultStr? string(defaultStr) : string("");

}

bool ParseIni::writePrivateProfile(const char* section, const char* key, const char* value, const char* filename)
{
    using boost::property_tree::ptree;

    ptree pt;
    try {
        // Create empty file if it doesn't exist
        int fd = FileFunx::createFile(filename);
        FileFunx::closeFile(fd);

        read_ini(filename, pt);
        std::string keystr = section;
        keystr += ".";
        keystr += key;
        pt.put(keystr, value);
        write_ini(filename, pt);
    }
    catch (...) {
        return false;
    }

    return true;
}

