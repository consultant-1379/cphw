/*
 * FileFunx.cpp
 *
 *  Created on: Dec 8, 2015
 *      Author: xdtthng
 */

#include <cerrno>
#include <dirent.h>

#include "FileFunx.h"
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>

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
// Create File if sucecced return file descriptor otherwise ...
// Note: flags is defined in #include <fcntl.h> such as O_RDONLY,...
//-------------------------------------------------------------------------
int FileFunx::createFile(const char* fileName, int flags, mode_t mode)
{

    UMask a;
    int fd = open(fileName, flags, mode);

    if (fd == -1)
    {
    	//TRACE;
    }

    return fd;
}

//
// Close File
//----------------------------------------------------------------------
bool FileFunx::closeFile(int fd)
{

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


//
// Get file size as u_int64
//----------------------------------------------------------------------
int FileFunx::getFileSize(const char* fileName)
{
    struct stat fileStat;

    if (stat(fileName, &fileStat) != 0)
    {
        // TRACE
    	return -1;
    }

    return fileStat.st_size;
}

int FileFunx::rmDir(const char* name)
{
	//TRACE_DEBUG(("FileFunx::rmDir(%s)", name));

	DIR *dp;
	struct dirent *ep;

	dp = opendir(name);
	if (dp == 0)
	{
		//TRACE_DEBUG(("FileFunx::rmDir() failed to opendir()"));
		return -1;
	}

	string fileName;
	string fname = name;
	fname += "/";
	int errorCount = 0;
	while ((ep = readdir(dp)) != NULL)
	{
		if (ep->d_type == DT_DIR)
		{
			// Only interest on files
			// Don't want to remove "." and ".." directories
			continue;
		}

		//cout << ep->d_name << endl;
		fileName = fname;
		fileName += ep->d_name;
		if (remove(fileName.c_str()) != 0)
		{
			//TRACE_DEBUG(("FileFunx::rmDir() failed to remove <%s>", fileName.c_str()));
			++errorCount;
		}
	}
    if (closedir(dp) != 0)
    {
    	//TRACE_DEBUG(("FileFunx::rmDir() failed to closedir()"));
    	return -2;
    }


    if (rmdir(name))
    	++errorCount;

    //TRACE_DEBUG(("FileFunx::rmDir() return <%d>", errorCount));
	return errorCount;

}
