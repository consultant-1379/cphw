/*
 * maus_sha.cpp
 *
 *  Created on: Jun 14, 2015
 *      Author: xdtthng
 */

#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <sys/types.h>
#include <sys/stat.h>


#include "maus_trace.h"
#include "maus_sha.h"
#include "AtomicFlag.h"

#include <openssl/sha.h>

extern BoolFlagT s_applicationAborting;
extern BoolFlagT s_abortChecksum;

using namespace std;


MausSha::MausSha(const std::string& path, const std::string& name): m_libPath(path),
	m_libName(name)
{
	TRACE_DEBUG(("MausSha::MausSha() ctor"));
}

MausSha::~MausSha()
{
	TRACE_DEBUG(("MausSha::~MausSha() dtor"));
}

int MausSha::readHash(std::string& hash)
{
	TRACE_DEBUG(("MausSha::readHash()"));

	hash.reserve(64);
	hash = "";
	int result = 10;

	string soName = m_libPath;
	soName += m_libName;
	string soTxtName = soName;
	soTxtName += ".txt";

	ifstream soTxtFile(soTxtName.c_str());
	if (!soTxtFile.is_open())
	{
		TRACE_DEBUG(("MausSha::readHash() failed to open file <%s>", soTxtName.c_str()));
		return 1;
	}

	string line;
	vector<string> tokens;
	string hashLine = "";
	hashLine.reserve(64);
	bool hashFound = false;
	while(getline(soTxtFile, line))
	{

		if (s_abortChecksum || s_applicationAborting)
			return 30;

		vector<string>().swap(tokens);
        boost::split(tokens, line, boost::is_any_of("\t "), boost::token_compress_on);
		//copy(tokens.begin(), tokens.end(), ostream_iterator<string>(cout, "\n"));

        hashFound = (tokens.size() > 1 ) && (tokens[1] == m_libName);
		if (hashFound)
		{
			hashLine = tokens[0];
			break;
		}
    }
	vector<string>().swap(tokens);
	if (hashFound && hashLine.length() < 128)
	{
		result = 0;
		hash = hashLine;
	}
	else if (hashFound && hashLine.length() >= 128)
	{
		result = 10;
		hash = hashLine;
	}
	else
	{
		result = 20;
	}
	soTxtFile.close();

	TRACE_DEBUG(("MausSha::readHash() returns hash length <%d>", hash.length()));
	TRACE_DEBUG(("MausSha::readHash() returns hash <%s>", hash.c_str()));
	TRACE_DEBUG(("MausSha::readHash() returns result <%d>", result));

	return result;
}

int MausSha::computeHash(std::string& hash)
{
	TRACE_DEBUG(("MausSha::computeHash()"));

	if (s_abortChecksum || s_applicationAborting)
		return 30;

	ssize_t size;

	// Get file size
	struct stat statRes;
	string fileName = m_libPath;
	fileName += m_libName;
	if (stat(fileName.c_str(), &statRes) == 0)
	{
		size = statRes.st_size;
		//cout << "the file size is " << size << endl;
		TRACE_DEBUG(("MausSha::computeHash() lib file size <%ld>", size));
	}
	else
	{
		//cout << "cannot get file size" << endl;
		TRACE_DEBUG(("MausSha::computeHash() cannot get lib file size"));
		return 1;
	}

	ssize_t quotion = size / BUFFER_SIZE;
	ssize_t remainder = size % BUFFER_SIZE;
	TRACE_DEBUG(("MausSha::computeHash() quotion <%ld> remainder <%ld>", quotion, remainder));

	ifstream sofile(fileName.c_str(), ios::in | ios::binary);
	if (!sofile.good())
	{
		//cout << "Error openning file" << endl;
		TRACE_DEBUG(("MausSha::computeHash() cannot open lib file"));
		return 2;
	}

	ssize_t total = 0;
	char buffer[BUFFER_SIZE];

	TRACE_DEBUG(("MausSha::computeHash() using open ssl lib"));

	SHA256_CTX ctx;
	SHA256_Init(&ctx);

	for(int i = 0; i < quotion; ++i)
	{
		// Check for abort before reading
		if (s_abortChecksum || s_applicationAborting)
			return 30;

		if (!sofile.read (buffer, BUFFER_SIZE))
		{
			//cout << "Error reading file" << endl;
			TRACE_DEBUG(("MausSha::computeHash() error when reading lib file"));
			return 3;
		}
		total += sofile.gcount();

		// Check for abort before hasing
		SHA256_Update(&ctx, buffer, BUFFER_SIZE);

		if (s_abortChecksum || s_applicationAborting)
			return 30;

	}

	// Check before reading the last bit
	if (s_abortChecksum || s_applicationAborting)
		return 30;

	// Read the last part
	if (!sofile.read (buffer, remainder))
	{
		//cout << "Error reading the last part" << endl;
		TRACE_DEBUG(("MausSha::computeHash() error when reading the last part of lib file"));
		return 4;
	}

	ssize_t nLast = sofile.gcount();
	total += nLast;

	if (sofile.eof() && total != size)
	{
		//cout << "Reading not reach eof or not reading all" << endl;
		return 5;
	}
	//cout << "Total read size is " << total << endl;
	TRACE_DEBUG(("MausSha::computeHash() total read size is <%ld>", total));

	// check before hasing the last bit
	if (s_abortChecksum || s_applicationAborting)
		return 30;

	string().swap(hash);

	SHA256_Update(&ctx, buffer, nLast);
	unsigned char digest[SHA256_DIGEST_LENGTH];
	SHA256_Final(digest, &ctx);

	hash.reserve(2*SHA256_DIGEST_LENGTH);

	if (s_abortChecksum || s_applicationAborting)
		return 30;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
      static const char hexchar[16+1] = "0123456789abcdef";
      hash += hexchar[(digest[i] >> 4) & 15];
      hash += hexchar[ digest[i]       & 15];
    }

	if (s_abortChecksum || s_applicationAborting)
		return 30;
	TRACE_DEBUG(("MausSha::computeHash() computed hash <%s>", hash.c_str()));
	return 0;
}
