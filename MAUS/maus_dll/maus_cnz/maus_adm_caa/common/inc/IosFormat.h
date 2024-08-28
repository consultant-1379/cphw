/*
 * IosFormat.h
 *
 *  Created on: Jul 6, 2014
 *      Author: xdtthng
 */

#ifndef IOSFORMAT_H_
#define IOSFORMAT_H_

class IosFormat
{
public:
	IosFormat(std::ios& io) : m_Io(io), m_Flg(io.flags()) { }
    ~iosFormat() { mIo.flags(mFlg); }
private:
    std::ios&            m_Io;
    std::ios::fmtflags   m_Flg;
};

#endif // IOSFORMAT_H_
