/*
 * ScoCpId.h
 *
 *  Created on: Nov 5, 2015
 *      Author: xdtthng
 */

#ifndef GEN_FUNX_H_
#define GEN_FUNX_H_

namespace Funx
{
	unsigned int pack32(int cpid, int side);
	void unpack32(unsigned int composite, int& cpid, int& side);

};


inline
unsigned int Funx::pack32(int w0, int w1)
{
	return (w1 << 16) | w0;
}

inline
void Funx::unpack32(unsigned int composite, int& w0, int& w1)
{
	w0 = composite;
	w0 &= 0xffff;
	w1 = composite >> 16;
	w1 &= 0xffff;
}


#endif /* SCOCPID_H_ */
