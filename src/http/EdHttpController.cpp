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
	mBodyReader = NULL;
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
	strncpy(mStatusCode, code, 3);
	encodeResp();
	mCnn->scheduleTransmit();
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
	if(mBodyReader != NULL) {
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
	mIsResponsed = true;
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

packet_buf_t EdHttpController::getSendPacket()
{
	packet_buf_t bf;

	bf.len = 0;
	bf.buf = NULL;

	if (mEncStartStream != NULL)
	{
		bf.len = mEncHeaderSize;
		bf.buf = mEncStartStream;
		mEncStartStream = NULL;
		mEncHeaderReadCnt = 0;
		mEncHeaderSize = 0;
		return bf;
	}

	if(mBodyReader != NULL	) {
		bf.buf = malloc(SEND_BUF_SIZE);
		int rcnt = mBodyReader->Read((u8*)bf.buf, SEND_BUF_SIZE);
		bf.len = rcnt;
	}

	return bf;

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

} /* namespace edft */
