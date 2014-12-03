/*
 * EdLog.cpp
 *
 *  Created on: 2014. 2. 26.
 *      Author: netmind
 */

#include "EdLog.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace edft
{

EdLog::EdLog(int bsize) :
		mQue(bsize)
{

}

EdLog::~EdLog()
{
}

int EdLog::print(const char* str)
{

	u16 len = strlen(str) + 1;

	for (; mQue.getFreeSize() < (int)sizeof(len) + (int)len;)
	{
		int rdcnt;
		u16 slen;
		rdcnt = mQue.get(&slen, sizeof(slen));
		rdcnt = mQue.get(NULL, slen);
		if (rdcnt == 0)
			return -1;
	}

	mQue.put(&len, sizeof(len));
	mQue.put(str, len);

	return 0;
}

void EdLog::flush(int count)
{
	int i;
	int loop = (count <= 0) ? mQue.getDataSize() : count;
	u16 slen;
	int rdcnt;
	char *logstr = (char*) malloc(64 * 1024);
	for (i = 0; i < loop; i++)
	{
		rdcnt = mQue.get(&slen, sizeof(slen));
		if (rdcnt == 0)
			break;
		mQue.get(logstr, slen);
		puts(logstr);
	}
	free(logstr);
}

void EdLog::dumpAll(void)
{
	int rdcnt;
	u16 slen;
	char *logstr = (char*) malloc(64 * 1024);
	rdcnt = mQue.peek(&slen, sizeof(slen));
	rdcnt = mQue.peekNext(logstr, slen);
	for (; rdcnt;)
	{
		puts(logstr);
		rdcnt = mQue.peekNext(&slen, sizeof(slen));
		rdcnt = mQue.peekNext(logstr, slen);
	}
	free(logstr);
}

} /* namespace edft */
