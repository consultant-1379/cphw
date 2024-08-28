#ifndef CSAPI_Subscription
#define CSAPI_Subscription

#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"


class CpTableChange_Observer : public ACS_CS_API_CpTableObserver
{
    virtual void update (const ACS_CS_API_CpTableChange& observer);
};

std::string getmauType(uint16_t mauType);
std::string getOperationValue(ACS_CS_API_TableChangeOperation::OpType operation);
extern CpTableChange_Observer *cp;

#endif // CSAPI_Subscription
