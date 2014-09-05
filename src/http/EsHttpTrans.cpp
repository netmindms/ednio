/*
 * EsHttpTrans.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: khkim
 */

#define LOG_LEVEL LOG_DEBUG
#define TAG "htran"

#include <string.h>

#include "../EdType.h"

#include "../edslog.h"
#include "EdHttp.h"
#include "EsHttpTrans.h"

#include "EsHttpCnn.h"
#include "EsHttpTextBody.h"

EsHttpTrans::EsHttpTrans(u32 handle, EsHttpCnn *pcnn)
{
	mHandle = handle;
	mCnn = pcnn;
	mIsResponsed = false;
	mIsReleased = false;
	mIsRespReserved = false;

	mBodyStream = NULL;
	mUserBodyStream = NULL;
	*mStatusCode = 0;
}

EsHttpTrans::~EsHttpTrans()
{
	dbgd("trans dest, handle=%0x", mHandle);
	close();
}

void EsHttpTrans::addReqHeader(string* name, string* val)
{
	mReqMsg.addHdr(name, val);
}

const char* EsHttpTrans::getReqHeader(char* name)
{
	return mReqMsg.getHdr(name);
}

void EsHttpTrans::setUrl(string *url)
{
	mReqMsg.setUrl(url);
}

long EsHttpTrans::getReqContentLen()
{
	const char* val = mReqMsg.getHdr("Content-Length");
	if (val)
	{
		return atol(val);
	}
	else
	{
		return -1;
	}
}

void EsHttpTrans::sendResp(char* code, void *textbody, int len, char* cont_type)
{
#if 1
	EsHttpTextBody *body = new EsHttpTextBody((char*) textbody, len);
	setRespBody(body);
	memcpy(mStatusCode, code, 4);
	mIsResponsed = true;
	//mCnn->sendResponse(this);
#else
	char tmp[100];

	// status line
	string firstline = string("HTTP/1.1 ") + code + " " + es_get_http_desp(code) + "\r\n";
	mRespMsg.setStatusLine(&firstline);

	// Date header
	es_get_httpDate(tmp);
	mRespMsg.addHdr(HTTPHDR_DATE, tmp);

	mRespMsg.addHdr(HTTPHDR_SERVER, "ESEV/0.2.0");
	if(len > 0)
	{
		char tmp[100];
		sprintf(tmp, "%d", len);
		mRespMsg.addHdr(HTTPHDR_CONTENT_LEN, tmp);
		mRespMsg.addHdr(HTTPHDR_CONTENT_TYPE, cont_type);
	}

	string outbuf;
	mRespMsg.encodeRespMsg(&outbuf);
	if(len>0)
	{
		outbuf.append((char*)textbody, len);
	}
	mCnn->send(outbuf.c_str(), outbuf.size());
#endif
}

void EsHttpTrans::close()
{
	if(mBodyStream) {
		delete mBodyStream;
		mBodyStream = NULL;
	}

	if(mEncodeStream) {
		delete mEncodeStream;
		mEncodeStream = NULL;
	}

	mUserBodyStream = NULL;

	mReqMsg.free();
	mRespMsg.free();
}

void EsHttpTrans::setRespBody(EsHttpBodyStream* body)
{
	mBodyStream = body;
}

void EsHttpTrans::encodeResp()
{
	int len;
	EsHttpMsg *resp = &mRespMsg;
	EsHttpBodyStream *body = mBodyStream;
	char tmp[100];

	// status line
	string firstline = string("HTTP/1.1 ") + mStatusCode + " " + es_get_http_desp(mStatusCode) + "\r\n";
	resp->setStatusLine(&firstline);

	// Date header
	es_get_httpDate(tmp);
	resp->addHdr(HTTPHDR_DATE, tmp);
	resp->addHdr(HTTPHDR_SERVER, "ESEV/0.2.0");
	if (body != NULL)
	{
		char tmp[100];
		sprintf(tmp, "%d", body->getContentLen());
		resp->addHdr(HTTPHDR_CONTENT_LEN, tmp);
		resp->addHdr(HTTPHDR_CONTENT_TYPE, body->getContentType());
	}

	string outbuf;
	resp->encodeRespMsg(&outbuf);
	mEncSize = outbuf.size();
	//mEncodeStream = (char*)malloc(outbuf.size());
	mEncodeStream = new char[ outbuf.size() ];

	memcpy(mEncodeStream, outbuf.c_str(), mEncSize);

	/*
	if (body)
	{
		body->open();
		char* ptxt = (char*) body->getBuffer();
		outbuf.append(ptxt, body->getContentLen());
		body->close();
	}
	send(outbuf.c_str(), outbuf.size());
	*/
}

int EsHttpTrans::getRespEncodeStream(void* buf, int len)
{
}

int EsHttpTrans::transmitRespStream()
{
	if (mEncReadCnt < mEncSize)
	{
		int wcnt = mCnn->send(mEncodeStream, mEncSize - mEncReadCnt);
		if (mBodyStream)
		{
			mCnn->send(mBodyStream->getBuffer(), mBodyStream->getContentLen());
		}
		return 0; // TODO:
	}
	else
		return -1;
}
