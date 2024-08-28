/*
 * EndpointView.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: xdtthng
 */

#include "EndpointView.h"

using namespace std;

const char* EndpointView::s_endpointNames[] = {
	"AP-A", "AP-B", "CP-A", "CP-B", "None"
};

const char* EndpointView::getName(int m)
{
	unsigned int n = static_cast<unsigned int>(m);
	return n < MAX_ENDPOINT ? s_endpointNames[n] : s_endpointNames[MAX_ENDPOINT];
}

ostream& operator<<(std::ostream& os, const EndpointView& v)
{
	long sep = EndpointView::getSep(os);
	string sepStr = " ";
	if (sep != 0 && static_cast<char>(sep) != ' ')
	{
		sepStr = string(1, static_cast<char>(sep));
		sepStr += " ";
	}

	bool first = true;
	for (size_t i = 0; i < EndpointView::MAX_ENDPOINT; ++i)
	{
		if ((v.m_eps & 1 << i) == 0)
			continue;
		if (first)
		{
			first = false;
		}
		else
		{
			os << sepStr << flush;
		}
		os << EndpointView::s_endpointNames[i] << flush;
	}
	return os;
}
