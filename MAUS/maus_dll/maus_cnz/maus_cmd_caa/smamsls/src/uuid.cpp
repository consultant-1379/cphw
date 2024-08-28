/*
NAME
*/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <vector>

#include "ACS_CS_API.h"
#include <acs_apgcc_paramhandling.h>
#include <ACS_APGCC_CommonLib.h>

using namespace std;

int getuuid(int cpId, int side)
{
	//int sysId = cpId + 1000;
	(void) cpId;
	(void) side;

	ACS_CS_API_HWC* hwcTable = ACS_CS_API::createHWCInstance();
	if (!hwcTable)
		return -1;

	ACS_CS_API_BoardSearch* boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
	if (!boardSearch)
	{
		ACS_CS_API::deleteHWCInstance(hwcTable);
		return -2;
	}

	boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_CPUB);
	//boardSearch->setSysId(sysId);
	//boardSearch->setSide(side);
	ACS_CS_API_IdList boardList;

	//uint16_t sysno;
	//int res = (hwcTable->getBoardIds(boardList, boardSearch) == ACS_CS_API_NS::Result_Success)?
	//		(hwcTable->getSysNo(sysno, boardList[0]), sysno) : -3;

	int res = -99;
	if (hwcTable->getBoardIds(boardList, boardSearch) == ACS_CS_API_NS::Result_Success)
	{
		string uuidStr;
		unsigned short sysId, side;
		cout << "boardlist size " << boardList.size() << endl;

		for (unsigned int i = 0; i < boardList.size(); ++i)
		{
			if (hwcTable->getSysId(sysId, boardList[i]) == ACS_CS_API_NS::Result_Success
					&& hwcTable->getSide(side, boardList[i]) == ACS_CS_API_NS::Result_Success)
			{
				if (hwcTable->getUuid(uuidStr, boardList[i]) == ACS_CS_API_NS::Result_Success)
				{
					cout << "get the uuid ok; " << uuidStr << " side: " << side << " sysId " << sysId << endl;
					res = 0;
				}
				else
				{
					cout << "Cannot get uuid" << endl;
					res = -99;
				}
			}
			else
			{
				cout << "cannot get sysid or side" << endl;
				res = -99;
			}
		}

	}
	else
	{
		cout << "Cannot get board id" << endl;
		res = -99;
	}

	ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	ACS_CS_API::deleteHWCInstance(hwcTable);
	return res;
}


int main(int, char**)
{
     
    try {
    	//cout << "uuid of cp1, side A" << endl;
    	getuuid(1, 0);
    	//cout << "\nuuid of cp1, side B" << endl;
    	//getuuid(1, 1);
    }		
	catch (...) {
		cout << "Unknown error" << endl;
		return 222;
	}
    
	return 0;
}

/*
2.1.3.4
ACS_CS_API_NS::CS_API_Result resultHWC;
resultHWC = hwcTable-> getSysNo(sysNo, boardId);


*/

