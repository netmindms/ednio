/*
 * EdSharedBufferReader.h
 *
 *  Created on: Sep 15, 2014
 *      Author: netmind
 */

#ifndef EDSHAREDBUFFERREADER_H_
#define EDSHAREDBUFFERREADER_H_

#include "../config.h"

#include <string.h>
#include <algorithm>
#include "EdHttpReader.h"

using namespace std;

namespace edft
{

class EdSharedBufferReader: public EdHttpReader
{
public:
	EdSharedBufferReader();
	virtual ~EdSharedBufferReader();
private:
	void* mBuf;
	long mReadCnt;
	long mSize;
public:
	EdSharedBufferReader(void* ptr, long cnt)
	{
		mSize = cnt;
		mBuf = ptr;
		mReadCnt = 0;
	}
	virtual long getSize()
	{
		return 0;
	}
	virtual long Read(void *buf, long len)
	{
		if (mReadCnt < mSize)
		{
			int rdcnt = min(len, mSize - mReadCnt);
			memcpy(buf, mBuf + mReadCnt, rdcnt);
			mReadCnt += rdcnt;
			return rdcnt;
		}
		else
		{
			return -1;
		}
	}
};

} /* namespace edft */

#endif /* EDSHAREDBUFFERREADER_H_ */
