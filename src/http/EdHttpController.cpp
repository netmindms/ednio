/*
 * EdHttpController.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */
#define DBGTAG "htctr"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdHttpController.h"
#include "EsHttpCnn.h"
#include "EdHttp.h"

namespace edft
{

EdHttpController::EdHttpController()
{
	// TODO Auto-generated constructor stub
	mWriter = NULL;
	mReader = NULL;
	//mTrans = NULL;
	mIsResponsed = false;
}

EdHttpController::~EdHttpController()
{
	// TODO Auto-generated destructor stub
}



void EdHttpController::OnRequest()
{
}

void EdHttpController::OnContentRecvComplete()
{
}

void EdHttpController::OnContentSendComplete()
{
}


void EdHttpController::OnComplete()
{
}

void EdHttpController::setReqBodyWriter(EdHttpWriter* writer)
{
	if(mWriter == NULL)
	{
		mWriter = writer;
	}
	else
	{
		dbge("### Body writer already set...");
	}
}


void EdHttpController::setHttpResult(const char* code)
{
	strncpy(mStatusCode, code, 3);

}

void EdHttpController::setRespBodyReader(EdHttpReader* reader)
{
	if(mReader == NULL)
	{
		mReader = reader;
	}
	else
	{
		dbge("### Body reader already set...");
	}
}


void EdHttpController::setConnection(EsHttpCnn* pcnn)
{
	mCnn = pcnn;
}

void EdHttpController::addReqHeader(string* name, string* val)
{
	mReqMsg.addHdr(name, val);
}

const char* EdHttpController::getReqHeader(char* name)
{
	return mReqMsg.getHdr(name);
}

void EdHttpController::setUrl(string *url)
{
	mReqMsg.setUrl(url);
}

long EdHttpController::getReqContentLen()
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

void EdHttpController::encodeResp()
{
	int len;
	EsHttpMsg *resp = &mRespMsg;
	char tmp[100];

	// status line
	string firstline = string("HTTP/1.1 ") + mStatusCode + " " + es_get_http_desp(mStatusCode) + "\r\n";
	resp->setStatusLine(&firstline);

	// Date header
	es_get_httpDate(tmp);
	resp->addHdr(HTTPHDR_DATE, tmp);
	resp->addHdr(HTTPHDR_SERVER, "ESEV/0.2.0");

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


#if 0
void EdHttpController::sendResp(char* code, void *textbody, int len, char* cont_type)
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
#endif

void EdHttpController::close()
{
	mReqMsg.free();
	mRespMsg.free();
}

#if 0
void EdHttpController::setRespBody(EsHttpBodyStream* body)
{
	mBodyStream = body;
}



int EdHttpController::getRespEncodeStream(void* buf, int len)
{
}

int EdHttpController::transmitRespStream()
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
#endif

}

int EdHttpController::getStreamData(void* buf)
{
	return 0;
}

/* namespace edft */
