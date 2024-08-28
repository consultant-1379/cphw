/*
 * TermConn.h
 *
 *  Created on: Nov 4, 2015
 *      Author: xdtthng
 */

#ifndef TERMCONNECTION_H_
#define TERMCONNECTION_H_

#include <map>

// Set of terminal connections at run time
class TermConn
{
public:

	TermConn();
	~TermConn();

	int find(int cpId, int side);
	bool insert(int cpId, int side, int handle);
	void remove(int handle);

	size_t size() const;

private:
	std::map<int, int>	m_termConn;
};

inline
size_t TermConn::size() const
{
	return m_termConn.size();
}

#endif /* TERMCONNECTION_H_ */
