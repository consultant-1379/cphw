/*
 * testOI.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include <string>
#include <sys/poll.h>

#include "acs_apgcc_oihandler_V3.h"
#include "acs_apgcc_objectimplementerinterface_V3.h"
#include "acs_apgcc_omhandler.h"
#include "MausModel.h"

using namespace std;

class TestOI : public acs_apgcc_objectimplementerinterface_V3
{
public:
	TestOI();
	~TestOI();
	void run();
};

TestOI::TestOI():
		acs_apgcc_objectimplementerinterface_V3(MausModel::immAttr_maus1IdInstance,
				MausModel::immImpl_Maus1Name, ACS_APGCC_ONE)
{

}

TestOI::~TestOI()
{

}

void TestOI::run()
{
	struct pollfd fds[1];
	int pollResult = poll(fds, 1, 10000);
}

int main(int, char**)
{

    try {


    }
	catch (...) {
		cout << "Unknown error" << endl;
		return 222;
	}

	return 0;
}


