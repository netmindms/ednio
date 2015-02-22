/*
 * EdBuffer.h
 *
 *  Created on: Dec 31, 2014
 *      Author: netmind
 */

#ifndef EXTERNAL_EDNIO_EDBUFFER_H_
#define EXTERNAL_EDNIO_EDBUFFER_H_

#include "EdType.h"
#include <memory>
#include <vector>
#include <functional>

using namespace std;

namespace edft
{


class EdByteBuffer
{
private:
	size_t mSize;
	size_t mCapacity;
	size_t mReadPos;
	char* mBuf;
	int mErr;
	bool mOrder;

public:
	EdByteBuffer();
	virtual ~EdByteBuffer();

	EdByteBuffer(EdByteBuffer&& _other);	// move constructor
	EdByteBuffer& operator=(EdByteBuffer&& _other); // move assign operator


	void put8(u8 c);
	void put16(u16 c);
	void put32(u32 c);
	void put64(u64 c);

	u8 get8();
	u16 get16();
	u32 get32();
	u64 get64();

	template<typename T> T get()
	{
		if (remain() < sizeof(T))
		{
			mErr = -1;
			T t;
			memset(&t, 0, sizeof(T));
			return t;
		}
		auto oldidx = mReadPos;
		mReadPos += sizeof(T);
		mErr = 0;
		return *((T*) (mBuf + oldidx));
	}

	void takeBuffer(char* ptr, size_t datasize, size_t cap);
	void enlarge(size_t size);
	void reserve(size_t size);
	void rewind();
	size_t remain() const;
	size_t capacity() const;
	const char* getBuffer() const;
	size_t size() const;
	upmChar release();
	int getError() const;
	void setOrder(int order);
	void clear();
	char* getCurReadPtr();
	void consume(size_t len);

	int put(const void* buf, size_t size);
	template<typename T> int put(T val)
	{
		return put(&val, sizeof(val));
	}

	size_t get(char* buf, size_t);

	template<typename T> T getNum()
	{
		if (remain() >= sizeof(T))
		{
			T ret=0;
			char* ptr = mBuf + mReadPos;
			if (mOrder == 0)
			{
				ret = *((T*) (mBuf + mReadPos));
				mReadPos += sizeof(T);
				return ret;
			}
			else
			{
				int bn = sizeof(T);
				for (int i = 0; i < bn; i++)
				{
					ret |= ((T)(ptr[i])) << (bn - 1 - i) * 8;
				}
				mReadPos += sizeof(T);
				return ret;
			}
		}
		else
		{
			throw 1;
		}
	}

	template<typename T> void putNum(T val) {
		 if(mOrder==0) {
			 put(&val, sizeof(T));
		 } else {
			 int bn = sizeof(T);
			 T ret=0;
			 unsigned char *ptr = (unsigned char*)(&val);
			 for(int i=0;i<bn;i++)
			 {
				 ret |= ((T)ptr[i]) << (bn -1-i)*8;
			 }
			 put(&ret, sizeof(T));
		 }
	}


};

} /* namespace edft */

#endif /* EXTERNAL_EDNIO_EDBUFFER_H_ */
