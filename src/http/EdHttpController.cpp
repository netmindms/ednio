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
	if(mBodyReader == NULL)
	{
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

	string outbuf;
	resp->encodeRespMsg(&outbuf);
#if 1
	packet_buf_t pkt;
	pkt.len = outbuf.size();
	pkt.buf = malloc(outbuf.size());
	memcpy(pkt.buf, outbuf.c_str(), outbuf.size());
	mPacketList.push_back(pkt);
#else
	mEncHeaderSize = outbuf.size();
	mEncHeaderStream = (char*)malloc( outbuf.size() );
	memcpy(mEncHeaderStream, outbuf.c_str(), mEncHeaderSize);
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
	for(;mPacketList.size()>0;)
	{
		pkt = mPacketList.front();
		free(pkt.buf);
		mPacketList.pop_front();
	}

#else
	if(mEncHeaderStream != NULL)
	{
		free(mEncHeaderStream);
		mEncHeaderStream = NULL;
	}
#endif
}

#if 0
void EdHttpController::setRespBody(EsHttpBodyStream* body)
{
	mBodyStream = body;
}



int EdHttpController::getRespEncodeStream(void* buf, int len)
{
}
#endif

int EdHttpController::transmitRespStream()
{
#if 1
	if(mEncHeaderReadCnt < mEncHeaderSize)
	{

		int wcnt = mCnn->sendHttpPakcet(mEncHeaderStream+mEncHeaderReadCnt, mEncHeaderSize - mEncHeaderReadCnt);
		if(wcnt>=0)
		{
			mEncHeaderReadCnt += wcnt;
			return 0;
		}
		else
		{
			return -1;
		}
	}

	if(mBodyReader != NULL && (mEncHeaderReadCnt == mEncHeaderSize))
	{
		// send body stream
		char* buf = (char*)malloc(8*1024);
		if(buf != NULL)
		{
			long rcnt = mBodyReader->Read(buf, 8*1024);
			if(rcnt>0)
			{
				mCnn->sendHttpPakcet(buf, rcnt);
			}
			free(buf);
		}
		else
			assert(0); // TODO memory alloc fail processing
	}
#else // old

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
#endif
}

packet_buf_t EdHttpController::getSendPacket()
{
	packet_buf_t pkt;
	if(mPacketList.size() > 0)
	{
		pkt = mPacketList.front();
		mPacketList.pop_front();
	}
	else
	{
		pkt.len = 0;
		pkt.buf = NULL;
	}
	return pkt;
}

} /* namespace edft */
