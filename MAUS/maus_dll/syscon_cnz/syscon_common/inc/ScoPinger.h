/*
 * ScoPinger.h
 *
 *  Created on: Feb 23, 2016
 *      Author: xdtthng
 */

#ifndef SCOPINGER_H_
#define SCOPINGER_H_

#include <string>

class ScoPinger
{
public:
	enum N100
	{
		Wait_Seconds = 0,
		Wait_MilliSeconds = 30
	};

	ScoPinger(const char* ip1, const char* ip2);
	ScoPinger();
	void setIpAddr(const char* ip1, const char* ip2);
	int ping(int loop);
	int ping(int interface, int loop);

	int error() const;
	int index() const;
	int replyCounter() const;
	std::string ipAddr(int index) const;

private:
	int			m_error;
	int			m_index;
	int			m_echoReplyCount;

	std::string		m_ipAddress[2];

	unsigned short checksum(void *b, int len);

};



#endif /* SCOPINGER_H_ */

