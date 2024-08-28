/*
 * ScoPinger.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: xdtthng
 */


#include <fcntl.h>
#include <errno.h>
//#include <sys/socket.h>
//#include <resolv.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <netinet/ip_icmp.h>

//#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include <sys/select.h>

#include <resolv.h>
#include <netdb.h>

#include <string>
#include <iostream>

#include "AppTrace.h"
#include "ScoPinger.h"

using namespace std;

#define PACKETSIZE  64
struct packet
{
    struct icmphdr hdr;
    char msg[PACKETSIZE-sizeof(struct icmphdr)];
};



string
ScoPinger::ipAddr(int index) const
{
	return m_ipAddress[index];
}

int
ScoPinger::error() const
{
	return m_error;
}

int
ScoPinger::replyCounter() const
{
	return m_echoReplyCount;
}

int
ScoPinger::index() const
{
	return m_index;
}

ScoPinger::ScoPinger(const std::string& ip1, const std::string& ip2): m_error(0), m_index(0), m_echoReplyCount(0)
{
	TRACE(("ScoPinger::ScoPinger()"));

	m_ipAddress[0] = ip1;
	m_ipAddress[1] = ip2;

	TRACE(("ScoPinger::ScoPinger() returns"));
}

int
ScoPinger::ping(int interface, int loop)
{
	TRACE(("ScoPinger::ping(%d, %d)", interface, loop));

	in_addr dst;
	m_index = interface;
	m_echoReplyCount = 0;
	int tsequence = 0;
	protoent *proto = NULL;

	TRACE(("ScoPinger::ping() to <%s>", m_ipAddress[m_index].c_str()));
	if (inet_aton(m_ipAddress[m_index].c_str(), &dst) == 0)
	{
		TRACE(("inet_aton() failed to convert ip addr index <%d> <%s>", m_index, m_ipAddress[m_index].c_str()));
		m_error = -1;
		return m_error;
	}
	else
	{
		//cout << "inet_aton; Address " << m_ipAddress[m_index] << " is ok" << endl;
	}

	int pid = getpid();

    const int val=255;

    struct packet pckt;
    struct sockaddr_in r_addr;

    struct sockaddr_in addr_ping,*addr;

    proto = getprotobyname("ICMP");
    bzero(&addr_ping, sizeof(addr_ping));
    addr_ping.sin_family = AF_INET;
    addr_ping.sin_port = 0;
    addr_ping.sin_addr = dst;


    addr = &addr_ping;

    int sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
    if ( sd < 0 )
    {
        m_error = -1;
        return -1;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        perror("Set TTL option");
        return 1;
    }
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
    {
    	m_error = -2;
        return -2;
    }

    m_error = 0;
    for (int idx = 0; idx < loop; ++idx)
    {

        memset(&pckt, 0, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = pid;
        unsigned int jdx;
        for ( jdx = 0; jdx < sizeof(pckt.msg)-1; jdx++ )
            pckt.msg[jdx] = jdx +'0';
        pckt.msg[jdx] = 0;
        pckt.hdr.un.echo.sequence = ++tsequence;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
        if (sendto(sd, &pckt, sizeof(pckt), 0, (sockaddr*)addr, sizeof(*addr)) <= 0 )
        {
            m_error = -1;
            break;
        }

        // Use select later on
        usleep(30000);

        int len=sizeof(r_addr);

        if (recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, (socklen_t*)&len) > 0 )
        {
            TRACE(("echo reply"));
        	++m_echoReplyCount;

        }
        else
        {
        	TRACE(("echo reply not received"));
            m_error = -1;
            //break;
        }
    }
	return m_error;


#if 0

	in_addr dst;
	m_index = interface;
	m_echoReplyCount = 0;

	TRACE(("ScoPinger::ping() to <%s>", m_ipAddress[m_index].c_str()));
	if (inet_aton(m_ipAddress[m_index].c_str(), &dst) == 0)
	{
		TRACE(("inet_aton() failed to convert ip addr index <%d> <%s>", m_index, m_ipAddress[m_index].c_str()));
		m_error = -1;
		return m_error;
	}
	else
	{
		//cout << "inet_aton; Address " << m_ipAddress[m_index] << " is ok" << endl;
	}

	icmphdr icmp_hdr;
	sockaddr_in addr;
	int sequence = 0;
	int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_ICMP);
	if (sock < 0)
	{
		TRACE(("failed here to get a socket"));
		m_error = -1;
		return m_error;
	}
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr = dst;

	memset(&icmp_hdr, 0, sizeof icmp_hdr);
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = 4231;


	unsigned char data[2048];
	for (int i = 0; i < loop; ++i)
	{
		int rc;

		timeval timeout;
		timeout.tv_sec = Wait_Seconds;
		timeout.tv_usec = Wait_MilliSeconds * 1000UL;

		fd_set read_set;
		socklen_t slen;
		icmphdr rcv_hdr;

		icmp_hdr.un.echo.sequence = sequence++;
		memcpy(data, &icmp_hdr, sizeof icmp_hdr);
		memcpy(data + sizeof icmp_hdr, m_payload, m_payloadLen); 	//icmp payload
		rc = sendto(sock, data, sizeof icmp_hdr + m_payloadLen,
				0, (struct sockaddr*)&addr, sizeof addr);
		if (rc <= 0)
		{
			TRACE(("sendto() failed"));
			m_error = -1;
			break;
		}

		memset(&read_set, 0, sizeof read_set);
		FD_SET(sock, &read_set);

		rc = select(sock + 1, &read_set, NULL, NULL, &timeout);
		if (rc == 0)
		{
			TRACE(("Timeout without reply"));
			m_error = -1;
			continue;
		}
		else if (rc < 0)
		{
			TRACE(("select error"));
			m_error = -1;
			break;
		}

		//we don't care about the sender address in this example..
		slen = 0;
		rc = recvfrom(sock, data, sizeof data, 0, NULL, &slen);
		if (rc <= 0)
		{
			TRACE(("recvfrom error"));
			m_error = -1;
			break;
		}
		else if (rc < (int)sizeof rcv_hdr)
		{

			TRACE(("Error, got short ICMP packet, <%d> bytes", rc));
			m_error = -1;
			break;
		}
		memcpy(&rcv_hdr, data, sizeof rcv_hdr);
		if (rcv_hdr.type == ICMP_ECHOREPLY)
		{
			TRACE(("ICMP Reply, id=0x%p, sequence=0x%p", icmp_hdr.un.echo.id, icmp_hdr.un.echo.sequence));
			++m_echoReplyCount;
		}
		else
		{
			TRACE(("Rx ICMP packet with type <0x%x>", rcv_hdr.type));
		}
		m_error = 0;
	}

#endif

}

int
ScoPinger::ping(int loop)
{
	//cout << "ScoPinger::ping()" << endl;
	int result = ping(0, loop);

	// If failed then try the other interface
	if (result != 0)
	{
		//cout << "ScoPinger::ping() failed on index 0, try index 1" << endl;
		result = ping(1, loop);
	}

	//cout << "ScoPinger::ping() restuls " << result << endl;
	return result;
}

unsigned short ScoPinger::checksum(void *b, int len)
{
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

