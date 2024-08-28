/*
 * FileFunx.h
 *
 *  Created on: Dec 8, 2015
 *      Author: xdtthng
 */

#ifndef FILEFUNX_H_
#define FILEFUNX_H_

#include <fcntl.h>
#include <string>

namespace FileFunx
{

//
// Create recursive directories if fails check errno
// ============================================================================
bool createDirectories(const std::string& path, mode_t mode = 0755);

//
// Wrapper for Create or Open File
//=========================================================================
int createFile(const char* fileName, int flags = O_RDWR | O_CREAT, mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

//
// Wrapper for close file
//=========================================================================
bool closeFile(int fd);

//
// Check file exist
//=========================================================================
bool fileExists(const char* fileName);

}

#endif /* FILEFUNX_H_ */
