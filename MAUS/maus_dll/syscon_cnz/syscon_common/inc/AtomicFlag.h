/*
 * AtomicFlag.h
 *
 *  Created on: Sep 8, 2014
 *      Author: xdtthng
 */

#ifndef ATOMICFLAG_H_
#define ATOMICFLAG_H_

#include "boost/thread/mutex.hpp"
#include "AppTrace.h"

template<class TYPE>
class AtomicFlag
{
public:

	AtomicFlag(): m_flag(), m_cs()
	{
	}

	explicit AtomicFlag(const TYPE a): m_flag(a)
	{
	}

	~AtomicFlag()
	{
	}

	bool setFlag(const TYPE a);
	//void clearFlag();

	void operator()(const TYPE b);
	operator TYPE();

private:
	TYPE			m_flag;
	boost::mutex	m_cs;

	AtomicFlag(const AtomicFlag<TYPE>&);
	AtomicFlag<TYPE>& operator=(const AtomicFlag<TYPE>&);
};

template<class TYPE>
bool AtomicFlag<TYPE>::setFlag(const TYPE a)
{

	try {
		//boost::mutex::scoped_lock alock(m_cs);
		boost::lock_guard<boost::mutex> alock(m_cs);
		if (m_flag == a)
			return false;
		m_flag = a;
		return true;
	}
	catch (std::exception& e)
	{
		TRACE(("AtomicFlag::setFlag() mutex crashed with <%s>", e.what()));
		throw;
	}
}

#if 0
template<class TYPE>
void AtomicFlag<TYPE>::clearFlag()
{
	try {
		boost::mutex::scoped_lock alock(m_cs);
		m_flag = !m_flag;
	}
	catch (std::exception& e)
	{
		TRACE(("AtomicFlag::clearFlag() mutex crashed with <%s>", e.what()));
		throw;
	}
}
#endif

template<class TYPE>
void AtomicFlag<TYPE>::operator()(const TYPE b)
{

	try {
		//boost::mutex::scoped_lock lock(m_cs);
		boost::lock_guard<boost::mutex> alock(m_cs);
		m_flag = b;
	}
	catch (std::exception& e)
	{
		TRACE(("void AtomicFlag::operator() mutex crashed with <%s>", e.what()));
		throw;
	}
}

template<class TYPE>
AtomicFlag<TYPE>::operator TYPE()
{
	try {
		//boost::mutex::scoped_lock lock(m_cs);
		boost::lock_guard<boost::mutex> alock(m_cs);
		return m_flag;
	}
	catch (std::exception& e)
	{
		TRACE(("AtomicFlag::operator() TYPE() mutex crashed with <%s>", e.what()));
		throw;
	}
}

typedef AtomicFlag<bool>	BoolFlagT;


#endif // ATOMICFLAG_H_
