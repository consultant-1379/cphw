/*
 * main_client.cpp
 *
 *  Created on: Jul 5, 2015
 *      Author: xdtthng
 */

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "boost/shared_ptr.hpp"

#include "CpMauMsg.h"

using namespace std;


using boost::asio::ip::tcp;

int parseCmdLine(int argc, char **argv, string& name, int& fexVal, int& action, unsigned int& seqno);
static const string MAUS1_SRV_NAME = "maus1";
static const string MAUS2_SRV_NAME = "maus2";
static const string MAUS3_SRV_NAME = "maus3";
static const string PORT = "22222";

int help(int = 0);

class MAClient
{
public:
    enum {
        DEFAULT_PORT = 22222,
        DEFAULT_BUFFER = 512
    };

    MAClient(boost::asio::io_service& ioService, tcp::resolver::iterator it);
    ~MAClient();
    void send(char *, size_t);
    void recv(char *, size_t);
    bool connected() const;

private:

    boost::asio::io_service&    mIOService;
    tcp::socket                 mSocket;
    bool                        mConnected;

    size_t          mBufSize;
    char            *mRecvBuf;

};

MAClient::MAClient(boost::asio::io_service& ioService, tcp::resolver::iterator it) : mIOService(ioService),
            mSocket(ioService)
{

    //cout << "MAClient::MAClient() ctor" << endl;
    mSocket.connect(*it);
    //cout << "MAClient::MAClient() ctor returns" << endl;
}

MAClient::~MAClient()
{
	//cout << "MAClient::~MAClient() dtor" << endl;
}

bool MAClient::connected() const
{
	return mSocket.is_open();
}

void MAClient::send(char* buf, size_t size)
{
	boost::asio::write(mSocket, boost::asio::buffer(buf, size));
}

void MAClient::recv(char* buf, size_t size)
{
    size_t recSize = boost::asio::read(mSocket, boost::asio::buffer(buf, size));
    if (recSize == 0) {
    	// Todo something here
    }
}

class FexClient
{
public:

	enum CmdAction
	{
		Help,
		GetFex,
		SetFex,
		ClearFex,	// similar to SetFex, for conveninence
		GetAdminState,
		GetApgState,
		MaxCmdAction
	};

	FexClient(int instance);
	~FexClient()	{}

	int getFex(int&, unsigned int);
	int setFex(int&, unsigned int);
	int clearFex(int&, unsigned int);
	int getAdminState(int&, unsigned int);
	int getApgState(int&, unsigned int);

	int exec(int action, int& fexVal, unsigned int seqno)
	{
		return (this->*m_cmds[action])(fexVal, seqno);
	}

private:
	int		m_instance;
	bool	m_connected;

	boost::scoped_ptr<MAClient> 	m_client;
	boost::asio::io_service 		m_ioService;
	tcp::resolver 					m_resolver;

	typedef int (FexClient::*CmdT)(int&, unsigned int);

	CmdT		m_cmds[MaxCmdAction];

	static const char* IP_LISTENING_ADDRESS[4];

};

const char* FexClient::IP_LISTENING_ADDRESS[4] = {
    "192.168.169.1",
    "192.168.169.2",
    "192.168.170.1",
    "192.168.170.2"
};

FexClient::FexClient(int instance): m_instance(instance),
	m_connected(false),
	m_client(),
	m_ioService(),
	m_resolver(m_ioService)
{

	m_cmds[0] = &FexClient::getFex;
	m_cmds[GetFex] = &FexClient::getFex;
	m_cmds[SetFex] = &FexClient::setFex;
	m_cmds[ClearFex] = &FexClient::clearFex;
	m_cmds[GetAdminState] = &FexClient::getAdminState;
	m_cmds[GetApgState] = &FexClient::getApgState;

	for(int i = 0; i < 4; ++i)
	{
		tcp::resolver::query query(tcp::v4(), IP_LISTENING_ADDRESS[i], PORT);
		tcp::resolver::iterator endpoint = m_resolver.resolve(query);
		try
		{
			m_client.reset(new MAClient(m_ioService, endpoint));
			if (m_client->connected())
			{
				m_connected = true;
				break;
			}
		}
		catch (...) {}
	}
}

int FexClient::getApgState(int& adminState, unsigned int seqno)
{
	if (!m_connected)
		return 1;

	char buffer[512];
	new (buffer) CpMauMsg::GetApgStateReqMsg(seqno, 1, 0);
	CpMauMsg::GetApgStateReqMsg* reqMsg = reinterpret_cast<CpMauMsg::GetApgStateReqMsg*>(buffer);
	cout << *reqMsg << endl;

	m_client->send(buffer, 512);
	m_client->recv(buffer, 512);
	CpMauMsg::GetApgStateRspMsg* msg = reinterpret_cast<CpMauMsg::GetApgStateRspMsg*>(buffer);
	cout << *msg << endl;
	adminState = msg->apaState();

	return 0;
}

int FexClient::getAdminState(int& adminState, unsigned int seqno)
{
	if (!m_connected)
		return 1;

	char buffer[512];
	new (buffer) CpMauMsg::GetAdminStateReqMsg(m_instance, seqno, 1, 0);
	CpMauMsg::GetAdminStateReqMsg* reqMsg = reinterpret_cast<CpMauMsg::GetAdminStateReqMsg*>(buffer);
	cout << *reqMsg << endl;

	m_client->send(buffer, 512);
	m_client->recv(buffer, 512);
	CpMauMsg::GetAdminStateRspMsg* msg = reinterpret_cast<CpMauMsg::GetAdminStateRspMsg*>(buffer);
	cout << *msg << endl;
	adminState = msg->adminState();

	return 0;
}



int FexClient::getFex(int& fex, unsigned int seqno)
{
	if (!m_connected)
		return 1;

	char buffer[512];
	new (buffer) CpMauMsg::GetFexReqMsg(m_instance, seqno, 1, 0);
	CpMauMsg::GetFexReqMsg* reqMsg = reinterpret_cast<CpMauMsg::GetFexReqMsg*>(buffer);
	cout << *reqMsg << endl;

	m_client->send(buffer, 512);
	m_client->recv(buffer, 512);
	CpMauMsg::GetFexRspMsg* msg = reinterpret_cast<CpMauMsg::GetFexRspMsg*>(buffer);
	cout << *msg << endl;
	fex = msg->fexReqState();

	return 0;
}

int FexClient::setFex(int& fex, unsigned int seqno)
{
	if (!m_connected)
		return 1;

	//cout << "fex to set is " << fex << endl;

	char buffer[512]; // SetFexReqMsg
	new (buffer) CpMauMsg::SetFexReqMsg(m_instance, fex, seqno, 1, 0);
	CpMauMsg::SetFexReqMsg* reqMsg = reinterpret_cast<CpMauMsg::SetFexReqMsg*>(buffer);
	cout << *reqMsg << endl;

	m_client->send(buffer, 512);
	m_client->recv(buffer, 512);
	CpMauMsg::SetFexRspMsg* msg = reinterpret_cast<CpMauMsg::SetFexRspMsg*>(buffer);
	cout << *msg << endl;

	return 0;
}

int FexClient::clearFex(int& fex, unsigned int seqno)
{
	fex = 0;
	return setFex(fex, seqno);
}

int parseCmdLine(int argc, char **argv, string& name, int& fexVal, int& action, unsigned int& seqno)
{
	name = "";
	fexVal = 0;
	action = 0;
	seqno = 1234;

	opterr = 0;
	int opt = 0;

	string srvName = "";
	char *nStr = 0;
	bool helpOpt = false;
	char* transStr = 0;
	while ((opt = getopt(argc, argv,"as:f:hnrt:z")) != -1)
	{
		switch (opt)
		{
		case 's':
			srvName = optarg;
			break;
		case 'a':
			action = FexClient::GetAdminState;
			break;
		case 'f':
			nStr = optarg;
			action = FexClient::SetFex;
			break;
		case 'h':
			helpOpt = true;
			break;
		case 'r':
			action = FexClient::GetFex;
			break;
		case 'n':
			action = FexClient::GetApgState;
			break;
		case 't':
			transStr = optarg;
			break;
		case 'z':
			action = FexClient::ClearFex;
			break;
		default:
			helpOpt = true;
			break;
		}
	}

	transform(srvName.begin(), srvName.end(), srvName.begin(),(int (*)(int)) ::tolower);
	if (helpOpt || srvName.length() == 0 ||
	   (srvName != MAUS1_SRV_NAME && srvName != MAUS2_SRV_NAME && srvName != MAUS3_SRV_NAME) )
	{
		help();
		return EXIT_FAILURE;
	}

	long int nVal = 0L;
	if (action == FexClient::SetFex)
	{
		char* endptr = 0;
		nVal = strtol(nStr, &endptr, 10);
		if ((endptr[0])) {
			cout << "Inavlid value <" << nStr << "> for option <n>" << endl;
			return EXIT_FAILURE;
		}
	}

	long int seqVal = 1234L;
	if (transStr)
	{
		char* endptr = 0;
		seqVal = strtol(transStr, &endptr, 10);
		if ((endptr[0])) {
			cout << "Inavlid value <" << transStr << "> for option <t>" << endl;
			return EXIT_FAILURE;
		}
	}

    name = srvName;
    fexVal = nVal;
    seqno = seqVal;

	return 0;

}

int main(int argc, char* argv[])
{
	// Parsing command line
	string serviceName;
	int fexVal = 0;
	int action = 0;
	unsigned int seqno;
	int result = parseCmdLine(argc, argv, serviceName, fexVal, action, seqno);

	if (result)
		return result;

	// serviceName can only be MAUS1 or MAUS2
	int instance = serviceName == MAUS1_SRV_NAME? 1 :
				serviceName == MAUS2_SRV_NAME? 2 : 3;

	//cout << "Action is " << action << endl;
	//cout << "instance is " << instance << endl;
	//cout << "fexVal is " << fexVal << endl;
	//cout << "seq no is " << seqno << endl;

	if (action < FexClient::GetFex && action > FexClient::GetApgState)
	{
		cout << "Option not implemented yet" << endl;
		return 0;
	}

	FexClient fex(instance);
	fex.exec(action, fexVal, seqno);

	return 0;
}

int help(int)
{
	cout << "Usage:" << endl;
	cout << endl;
	cout << "fexclient -s <service_name> [-f <fex_value>] [-r ] [-z] [-h] [-t <seqno>] " << endl;
	cout << endl;
	cout << "<service_name>:   MAUS1 or MAUS2" << endl;
	cout << "<fex_value>:      fex value" << endl;
	cout << "<seqno>:          seq number used by client" << endl;
	cout << endl;
	cout << "-r read fex value" << endl;
	cout << "-z clear fex value" << endl;
	return 0;
}
