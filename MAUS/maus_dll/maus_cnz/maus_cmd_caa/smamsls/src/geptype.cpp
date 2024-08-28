/*
NAME
*/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <vector>

#include "boost/asio.hpp"
//#include "boost/asio/ip/address_v4.hpp"

#include "ACS_CS_API.h"
#include <acs_apgcc_paramhandling.h>
#include <ACS_APGCC_CommonLib.h>

using namespace std;

int getGepType()
{

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

	int res = -99;
	ACS_CS_API_IdList boardList;
	if (hwcTable->getBoardIds(boardList, boardSearch) == ACS_CS_API_NS::Result_Success)
	{
		uint32_t bgciIpEthA;
		uint32_t bgciIpEthB;
		uint32_t magazine;
		unsigned short sysId, side, sysType;
		cout << "boardlist size " << boardList.size() << endl;

		for (unsigned int i = 0; i < boardList.size(); ++i)
		{
			if (hwcTable->getSysId(sysId, boardList[i]) == ACS_CS_API_NS::Result_Success)
			{
				if (!hwcTable->getBgciIPEthA(bgciIpEthA, boardList[i]) == ACS_CS_API_NS::Result_Success)
				{
					cout << "Cannot get bgciIpEthA" << endl;
					res = -99;
				}
				else if (!hwcTable->getBgciIPEthB(bgciIpEthB, boardList[i]) == ACS_CS_API_NS::Result_Success)
				{

					cout << "Can get bgciIpEthA; Cannot get bgciIpEthB" << endl;
					res = -99;
				}
				else
				{
					boost::asio::ip::address_v4 ipa(bgciIpEthA);
					boost::asio::ip::address_v4 ipb(bgciIpEthB);

					if (!hwcTable->getSide(side, boardList[i]) == ACS_CS_API_NS::Result_Success)
					{
						side = 0;
					}

					if (!hwcTable->getSysType(sysType, boardList[i]) == ACS_CS_API_NS::Result_Success)
					{
						sysType = 0xffff;
					}

					if (!hwcTable->getMagazine(magazine, boardList[i]) == ACS_CS_API_NS::Result_Success)
					{
						magazine = 0xffffffff;
					}

					cout << "side: " << side
							<< " magazine 0x" << hex << setw(8) << magazine << dec
							<< " sysType " << setw(4) << sysType
							<< " sysId " << setw(4) << sysId
							<< " bgciIpEthA " << ipa.to_string()
							<< " bgciIpEthB " << ipb.to_string()
							<< endl;

					res = 0;
				}
			}
			else
			{
				cout << "cannot get sysid" << endl;
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
    	getGepType();
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

