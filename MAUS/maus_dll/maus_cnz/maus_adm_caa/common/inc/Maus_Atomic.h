/*
 * Maus_Atomic.h
 *
 *  Created on: Sep 8, 2014
 *      Author: xdtthng
 */

#ifndef MAUS_ATOMIC_H_
#define MAUS_ATOMIC_H_

#include "boost/thread/mutex.hpp"
#include "maus_trace.h"

template<class TYPE>
class Maus_Atomic
{
public:

	Maus_Atomic(): m_value(), m_cs()
	{
	}

	explicit Maus_Atomic(const TYPE a): m_value(a)
	{
	}

	~Maus_Atomic()
	{
	}

	// Only prefix operator is implemented
	Maus_Atomic& operator++();
	Maus_Atomic& operator--();

	void operator()(const TYPE b);
	operator TYPE();

private:
	TYPE			m_value;
	boost::mutex	m_cs;

	Maus_Atomic(const Maus_Atomic<TYPE>&);
	Maus_Atomic<TYPE>& operator=(const Maus_Atomic<TYPE>&);
};

template<class TYPE>
Maus_Atomic<TYPE>& Maus_Atomic<TYPE>::operator++()
{

	try {
		boost::lock_guard<boost::mutex> alock(m_cs);
		++m_value;
		return *this;
	}
	catch (std::exception& e)
	{
		TRACE(("Maus_Atomic::operator++() mutex crashed with <%s>", e.what()));
		throw;
	}
}

template<class TYPE>
Maus_Atomic<TYPE>&  Maus_Atomic<TYPE>::operator--()
{

	try {
		boost::lock_guard<boost::mutex> alock(m_cs);
		--m_value;
		return *this;
	}
	catch (std::exception& e)
	{
		TRACE(("Maus_Atomic::operator--() mutex crashed with <%s>", e.what()));
		throw;
	}
}

template<class TYPE>
void Maus_Atomic<TYPE>::operator()(const TYPE b)
{

	try {
		boost::lock_guard<boost::mutex> alock(m_cs);
		m_value = b;
	}
	catch (std::exception& e)
	{
		TRACE(("void Maus_Atomic::operator() mutex crashed with <%s>", e.what()));
		throw;
	}
}

template<class TYPE>
Maus_Atomic<TYPE>::operator TYPE()
{
	try {
		boost::lock_guard<boost::mutex> alock(m_cs);
		return m_value;
	}
	catch (std::exception& e)
	{
		TRACE(("Maus_Atomic::operator() TYPE() mutex crashed with <%s>", e.what()));
		throw;
	}
}

typedef Maus_Atomic<int>	MausIntT;


#endif // MAUS_ATOMIC_H_
