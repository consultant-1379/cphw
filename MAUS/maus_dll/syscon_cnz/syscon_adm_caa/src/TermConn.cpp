/*
 * TermConnection.cpp
 *
 *  Created on: Nov 4, 2015
 *      Author: xdtthng
 */

#include "TermConn.h"
#include "Funx.h"
#include "AppTrace.h"

using namespace std;

TermConn::~TermConn()
{
}

TermConn::TermConn(): m_termConn()
{
}

int TermConn::find(int cpId, int side)
{
	map<int, int>::iterator it = m_termConn.find(Funx::pack32(cpId, side));

	return it != m_termConn.end()? it->second : -1;

}

bool TermConn::insert(int cpId, int side, int handle)
{
	TRACE(("TermConn::insert() cpId <%d> side <%d> handle <%d>", cpId, side, handle));

	TRACE(("before insert, m_termConn.size() <%d>", m_termConn.size()));
	unsigned int key = Funx::pack32(cpId, side);

	pair<map<int, int>::iterator, bool> res = m_termConn.insert(make_pair(key, handle));

	TRACE(("after insert, m_termConn.size() <%d> key <%d>", m_termConn.size(), key));

	TRACE(("TermConn::insert() returns <%d>", res.second));
	return res.second;
}

void TermConn::remove(int handle)
{
	TRACE(("TermConn::remove() handle <%d>", handle));

	for (map<int, int>::iterator it = m_termConn.begin(); it != m_termConn.end(); )
	{
		if (it->second == handle)
			m_termConn.erase(it++);
		else
			++it;
	}
	TRACE(("TermConn::remove() returns, m_termConn.size() <%d> ", m_termConn.size()));
}
