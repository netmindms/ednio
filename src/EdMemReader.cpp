
/*
 * EdMemReader.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: netmind
 */
#define DBGTAG "MEMRD"
#define DBG_LEVEL DBG_WARN

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "edslog.h"
#include "EdMemReader.h"

using namespace std;

namespace edft
{

EdMemReader::EdMemReader()
{
	mBuf = nullptr;
	mReadCnt = 0;
	mSize = 0;
}

EdMemReader::~EdMemReader()
{
	dbgd("free _reader");
	if(mBuf!=nullptr)
	{
		free(mBuf);
	}
}


size_t EdMemReader::remain()
{
	return (mSize - mReadCnt);
}

size_t EdMemReader::read(char* buf, size_t bufsize)
{
	size_t rcnt = min(bufsize, remain());
	if (rcnt > 0)
	{
		memcpy(buf, mBuf + mReadCnt, rcnt);
		mReadCnt += rcnt;
	}
	return rcnt;
}


void EdMemReader::setBuffer(char *buf, size_t bsize)
{
	if(mBuf != nullptr)
	{
		free(mBuf);
	}
	mBuf = buf;
	mSize = bsize;
	mReadCnt = 0;
}


} /* namespace edft */


#if 0
#include <gtest/gtest.h>
using namespace edft;
using namespace testing;
TEST(edmemreader, 1)
{
	EdMemReader rd;
	char *buf = (char*)malloc(1024);
	strcpy(buf, "1234567890");
	rd.setBuffer(buf, strlen(buf)+1);
	char temp[3];
	size_t rcnt;
	string rs;

	rcnt = rd.read(temp, 4);
	ASSERT_EQ(rcnt, 4);
	rs.append(temp, rcnt);
	rcnt = rd.read(temp, 4);
	ASSERT_EQ(rcnt, 4);
	rs.append(temp, rcnt);
	rcnt = rd.read(temp, 4);
	ASSERT_EQ(rcnt, 3);
	rs.append(temp, rcnt);

	ASSERT_STREQ(rs.c_str(), buf);

}

#endif
