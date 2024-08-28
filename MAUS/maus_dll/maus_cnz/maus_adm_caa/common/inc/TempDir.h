/*
 * TempDir.h
 *
 *  Created on: Jan 14, 2016
 *      Author: xdtthng
 */

#ifndef TEMPDIR_H_
#define TEMPDIR_H_

#include <string>

class TempDir
{
public:
	TempDir(const std::string& newdir);
	~TempDir();

private:
	std::string m_olddir;
};

#endif /* TEMPDIR_H_ */
