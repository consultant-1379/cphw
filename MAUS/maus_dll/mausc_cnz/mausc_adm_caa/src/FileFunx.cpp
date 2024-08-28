/*
 * FileFunx.cpp
 *
 *  Created on: Dec 8, 2015
 *      Author: xdtthng
 */

#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

#include "FileFunx.h"

using namespace std;

class UMask
{
public:
    UMask() {
        m_mode = umask(0);
    }
    ~UMask() {
        umask(m_mode);
    }
private:
    mode_t  m_mode;
};

//
// Create recursive directories if fails check errno
// ============================================================================
bool FileFunx::createDirectories(const string& path, mode_t mode)
{
    string::size_type search_pos = 0;
    string::size_type result_pos = 0;
    int result;
    UMask a;

    do
    {
        result_pos = path.find("/", search_pos);

        if (result_pos == string::npos)
        {
            // can't find '/'
            break;
        }

        result = mkdir(path.substr(0, result_pos + 1).c_str(), mode);

        if (result == -1 && errno != EEXIST)
        {
            // Can't create directory
            break;
        }

        search_pos = result_pos + 1;
    }
    while (true);

    // Check if directory exist
    struct stat buf;
    int org_errno = errno;

    if (stat(path.c_str(), &buf) == 0)
    {
        return true;
    }

    errno = org_errno;

    // Check errno for error details
    return false;
}

//
// Create File if sucecced return file descriptor otherwise throw exception
// Note: flags is defined in #include <fcntl.h> such as O_RDONLY,...
//-------------------------------------------------------------------------
int FileFunx::createFile(const char* fileName, int flags, mode_t mode)
{
    //TRACE((LOG_LEVEL_INFO, "FileFunx::CreateFileX(%s, %d)", 0, fileName, flags));

    UMask a;
    int fd = open(fileName, flags, mode);

    if (fd == -1)
    {
    	//TRACE((LOG_LEVEL_ERROR, "Failed to create file; throw exception", 0));
    }

    return fd;
}

//
// Close File
//----------------------------------------------------------------------
bool FileFunx::closeFile(int fd)
{
    //TRACE((LOG_LEVEL_INFO, "FileFunx::CloseFileX(%d)", 0, fd));

    return (close(fd) == 0);
}

//
// Check file exist
//----------------------------------------------------------------------
bool FileFunx::fileExists(const char* fileName)
{
    struct stat fileStat;
    return stat(fileName, &fileStat) == 0;
}
