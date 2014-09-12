/*
 * EsHttpPaser.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: khkim
 */

#include <stdio.h>
#include <stdlib.h>
#include "EsHttpPaser.h"


EsHttpPaser::EsHttpPaser()
{
	mBuffer = NULL;
	mRcnt = mWcnt = 0;
	mBufSize = 8*1024;

}

EsHttpPaser::~EsHttpPaser()
{
	// TODO Auto-generated destructor stub
}

char* EsHttpPaser::getBuffer(int* size)
{
	if(mBuffer==NULL) {
		mBufSize = 8*1024;
		mBuffer = (char*)malloc(mBufSize);
		mRcnt = 0;
		mWcnt = 0;
	}

	int remain = mBufSize - mWcnt;
	if(remain>0)
	{
		*size = remain;
		return (mBuffer+mRcnt);
	}
	else
	{
		*size = 0;
		return NULL;
	}
}

void EsHttpPaser::consumeData(int size)
{
	mWcnt += size;
	int remain = mWcnt - mRcnt;
	char* ptr = mBuffer + mRcnt;
	if(mState == REQLINE_S)
	{
		REM_CHAR(ptr, ' ', remain);
		mElem = ptr;
		CON_CHAR(ptr, ' ', remain);
		if(*ptr != ' ') throw -100;
		httpVer.assign(mElem, ptr-mElem);
		REM_CHAR(ptr, ' ', remain);
		mElem = ptr;
		if(*ptr != ' ') throw -100;
		CON_CHAR(ptr, '\r', remain);
		if(*ptr != '\r') throw -100;
		statusCode.assign(mElem, ptr-mElem);
		REM_CHAR(ptr, ' ', remain);
		mElem = ptr;
		CON_CHAR(ptr, '\r', remain);
		if(*ptr != '\r') throw -100;
		respDesc.assign(mElem, ptr-mElem);
		REM_CHAR(ptr, '\n', remain);
	}
}

void EsHttpPaser::putData(char* str, int len)
{
	char* ptr = str;
	if(mState == REQLINE_S) {

	}

}

