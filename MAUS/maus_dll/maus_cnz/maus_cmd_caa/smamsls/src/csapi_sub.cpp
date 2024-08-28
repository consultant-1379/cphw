#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include "csapi_sub.h"
using namespace std;

bool exit = false;

void CpTableChange_Observer::update (const ACS_CS_API_CpTableChange& observer)
{
//cout<<"update"<<endl;
ACS_CS_API_CpTableData *data = observer.cpData;

string type=getOperationValue(data->operationType);
cout << "operation type: " << type.c_str()<< endl;
cout << "MauType: " << getmauType(data->mauType) << endl;
    char *buff = new char[1024];
    size_t size = sizeof(buff);
    cout << "CPId: " << data->cpId << endl;
    data->cpName.getName(buff, size);
    cout << "cpName: " << buff << endl;

    data->cpAliasName.getName(buff, size);
    cout << "cpAliasname: " << buff << endl;

    cout << "apzSystem: " << data->apzSystem << endl;
    cout << "cpType: " << data->cpType << endl;
    cout << "cpState: " << data->cpState << endl;
    cout << "applicationId: " << data->applicationId << endl;
    cout << "apzSubstate: " << data->apzSubstate << endl;
    cout << "aptSubstate: " << data->aptSubstate << endl;
    cout << "stateTransition: " << data->stateTransition << endl;
    cout << "blockingInfo: " << data->blockingInfo << endl;
    cout << "cpCapacity: " << data->cpCapacity << endl;
    cout << "datasize: " << data->dataSize << endl;
//exit =true;
}
string getOperationValue(ACS_CS_API_TableChangeOperation::OpType operation)
{
        string ret = "UNKNOWN";

        switch(operation)
        {
        case ACS_CS_API_TableChangeOperation::Add:
                ret = "Add";
                break;
        case ACS_CS_API_TableChangeOperation::Change:
                ret = "Change";
                break;
        case ACS_CS_API_TableChangeOperation::Delete:
                ret = "Delete";
                break;
        case ACS_CS_API_TableChangeOperation::Unspecified:
                ret = "Unspecified";
                break;
        }

return ret;
}
string getmauType(uint16_t mauType)
{
        string ret = "UNDEFINED";

        switch(mauType)
        {
        case 1:
                ret = "MAUB";
                break;
        case 2:
                ret = "MAUS";
		break;
        }
return ret;
}
void cs_subscription_init()
{
    cout << "Getting instance of subscribe api" << endl;
    ACS_CS_API_SubscriptionMgr *instance = ACS_CS_API_SubscriptionMgr::getInstance();
    CpTableChange_Observer *cp = NULL;
    cp = new CpTableChange_Observer();
    if (instance->subscribeCpTableChanges(*cp) != ACS_CS_API_NS::Result_Success)
    {
        cout << "Could not register for CP table changes" << endl;
    }
	else
	cout << "subscription for cpTable change is succesfull" << endl;
}
int main(int argc, char *argv[])
{
cs_subscription_init();
while (!exit){
sleep (1);
}
}
