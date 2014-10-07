/*
 * EdHttpController.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#include "../config.h"

#define DBGTAG "htctr"
#define DBG_LEVEL DBG_DEBUG

#include <string.h>
#include "../EdType.h"
#include "../edslog.h"
#include "EdHttpController.h"
#include "EdHttpCnn.h"
#include "EdHttp.h"

namespace edft
{

#define CLEAR_ALL_MEMBERS() { 	mWriter = NULL, \
	mBodyReader = NULL, \
	mIsFinalResponsed = false, \
	mIsContinueResponse = false, \
	mCnn = NULL, \
	mUserData = NULL, \
	mTxTrying = false, \
	mIsBodyTxComplete = false, \
	mIsMultipartBody = false, \
	memset(mStatusCode, 0, sizeof(mStatusCode)); \
}

EdHttpController::EdHttpController()
{
	CLEAR_ALL_MEMBERS();
}

EdHttpController::~EdHttpController()
{
}

void EdHttpController::OnRequestHeader()
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
		if(mBodyReader != NULL) mIsBodyTxComplete = false;
		else mIsBodyTxComplete = true;

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

void EdHttpController::setConnection(EdHttpCnn* pcnn)
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

const string* EdHttpController::getReqHeaderString(const char* name)
{
	return mReqMsg.getHdrString(name);
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
	EdHttpMsg *resp = &mRespMsg;
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

	mIsFinalResponsed = true;
}

void EdHttpController::close()
{
	mReqMsg.free();
	mRespMsg.free();

// clear response packet bufs
	packet_buf_t pkt;
	for (; mPacketList.size() > 0;)
	{
		pkt = mPacketList.front();
		free(pkt.buf);
		mPacketList.pop_front();
	}

	CLEAR_ALL_MEMBERS();

}

void EdHttpController::getSendPacket(packet_buf_t* pinfo)
{
	int pktbufsize;

	pinfo->len = 0;
	pinfo->buf = NULL;

	if(mIsContinueResponse == true) {
#define CONTINUE_MSG "HTTP/1.1 100 Continue\r\n"
		pinfo->buf = strdup(CONTINUE_MSG);
		pinfo->len = strlen(CONTINUE_MSG);
		mIsContinueResponse = false;
		dbgd("  get packet for 100 continue...");
		return;
	}


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
			else
			{
				mIsBodyTxComplete = true;
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

void EdHttpController::initCtrl(EdHttpCnn* pcnn)
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


int EdHttpController::feedBodyData(void* buf, int len)
{
	if(mWriter != NULL) {
		return mWriter->writeData(buf, len);
	} else {
		return -1;
	}
}

bool EdHttpController::checkExpect()
{
	const string *ps = mReqMsg.getHdrString("Expect");
	if(ps != NULL && !ps->compare("100-continue"))
	{
		dbgd("Expect header exists", ps->c_str());
		mIsContinueResponse = true;
	}

	return mIsContinueResponse;
}


void EdHttpController::checkHeaders()
{
	const char *type = mReqMsg.getHdr("Content-Type");
	if(type!=NULL)
	{
		mReqCtype = new EdHdrContentType;
		mReqCtype->parse(type, strlen(type));
		if( !memcmp(mReqCtype->getType(), "multipart", sizeof("multipart")-1) ) {
			mIsMultipartBody = true;
		}
	}


}


const char* EdHttpController::getBoundary()
{
	if(mIsMultipartBody==true) {
		return mReqCtype->getParam("boundary");
	} else {
		return NULL;
	}
}


void EdHttpController::OnNewMultipart(EdMultipartInfo* pinfo)
{
}

void EdHttpController::OnMultipartData(EdMultipartInfo* pinfo)
{
}


void EdHttpController::OnDataNew(EdHttpContent* pct)
{
}

void EdHttpController::OnDataContinue(EdHttpContent* pct, const void* buf, int len)
{
}

void EdHttpController::OnDataRecvComplete(EdHttpContent* pct)
{
}

void EdHttpController::OnRequestMsg()
{
}

} /* namespace edft */

