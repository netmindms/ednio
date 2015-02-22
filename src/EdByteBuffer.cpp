/*
 * EdBuffer.cpp
 *
 *  Created on: Dec 31, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "BYBUF"

#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include "EdType.h"
#include "edslog.h"
#include "EdByteBuffer.h"

using namespace std;

namespace edft
{

EdByteBuffer::EdByteBuffer()
{
	mSize = 0;
	mBuf = nullptr;
	mCapacity = 0;
	mReadPos = 0;
	mErr = 0;
	mOrder = 0;
}

EdByteBuffer::~EdByteBuffer()
{
	if (mBuf != nullptr)
	{
		free(mBuf);
		mBuf = nullptr;
	}
}

EdByteBuffer::EdByteBuffer(EdByteBuffer&& _other) // move constructor
{
	*this = move(_other);
}

EdByteBuffer& EdByteBuffer::operator=(EdByteBuffer&& _other)
{
	if (this != &_other)
	{
		mSize = _other.mSize;
		mBuf = _other.mBuf;
		mReadPos = _other.mReadPos;
		mErr = _other.mErr;
		mCapacity = _other.mCapacity;
		mOrder = _other.mOrder;

		_other.mBuf = nullptr;
		_other.mSize = 0;
		_other.mReadPos = 0;
		_other.mErr = 0;
		_other.mCapacity = 0;
		_other.mOrder = 0;
	}
	return *this;
}

int EdByteBuffer::put(const void* buf, size_t size)
{
	if (mSize + size > mCapacity)
	{
		size_t t;
		if (mCapacity == 0)
			t = 4;
		else
			t = mCapacity;
		for (; t < mSize + size && t < 16 * 1024; t *= 2)
			;
		if (t < mSize + size)
		{
			t = mSize + size;
			t = (t / 128 + t % 128) * 128;
		}
		reserve(t);
	}
	memcpy(mBuf + mSize, buf, size);
	mSize += size;
	return mSize;
}

void EdByteBuffer::enlarge(size_t size)
{
	reserve(mCapacity + size);
}

void EdByteBuffer::reserve(size_t size)
{
	mBuf = (char*) realloc(mBuf, size);
	mCapacity = size;
}

size_t EdByteBuffer::capacity() const
{
	return mCapacity;
}

const char* EdByteBuffer::getBuffer() const
{
	return mBuf;
}

size_t EdByteBuffer::size() const
{
	return mSize;
}

void EdByteBuffer::rewind()
{
	mReadPos = 0;
}

size_t EdByteBuffer::remain() const
{
	return (mSize - mReadPos);
}

void EdByteBuffer::clear()
{
	mSize = 0;
	mReadPos = 0;
}

void EdByteBuffer::put8(u8 c)
{
	put(&c, 1);
}

void EdByteBuffer::put16(u16 c)
{
	if (mOrder == 0)
	{
		put(&c, 2);
	}
	else
	{
		u16 v = htons(c);
		put(&v, 2);
	}
}

void EdByteBuffer::put32(u32 c)
{
	if (mOrder == 0)
	{
		put(&c, 4);
	}
	else
	{
		u32 v = htonl(c);
		put(&v, 4);
	}
}

void EdByteBuffer::put64(u64 c)
{
	if (mOrder == 0)
	{
		put(&c, 8);
	}
	else
	{
		u8* ptr = (u8*) &c;
		u64 val = ((u64) ptr[0] << (8 * 7)) | ((u64) ptr[1] << (8 * 6)) |
				((u64) ptr[2] << (8 * 5)) | ((u64) ptr[3] << (8 * 4)) |
				((u64) ptr[4] << (8 * 3)) | ((u64) ptr[5] << (8 * 2)) |
				((u64) ptr[6] << (8 * 1)) | ((u64) ptr[7] << (8 * 0));
		put(&val, 8);
	}
}

char* EdByteBuffer::getCurReadPtr()
{
	return mBuf+mReadPos;
}

void EdByteBuffer::consume(size_t len)
{
	size_t c = min(len, mSize);
	mReadPos += c;
}

size_t EdByteBuffer::get(char* buf, size_t count)
{
	auto s = min(count, mSize - mReadPos);
	memcpy(buf, mBuf + mReadPos, s);
	mReadPos += s;
	return s;
}

upmChar EdByteBuffer::release()
{
	upmChar uptr(mBuf);
	mBuf = nullptr;
	mCapacity = 0;
	mSize = 0;
	mReadPos = 0;
	return uptr;
}

void EdByteBuffer::takeBuffer(char* ptr, size_t datasize, size_t cap)
{
	if (mBuf != nullptr)
	{
		free(mBuf);
	}
	mBuf = ptr;
	mReadPos = 0;
	mCapacity = cap;
	mSize = datasize;
}

int EdByteBuffer::getError() const
{
	return mErr;
}

u32 EdByteBuffer::get32()
{
	if (remain() >= 4)
	{
		if (mOrder == 0)
		{
			u32 ret = *((u32*) (mBuf + mReadPos));
			mReadPos += 4;
			return ret;
		}
		else
		{
			u8 *ptr = (u8*) (mBuf + mReadPos);
			u32 c;
			c = (ptr[0] << (8 * 3)) | (ptr[1] << (8 * 2)) | (ptr[2] << (8 * 1)) | (ptr[3] << (8 * 0));
			mReadPos += 4;
			return c;

		}
	}
	else
	{
		throw 1;
	}
}

u16 EdByteBuffer::get16()
{
	if (remain() >= 2)
	{
		if (mOrder == 0)
		{
			u16 ret = *((u16*) (mBuf + mReadPos));
			mReadPos += 2;
			return ret;
		}
		else
		{
			u8 *ptr = (u8*) (mBuf + mReadPos);
			u16 c;
			c = (ptr[0] << (8 * 1)) | (ptr[1] << (8 * 0));
			mReadPos += 2;
			return c;
		}
	}
	else
	{
		throw 1;
	}
}

u64 EdByteBuffer::get64()
{
	if (remain() >= 8)
	{
		if (mOrder == 0)
		{
			u64 ret = *((u64*) (mBuf + mReadPos));
			mReadPos += 8;
			return ret;
		}
		else
		{
			u8 *ptr = (u8*) (mBuf + mReadPos);
			u64 c;
			c = ((u64) ptr[0] << (8 * 7)) | ((u64) ptr[1] << (8 * 6)) |
					((u64) ptr[2] << (8 * 5)) | ((u64) ptr[3] << (8 * 4)) |
					((u64) ptr[4] << (8 * 3)) | ((u64) ptr[5] << (8 * 2)) |
					((u64) ptr[6] << (8 * 1)) | ((u64) ptr[7] << (8 * 0));
			mReadPos += 8;
			return c;

		}
	}
	else
	{
		throw 1;
	}
}

u8 EdByteBuffer::get8()
{
	if (mReadPos < mSize)
	{
		return (u8) (mBuf[mReadPos++]);
	}
	else
	{
		throw 1;
	}
}

void EdByteBuffer::setOrder(int order)
{
	mOrder = order;
}

} /* namespace edft */

#if 0
// test
#include <gtest/gtest.h>
using namespace edft;
TEST(edbytebuffer, 1)
{
	EdByteBuffer bf;
#pragma pack(1)
	struct _td
	{
		char c;
		u32 i1;
		u16 s;
		u32 i2;
		char buf[11];
		u64 l;
	};
#pragma pack()
	_td data;
	data.c = 0x10;
	data.i1 = 0x5050a0a0;
	data.s = 0xb0b0;
	data.i2 = 0x10203040;
	strcpy(data.buf, "0123456789");
	data.l = 0x1020304050607080;
	bf.put<u8>(0x10);
	bf.put<u32>(0x5050a0a0);
	bf.put<u16>(0xb0b0);
	bf.put<u32>(0x10203040);
	bf.put("0123456789", 11);
	bf.put<u64>(0x1020304050607080);

	// test move constructor
//	EdByteBuffer tbf;
//	memcpy(&tbf, &bf, sizeof(tbf));
	EdByteBuffer &&rrbf = move(bf);
//	EXPECT_EQ(0, memcmp(&tbf, &rrbf, sizeof(tbf)));
//	memset(&tbf, 0, sizeof(tbf));

	EXPECT_EQ(0x10, rrbf.get<u8>());
	EXPECT_EQ(0x5050a0a0, rrbf.get<u32>());
	EXPECT_EQ(0xb0b0, rrbf.get<u16>());
	EXPECT_EQ(0x10203040, rrbf.get<u32>());
	char rbuf[11];
	auto rcnt = rrbf.get(rbuf, 11);
	EXPECT_EQ(11, rcnt);
	EXPECT_STREQ("0123456789", rbuf);
	EXPECT_EQ(8, rrbf.remain());
	EXPECT_EQ(sizeof(data), bf.size());
	EXPECT_EQ(0, memcmp(&data, rrbf.getBuffer(), rrbf.size()));

	auto ptr = rrbf.getBuffer();
	auto uptr = rrbf.release();
	EXPECT_EQ(1, (ptr == uptr.get()));

	{	// host byte order put data
		bf.release();
		bf.put32(0x11223344);
		bf.put16(0x1122);
		bf.put64(0x1122334455667788);
		auto n32 = bf.get32();
		auto n16 = bf.get16();
		auto n64 = bf.get64();
		try
		{
			n32 = bf.get32();
			ASSERT_EQ(1, 0);
		} catch (int err)
		{
			dbgd("out_of_rage ...");
		}
		dbgd("n32=%x, n16=%x, n64=%lx", n32, n16, n64);
	}



	{	// network byte order put data
		bf.release();

		bf.setOrder(1);

		auto a8 = 0xaa;
		auto a16 = 0x1122;
		auto a32 = 0x11223344;
		auto a64 = 0x1122334455667788;

		bf.put32(a32);
		bf.put16(a16);
		bf.put8(a8);
		bf.put64(a64);

		ASSERT_EQ(bf.get32(), a32);
		ASSERT_EQ(bf.get16(), a16);
		ASSERT_EQ(bf.get8(), a8);
		ASSERT_EQ(bf.get64(), a64);
		try
		{
			auto b64 = bf.get64();
			ASSERT_EQ(1, 0);
		} catch (int err)
		{
			dbgd("out_of_rage ...");
		}

	}

}
#endif
