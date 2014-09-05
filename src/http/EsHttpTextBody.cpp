/*
 * EsHttpTextBody.cpp
 *
 *  Created on: Jul 9, 2014
 *      Author: netmind
 */
#include <string.h>
#include <stdlib.h>
#include "EsHttpTextBody.h"

EsHttpTextBody::EsHttpTextBody(char* txt, int len)
{
	mBuf = (char*)malloc(len);
	mLen = len;
	memcpy(mBuf, txt, len);
}

EsHttpTextBody::~EsHttpTextBody()
{
	if(mBuf)
	{
		free(mBuf);
	}
}

int EsHttpTextBody::open()
{
	return 0;
}

const char* EsHttpTextBody::getContentType()
{
	return "text/plain";
}

int EsHttpTextBody::getContentLen()
{
	return mLen;
}

int EsHttpTextBody::getData(void* buf, int bufsize)
{
	return 0;
}

int EsHttpTextBody::getStreamType()
{
	return mType;
}

void* EsHttpTextBody::getBuffer()
{
	return mBuf;
}

void EsHttpTextBody::close()
{
	delete this;
}
