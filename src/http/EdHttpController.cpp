#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "../EdType.h"

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
	mWriter = NULL;
	mBodyReader = NULL;
	//mTrans = NULL;
	mIsFinalResponsed = false;
	mCnn = NULL;
	mEncStartStream = NULL;
	mEncHeaderSize = 0;
	mEncHeaderReadCnt = 0;
	memset(mStatusCode, 0, sizeof(mStatusCode));

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

#if 0
void EdHttpController::OnContentSendComplete()
{
}
#endif

void EdHttpController::OnComplete(int result)
{
}

void EdHttpController::setReqBodyWriter(EdHttpWriter* writer)
{
	if (mWriter == NULL)
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
	if (mIsFinalResponsed == false)
	{
		memcpy(mStatusCode, code, 4);
		encodeResp();
		mCnn->reqTx(this);
	}
	else
	{
		dbge("### Fail: result set error,...code=%s", code);
	}
}

void EdHttpController::setRespBodyReader(EdHttpReader* reader, const char* type)
{
	if (mBodyReader == NULL)
	{
		mRespMsg.addHdr("Content-Type", type);
		mBodyReader = reader;
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
	EsHttpMsg *resp = &mRespMsg;
	char tmp[100];

	// status line
	string firstline = string("HTTP/1.1 ") + mStatusCode + " " + es_get_http_desp(mStatusCode) + "\r\n";
	resp->setStatusLine(&firstline);

	// Date header
	es_get_httpDate(tmp);
	resp->addHdr(HTTPHDR_DATE, tmp);
	resp->addHdr(HTTPHDR_SERVER, "ESEV/0.2.0");
	if (mBodyReader != NULL)
	{
		int clen = mBodyReader->getSize();
		char buf[20];
		sprintf(buf, "%d", clen);
		resp->addHdr(HTTPHDR_CONTENT_LEN, buf);
		resp->addHdr(HTTPHDR_CONTENT_TYPE, "text/plain");
	}

	string outbuf;
	resp->encodeRespMsg(&outbuf);
#if 0
	packet_buf_t pkt;
	pkt.len = outbuf.size();
	pkt.buf = malloc(outbuf.size());
	memcpy(pkt.buf, outbuf.c_str(), outbuf.size());
	mPacketList.push_back(pkt);
#else
	mEncHeaderReadCnt = 0;
	mEncHeaderSize = outbuf.size();
	mEncStartStream = (char*) malloc(outbuf.size());
	memcpy(mEncStartStream, outbuf.c_str(), mEncHeaderSize);
#endif
	mIsFinalResponsed = true;
}

#if 0
void EdHttpController::sendResp(char* code, void *textbody, int len, char* cont_type)
{
#if 1
	EsHttpTextBody *body = new EsHttpTextBody((char*) textbody, len);
	setRespBody(body);
	memcpy(mStatusCode, code, 4);
	mIsFinalResponsed = true;
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

#if 1
	// clear response packet bufs
	packet_buf_t pkt;
	for (; mPacketList.size() > 0;)
	{
		pkt = mPacketList.front();
		free(pkt.buf);
		mPacketList.pop_front();
	}

#else
	if(mEncStartStream != NULL)
	{
		free(mEncStartStream);
		mEncStartStream = NULL;
	}
#endif
}

void EdHttpController::getSendPacket(packet_buf_t* pinfo)
{

	pinfo->len = 0;
	pinfo->buf = NULL;

	if (mEncStartStream != NULL)
	{
		pinfo->len = mEncHeaderSize;
		pinfo->buf = mEncStartStream;
		mEncStartStream = NULL;
		mEncHeaderReadCnt = 0;
		mEncHeaderSize = 0;
		return;
	}

	if (mBodyReader != NULL)
	{
		pinfo->buf = malloc(SEND_BUF_SIZE);
		if (pinfo->buf != NULL)
		{
			int rcnt = mBodyReader->Read((u8*) pinfo->buf, SEND_BUF_SIZE);
			pinfo->len = rcnt;
			if (pinfo->len <= 0)
			{
				free(pinfo->buf);
				pinfo->buf = NULL;
			}
		}
		return;
	}

}

int EdHttpController::getSendPacketData(void* buf, int len)
{
	int totalcnt = 0;
	if (mEncHeaderReadCnt < mEncHeaderSize)
	{
		int remain = mEncHeaderSize - mEncHeaderReadCnt;
		totalcnt = min(len, remain);
		memcpy(buf, mEncStartStream + mEncHeaderReadCnt, totalcnt);
		mEncHeaderReadCnt += totalcnt;
	}

	return 0;
}

void EdHttpController::initCtrl(EsHttpCnn* pcnn)
{
	mCnn = pcnn;
}

void EdHttpController::OnInit()
{
}

void* EdHttpController::getUserData()
{
	return mUserData;
}

const string* EdHttpController::getReqUrl()
{
	return mReqMsg.getUrl();
}

} /* namespace edft */
