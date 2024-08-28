/*
Hello style of coding
*/

#include <iostream>
#include <string>
#include "ACS_DSD_Client.h"
#include "ACS_DSD_Session.h"

using namespace std;


int main(int, char**)
{
     
    try {
    	
        ACS_DSD_Client  mClient;
        ACS_DSD_Session mSession;

        string mName = "CP1MAUM";
        string mDomain = "MAUS";

        std::vector<ACS_DSD_Node>  mReachableNodes;
        std::vector<ACS_DSD_Node>  mUnreachableNodes;

    	if(mClient.query(mName, mDomain, acs_dsd::SYSTEM_TYPE_AP, mReachableNodes, mUnreachableNodes) < 0)
    	{
    		cout << "failed to query for name <" << mName << "> domain <" << mDomain << ">" << endl;
            return 1;
    	}

    	cout << "Query for name <" << mName << "> domain <" << mDomain << "> ok" << endl;
    	cout << "number of reachable nodes are: " << mReachableNodes.size() << endl;
    	for (unsigned int i = 0; i < mReachableNodes.size(); ++i)
    	{
    		ACS_DSD_Node node = mReachableNodes[i];
    		cout << "For node element: " << i << endl;
    		cout << "system_id: " << node.system_id << endl;
    		cout << "node_name: " << node.node_name << endl;
    		cout << "node state: " << node. node_state << endl;
    		cout << "node side: " << node.node_side << endl;
    	}

#if 1
    	{
    		cout << "\nconnect to state = 1" << endl;
    	int mState = 1;

    	if (mClient.connect(mSession, mName, mDomain, 2001, acs_dsd::NodeStateConstants(mState)) < 0) {
    	//if (mClient.connect(mSession, mName, mDomain, 2001, acs_dsd::NODE_STATE_PASSIVE) < 0) {
    		cout << "failed to connect for name <" << mName << "> domain <" << mDomain << "> mState <"
    				<< mState << ">" << endl;
            return 1;
    	}

    	ACS_DSD_Node  node;
    	//mSession.get_local_node(node);
    	mSession.get_remote_node(node);

    	cout << "\nPrintout after connect" << endl;
		cout << "system_id: " << node.system_id << endl;
		cout << "node_name: " << node.node_name << endl;
		cout << "node state: " << node.node_state << endl;
		cout << "node side: " << node.node_side << endl;

    	}
#endif
    	{
    	int mState = 0;
		cout << "\nconnect to state = 0" << endl;

    	if (mClient.connect(mSession, mName, mDomain, 2001, acs_dsd::NodeStateConstants(mState)) < 0) {
    	//if (mClient.connect(mSession, mName, mDomain, 2001, acs_dsd::NODE_STATE_PASSIVE) < 0) {
    		cout << "failed to connect for name <" << mName << "> domain <" << mDomain << "> mState <"
    				<< mState << ">" << endl;
            return 1;
    	}

    	ACS_DSD_Node  node;
    	//mSession.get_local_node(node);
    	mSession.get_remote_node(node);

    	cout << "\nPrintout after connect" << endl;
		cout << "system_id: " << node.system_id << endl;
		cout << "node_name: " << node.node_name << endl;
		cout << "node state: " << node.node_state << endl;
		cout << "node side: " << node.node_side << endl;

    	}

    }		
	catch (...) {
		cout << "Unknown error" << endl;
		return 222;
	}
	return 0;
}

