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
#if 0
	mEncStartStream = NULL;
	mEncHeaderSize = 0;
	mEncHeaderReadCnt = 0;
#endif
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
		mBodyReader = reader;
		int clen = mBodyReader->getSize();
		char buf[20];
		sprintf(buf, "%d", clen);
		mRespMsg.addHdr(HTTPHDR_CONTENT_LEN, buf);
		mRespMsg.addHdr(HTTPHDR_CONTENT_TYPE, type);
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

	mHeaderEncStr.clear();
	resp->encodeRespMsg(&mHeaderEncStr);
#if 1
#else
	mEncHeaderReadCnt = 0;
	mEncHeaderSize = mHeaderEncStr.size();
	mEncStartStream = (char*) malloc(mHeaderEncStr.size());
	memcpy(mEncStartStream, mHeaderEncStr.c_str(), mEncHeaderSize);
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
	int pktbufsize;

	pinfo->len = 0;
	pinfo->buf = NULL;

	if (mHeaderEncStr.size() > 0)
	{
		pinfo->len = mHeaderEncStr.size();
		pktbufsize = max(SEND_BUF_SIZE, pinfo->len);
		pinfo->buf = malloc(pktbufsize);
		memcpy(pinfo->buf, mHeaderEncStr.c_str(), pinfo->len);
		mHeaderEncStr.clear();
	}
	else
	{
		pinfo->buf = malloc(SEND_BUF_SIZE);
		pinfo->len = 0;
		pktbufsize = SEND_BUF_SIZE;

	}

	if (mBodyReader != NULL)
	{
		int remain = pktbufsize - pinfo->len;
		if (remain > 0)
		{
			int rcnt = mBodyReader->Read((u8*) pinfo->buf + pinfo->len, remain);
			if (rcnt > 0)
			{
				pinfo->len += rcnt;
			}
		}
	}

	if (pinfo->len <= 0)
	{
		free(pinfo->buf);
		pinfo->buf = NULL;
	}

}

#if 0
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
#endif

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
