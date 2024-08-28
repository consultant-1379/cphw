//#include <sys/poll.h>
#include <iostream>

#include "GMClient.h"
#include "maus_trace.h"

using namespace std;

static const int MSG_SIZE_IN_BYTES = 512;
static const int MAX_NO_OF_HANDLES = 64;
static const unsigned MAX_OF_TIMEOUT_MS = 3000;

GMClient::GMClient(const char* name, const char* domain, int state):
		mResultCode(RC_OK),
		mClient(), mSession(), mName(name), mDomain(domain),
		mState(state), mConn(false)
		//mGmThread(), mRunning(false)
{
    //newTRACE(("GMClient::GMClient() ctor", 0));
}

GMClient::~GMClient()
{
    if (mConn)
    	mSession.close();
}

bool GMClient::connect()
{
    
    std::vector<ACS_DSD_Node>  mReachableNodes;
    std::vector<ACS_DSD_Node>  mUnreachableNodes;

    //cout << "GMClient::connect() name <" << mName << "> domain <" << mDomain << ">" << endl;

	if(mClient.query(mName, mDomain, acs_dsd::SYSTEM_TYPE_AP, mReachableNodes, mUnreachableNodes) < 0)
	{
		//cout << "failed to query for name <" << mName << "> domain <" << mDomain << ">" << endl;
        mResultCode = RC_NOCONTACT;
        return mConn = false;
	}

#if 0
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

	cout << "number of unreachable nodes are: " << mUnreachableNodes.size() << endl;
	for (unsigned int i = 0; i < mUnreachableNodes.size(); ++i)
	{
		ACS_DSD_Node node = mUnreachableNodes[i];
		cout << "For node element: " << i << endl;
		cout << "system_id: " << node.system_id << endl;
		cout << "node_name: " << node.node_name << endl;
		cout << "node state: " << node. node_state << endl;
		cout << "node side: " << node.node_side << endl;
	}
#endif
	// acs_dsd::SYSTEM_ID_THIS_NODE
	// acs_dsd::SYSTEM_ID_PARTNER_NODE
	if (mClient.connect(mSession, mName, mDomain, 2001, acs_dsd::NodeStateConstants(mState)) < 0) {
	//if (mClient.connect(mSession, mName, mDomain, 2001, acs_dsd::NODE_STATE_PASSIVE) < 0) {
        mResultCode = RC_NOCONTACT;
        return mConn = false;
	}
    return  mConn = true;
}

ssize_t GMClient::send(void* buf, size_t size)
{
    ssize_t res = mSession.sendf(buf, size, MSG_NOSIGNAL);
    if (res < 0) {
        mResultCode = RC_INTERNAL_ERROR;
        return res;
    }
    
    // Reuse the buffer for reponse message
    memset(buf, 0, size);
    
    // Prepare to receive the return message
    if ((res = mSession.recv(buf, size, MAX_OF_TIMEOUT_MS)) < 0) {
        mResultCode = RC_INTERNAL_ERROR;
        return res;
    }
    mResultCode = RC_OK;
    return res;
}

