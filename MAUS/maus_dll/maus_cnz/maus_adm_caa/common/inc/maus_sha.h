/*
 * maus_sha.h
 *
 *  Created on: Jun 14, 2015
 *      Author: xdtthng
 */

#ifndef MAUS_SHA_H_
#define MAUS_SHA_H_

#include <string>

class MausSha
{

public:

	MausSha(const std::string& path, const std::string& name);
	~MausSha();

	// Get hash value from txt file
	// Return value
	// 0, possible sha256, hash value returned in string hash
	// 1, cannot open txt file
	// 2, other fautls ...
	// ....
	// ....
	// 10, other sha, value is in string
	int readHash(std::string& hash);

	// Calculate hash value of the lib file
	// Return value
	// 0, calculation ok, retuned in hash
	// 1, error during calculation
	int computeHash(std::string& hash);

private:
	std::string		m_libPath;
	std::string		m_libName;

	static const int BUFFER_SIZE = 0x10000;
};

#endif /* MAUS_SHA_H_ */
