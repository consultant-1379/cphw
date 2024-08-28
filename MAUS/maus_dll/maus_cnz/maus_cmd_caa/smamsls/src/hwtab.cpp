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

#include "CmdFunx.h"

using namespace std;

#define COMMAND_LEN 200
#define DATA_SIZE 512
 
void get_popen_data()
{
    FILE *pf;
    char command[COMMAND_LEN];
    char data[DATA_SIZE];
 
#if 0
    // Execute a process listing
    sprintf(command, "amf-adm lock safSi=MAUS-1,safApp=ERIC-MAUS >/dev/null 2>/dev/null"); 
    int res = system(command);
    cout << " return valued from system() res is " << res << endl;
    cout << "WIFEXITED (res) return " << WIFEXITED (res) << endl;
    cout << "WEXITSTATUS (res) return " << WEXITSTATUS(res) << endl;
    
#endif

    // Setup our pipe for reading and execute our command.
    sprintf(command, "amf-state siass | grep -i maus"); 
    pf = popen(command,"r"); 
 
    if(!pf){
      fprintf(stderr, "Could not open pipe for output.\n");
      return;
    }
 
    // Grab data from process execution
    //fgets(data, DATA_SIZE , pf);
    //void *pstr = data;
    //size_t count = fread(data, 1, DATA_SIZE, pf);
    //data[count] = '\0';
    //cout << "data read is " << count << endl;
    //cout << data << endl;
    
    //cout << "start priting" << endl;
    //while (fgets(data, DATA_SIZE , pf))
    //    fprintf(stdout, "%s",data);
        //puts(data);
  
    int count  = 0;  
    while (!feof(pf)) {
        if (fgets(data, DATA_SIZE , pf)) {
            fprintf(stdout, "%s",data);
            ++count;
        }
    }
 
    cout << "Number of lines return is " << count << endl;
    // Print grabbed data to the screen.
    //fprintf(stdout, "%s\n",data); 
 
    if (pclose(pf) != 0)
        fprintf(stderr," Error: Failed to close command stream \n");

    return;
}

void test(int fbn)
{
	ACS_CS_API_HWC* hwcTable = ACS_CS_API::createHWCInstance();
    if (hwcTable)
    {
       ACS_CS_API_BoardSearch* boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
       if (boardSearch)
       {
          //boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_MAUB);
          boardSearch->setFBN(fbn);
          ACS_CS_API_IdList boardList;
          unsigned short int systype, sysno, fbn;
          if (hwcTable->getBoardIds(boardList, boardSearch) == ACS_CS_API_NS::Result_Success)
          {
           	  for (unsigned int i = 0; i < boardList.size(); ++i)
           	  {
           		  //cout << boardList[i].getSysType() << " " << boardList[i].getSysNo() << " "<< boardList[i].getFBN() << endl;
           		hwcTable->getSysType(systype, boardList[i]);
           		hwcTable->getSysNo(sysno, boardList[i]);
           		hwcTable->getFBN(fbn, boardList[i]);

           		cout << systype << " " << sysno << " " << fbn << endl;
           	  }
          }
          else
          {
        	  cout << "Cannot get the board list" << endl;
          }
       }
       else
       {
    	   cout << "Cannot search for board" << endl;
       }

    }
    else
    {
    	cout << "Cannot get hwcTable" << endl;
    }
}

void test()
{
    OmHandler immHandle;
    std::vector<std::string> dnList;

    ACS_CC_ReturnType ret = immHandle.Init();
    if (ret != ACS_CC_SUCCESS)
    {
    	cout << "cannot initialize OmHandler" << endl;
       return;
    }
    ret = immHandle.getClassInstances("AxeFunctions", dnList);
    if (ret != ACS_CC_SUCCESS)
    {
    	cout << "cannot initialize OmHandler" << endl;
       return;
    }
    for (unsigned int i = 0; i < dnList.size(); ++i)
    {
    	cout << "dnList[" << i << "] = " << dnList[i] << endl;
    }
}

int get_attribute()
{
    OmHandler omHandler;
    ACS_CC_ReturnType result = omHandler.Init();
    
    if (result != ACS_CC_SUCCESS) {
        cout << "cannot start object handler" << endl;
        return -1;
    }
    
    ACS_APGCC_ImmAttribute admin;
    admin.attrName = "saAmfSIAdminState";
    
    vector<ACS_APGCC_ImmAttribute *> attributes;
    attributes.push_back(&admin);
    
    string dn = "safSi=MAUS-1,safApp=ERIC-MAUS";
    result = omHandler.getAttribute(dn.c_str(), attributes);
    if (result != ACS_CC_SUCCESS)
    {
        cout << "there is something wrong" << endl;
        return -1;
    }
    
    int adminstate = *((int*)admin.attrValues[0]);
    cout << "admin state of MAUS-1 is " << adminstate << endl;
    
    return 0;
}

int get_mauType()
{
    OmHandler omHandler;
    ACS_CC_ReturnType result = omHandler.Init();

    if (result != ACS_CC_SUCCESS) {
        cout << "cannot start object handler" << endl;
        return -1;
    }

    ACS_APGCC_ImmAttribute mauTypeAttr;
    mauTypeAttr.attrName = "mauType";
    vector<ACS_APGCC_ImmAttribute *> attributes;
    attributes.push_back(&mauTypeAttr);

    string dn = "dualSidedCpId=CP1,logicalMgmtId=1,AxeEquipmentequipmentMId=1";

    result = omHandler.getAttribute(dn.c_str(), attributes);
    if (result != ACS_CC_SUCCESS)
    {
        cout << "there is something wrong" << endl;
        return -1;
    }

    int mautype = *((int*)mauTypeAttr.attrValues[0]);
    cout << "mau type is " << mautype << endl;

	return mautype;
}


int testsusi()
{
	using namespace CmdFunx;
	int su[2];
	const string sname = "MAUS";
	int size = getAmfSuSiAssignment(su, sname);

	if (size < 0)
	{
		cout << "Error in amf" << endl;
		return -1;
	}

	cout << "from the calling site" << endl;
	cout << "su[0] is " << su[0] << " su[1] is " << su[1] << endl;
	cout << "size is " << size << endl;
	cout << endl;

	const char* apNames[] = {"AP-A", "AP-B"};

	int isu = 0;
	for (int i = 0; i < 2; ++i)
	{
		isu = i;
		if (su[i])
		{
			cout << "su is " << ++isu << " node " << apNames[i] << " si is " << su[i] << endl;
		}
	}

	return 0;
}
int main(int, char**)
{
     
    try {
    
        //get_attribute();
        //get_popen_data();
    	//testsusi();
    	get_mauType();
#if 0
    	//test();
    	//ACS_CS_API_NS::CS_API_Result getDoubleSidedCPCount(unsigned int&cpCount);
    	//ACS_CS_API_NS::CS_API_Result result;
    	//ACS_CS_API_CP* cpTable = ACS_CS_API::createCPInstance();
    	unsigned int cpCount;
    	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getDoubleSidedCPCount(cpCount);
    	if (ACS_CS_API_NS::Result_Success == res)
    	{
    	   cout << "Number of double sided CP is " << cpCount << endl;
    	}
    	else 
    	{
    	   cout << "Error, cannot get number of double sided CP count" << endl;
    	}
    	
    	cout << "Print out all MAUBs in the system" << endl;
    	test(ACS_CS_API_HWC_NS::FBN_MAUB);
    	cout << endl;
    	
    	
     	cout << "Print out all CPUBs in the system" << endl;
    	test(ACS_CS_API_HWC_NS::FBN_CPUB);
    	cout << endl;
#endif  	
    }		
	catch (...) {
		cout << "Unknown error" << endl;
		return 222;
	}
    
  	//cout << "End of Test" << endl;	
	return 0;
}

/*
2.1.3.4
ACS_CS_API_NS::CS_API_Result resultHWC;
resultHWC = hwcTable-> getSysNo(sysNo, boardId);


*/

