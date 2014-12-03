/*
 * EdRingQue.h
 *
 *  Created on: 2014. 1. 11.
 *      Author: netmind
 */

#ifndef EDRINGQUE_H_
#define EDRINGQUE_H_
#include "EdType.h"
#include "EdMutex.h"


namespace edft
{

class EdRingQue : public EdMutex
{
public:
	EdRingQue(int bufsize);
	virtual ~EdRingQue();
	int get(void *buf, int size);
	int peek(void* buf, int size);

	int peekNext(void* buf, int size);
	int put(const void *buf, int size);
	int getDataSize(void);
	int getFreeSize(void);

private:
	int readPos(void* buf, int size, int readpos, int datasize);
private:
	u8* mBuf;
	int mReadPos, mWritePos;
	int mBufSize;
	int mDataSize;
	int mLastPeekPos, mLastPeekDataSize;

};

} /* namespace edft */

#endif
