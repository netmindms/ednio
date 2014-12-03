/*
 * EdRingQue.cpp
 *
 *  Created on: 2014. 1. 11.
 *      Author: netmind
 */

#include <stdlib.h>
#include <string.h>

#include "EdRingQue.h"

namespace edft
{

EdRingQue::EdRingQue(int bufsize)
{
	mDataSize = 0;
	mBufSize = bufsize;
	mBuf = (u8*) malloc(bufsize);
	mReadPos = mWritePos = 0;
	mLastPeekPos = mLastPeekDataSize = 0;
	//loge("ringque, buf=%p", mBuf);
}

EdRingQue::~EdRingQue()
{
	//loge("~~~ringque, free=%p", mBuf);
	free(mBuf);
}

/*
 * param buf : buffer where read to
 * param readpos : index in buffer(mBuf)
** param datasize : max data size to read
*/
int EdRingQue::readPos(void* buf, int size, int readpos, int datasize)
{
	int rdsize = ESMIN(size, datasize);
	int retVal = rdsize;
	u8* ptr = (u8*) buf;

	int c = ESMIN(rdsize, mBufSize - readpos);
	if(buf)
		memcpy(ptr, mBuf + readpos, c);
	readpos = (readpos + c) % mBufSize;
	ptr += c;
	rdsize -= c;

	if (rdsize > 0)
	{
		if(buf)
			memcpy(ptr, mBuf, rdsize);
		readpos += rdsize;
	}
	return retVal;
}

int EdRingQue::get(void* buf, int size)
{
	int rdcnt = readPos(buf, size, mReadPos, mDataSize);
	mReadPos = (mReadPos + rdcnt) % mBufSize;
	mDataSize -= rdcnt;
	return rdcnt;
}

int EdRingQue::peek(void* buf, int size)
{
	int rdcnt = readPos(buf, size, mReadPos, mDataSize);
	mLastPeekPos = (mReadPos + rdcnt) % mBufSize;
	mLastPeekDataSize = mDataSize - rdcnt;
	return rdcnt;
}

int EdRingQue::peekNext(void* buf, int size)
{
	int rdcnt = readPos(buf, size, mLastPeekPos, mLastPeekDataSize);
	mLastPeekPos = (mLastPeekPos+rdcnt) % mBufSize;
	mLastPeekDataSize -= rdcnt;
	return rdcnt;
}

int EdRingQue::put(const void* buf, int size)
{
	u8 *ptr = (u8*) buf;
	int wsize = ESMIN(size, mBufSize - mDataSize);
	int retVal = wsize;
	int c = ESMIN(wsize, mBufSize - mWritePos);
	memcpy(mBuf + mWritePos, ptr, c);
	ptr += c;
	mWritePos = (mWritePos + c) % mBufSize;
	wsize -= c;
	if (wsize > 0)
	{
		memcpy(mBuf, ptr, wsize);
		mWritePos += wsize;
	}

	mDataSize += retVal;
	return retVal;
}

int EdRingQue::getDataSize(void)
{
	return mDataSize;
}

int EdRingQue::getFreeSize(void)
{
	return (mBufSize - mDataSize);
}

} /* namespace edft */
