/*
 * EndpointView.h
 *
 *  Created on: Jul 5, 2014
 *      Author: xdtthng
 */

#ifndef ENDPOINTVIEW_H_
#define ENDPOINTVIEW_H_

#include <ostream>

class EndpointView
{
public:
	enum
	{
		AP_A,
		AP_B,
		CP_A,
		CP_B,
		MAX_ENDPOINT
	};
	static const char* s_endpointNames[];

	EndpointView(): m_eps() 	{};
	EndpointView(unsigned int v): m_eps(v) 	{};
	void view(unsigned v) 	{ m_eps = v; }

	static const char* getName(int);
	static void setSep(std::ios_base& os, long sep) { flag(os) = sep; }
	static long getSep(std::ios_base& os) { return flag(os); }

	friend std::ostream& operator<<(std::ostream& os, const EndpointView& v);

public:
	unsigned int	m_eps;

	static long& flag(std::ios_base& os)
	{
		static int n = std::ios_base::xalloc();
		return os.iword(n);
	}

};

class epsep
{
public:
	explicit epsep(unsigned int i = 0) : m_sep(i) { }
private:
	unsigned int m_sep;
	template <class charT, class Traits>
	friend std::basic_ostream<charT,Traits>& operator<<(std::basic_ostream<charT,Traits>& os, const epsep& w)
	{
		EndpointView::setSep(os, w.m_sep);
		return os;
	}
};


#endif // ENDPOINTVIEW_H_
