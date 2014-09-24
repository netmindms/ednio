/*
 * EdHttpStringReader.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */
#define DBGTAG "sread"
#define DBG_LEVEL DBG_DEBUG

#include "../config.h"

#include <algorithm>
#include <string.h>
#include <assert.h>
#include "../edslog.h"
#include "EdHttpStringReader.h"

namespace edft
{

EdHttpStringReader::EdHttpStringReader()
{
	// TODO Auto-generated constructor stub
	mReadIdx = mSize = 0;
	mBuf = NULL;
	mTakenString = NULL;
}

EdHttpStringReader::~EdHttpStringReader()
{
	// TODO Auto-generated destructor stub
	if(mTakenString == NULL)
	{
		if(mBuf != NULL)
		{
			free(mBuf);
		}
	}
	else
	{
		delete mTakenString;
	}
}

long EdHttpStringReader::Read(void* buf, long len)
{
	if (mReadIdx < mSize)
	{
		int rdcnt = min(len, mSize - mReadIdx);
		memcpy(buf, (char*)mBuf+mReadIdx, rdcnt);
		mReadIdx += rdcnt;
		return rdcnt;
	}
	else
	{
		return -1;
	}
}

long EdHttpStringReader::getSize()
{
	return mSize;
}

void EdHttpStringReader::setString(string* s)
{
	setString(s->c_str());
}

void EdHttpStringReader::setString(const char* ptr)
{
	if (mBuf != NULL)
	{
		free(mBuf);
	}
	mReadIdx = 0;
	mSize = strlen(ptr);
	mBuf = malloc(mSize);
	if (mBuf == NULL)
	{
		dbge("### Fail: memory allocation error for string reader, size=%ld", mSize);
		mSize = 0;
		assert(0);
	}
	else
	{
		memcpy(mBuf, ptr, mSize);
	}
}


void EdHttpStringReader::takeString(string* s)
{
	mTakenString = s;
	mReadIdx = 0;
	mBuf = (char*)(s->c_str());
	mSize = s->size();
}

} /* namespace edft */
