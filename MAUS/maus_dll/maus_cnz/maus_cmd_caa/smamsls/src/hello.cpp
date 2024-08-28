/*
Hello style of coding
*/

#include <iostream>
#include "ACS_CS_API.h"
using namespace std;

typedef ACS_CS_API_CommonBasedArchitecture Acs_ArchT;
typedef Acs_ArchT::ArchitectureValue ArchValueT;

int main(int, char**)
{
     
    try {
    	
    	ArchValueT infra;
    	ACS_CS_API_NS::CS_API_Result res = ACS_CS_API_NetworkElement::getNodeArchitecture(infra);
		if (ACS_CS_API_NS::Result_Success == res) {
			cout << "Node architecture is <" << infra << ">" << endl;
		}
		else {
			cout << "Error from CS. Error returned code is <" << res << ">" << endl;
		}

    }		
	catch (...) {
		cout << "Unknown error" << endl;
		return 222;
	}
	return 0;
}

