#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include <cstdio>
#include "sha256.h"

using namespace std;


void test(string fileName)
{
	//static int BUFFER_SIZE = 32768;
	static int BUFFER_SIZE = 0x10000;
	
	cout << "file name is " << fileName << endl;
	ssize_t size;
	
	// Get file size
	struct stat statRes;
	if (stat(fileName.c_str(), &statRes) == 0)
	{
		size = statRes.st_size;
		cout << "the file size is " << size << endl;
	}
	else
	{
		cout << "cannot get file size" << endl;
		return;
	}
	
	ssize_t quotion = size / BUFFER_SIZE;
	ssize_t remainder = size % BUFFER_SIZE;
	cout << "size / BUFFER_SIZE " << quotion << endl;
	cout << "size % BUFFER_SIZE " << remainder << endl; 
	cout << "reconstruct size " << quotion * BUFFER_SIZE + remainder << endl;
	cout << "buffer size is " << BUFFER_SIZE << endl;
	
	cout << endl;
	cout << "Open file " << fileName << endl;
	
	ifstream sofile(fileName.c_str(), ios::in | ios::binary);
	if (!sofile.good())
	{
		cout << "Error openning file" << endl;
		return;
	}
	cout << endl;
	cout << "Reading file " << fileName << endl;
	
	ssize_t total = 0;
	char buffer[BUFFER_SIZE];
	SHA256 soSha;
	for(int i = 0; i < quotion; ++i)
	{
		if (!sofile.read (buffer, BUFFER_SIZE))
		{
			cout << "Error reading file" << endl;
			return;
		}
		total += sofile.gcount();
		soSha.add(buffer, BUFFER_SIZE);
	}
	// Read the last part
	sofile.read (buffer, BUFFER_SIZE);
	if (sofile.good())
	{
		cout << "Error reading the last part" << endl;
		return;
	}
	
	ssize_t nLast = sofile.gcount();;
	total += nLast;
	
	if (sofile.eof() && total != size)
	{
		cout << "Reading not reach eof or not reading all" << endl;
		return;
	}
	cout << "Total read size is " << total << endl;
	
	soSha.add(buffer, nLast);
	
	cout << soSha.getHash() << endl;
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 2)
		{
			cout << "File name needed" << endl;
			return (1);
		}
		string fname = argv[1];
		test(fname);
	}
	catch (...)
	{
		cout << "error occur during execution" << endl;
	}
	return 0;
}
