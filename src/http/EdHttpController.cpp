/*
 * EdHttpController.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#include "../ednio_config.h"

#define DBGTAG "HTCTR"
#define DBG_LEVEL DBG_WARN

#include <string.h>
#include "../EdType.h"
#include "../edslog.h"
#include "EdHttpController.h"
#include "EdHttpCnn.h"
#include "EdHttp.h"

namespace edft
{

#define CLEAR_ALL_MEMBERS() {  \
	mBodyReader = NULL, \
	mIsFinalResponsed = false, \
	mIsContinueResponse = false, \
	mCnn = NULL, \
	mUserData = NULL, \
	mTxTrying = false, \
	mIsBodyTxComplete = false, \
	mIsMultipartBody = false, \
	mReqCtype = NULL, \
	memset(mStatusCode, 0, sizeof(mStatusCode)); \
}

EdHttpController::EdHttpController()
{
	CLEAR_ALL_MEMBERS();
}

EdHttpController::~EdHttpController()
{
	dbgd("dest http ctrl,...ctype=%x", mReqCtype);

}

void EdHttpController::OnHttpRequestHeader()
{
}

void EdHttpController::OnHttpComplete(int result)
{
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
		dbge("### Fail: result already set.");
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

void EdHttpController::addReqHeader(string name, string val)
{
	mReqMsg.addHdr(name, val);
}

const char* EdHttpController::getReqHeader(const char* name)
{
	return mReqMsg.getHdr(name);
}

const string EdHttpController::getReqHeaderString(const char* name)
{
	return mReqMsg.getHdrString(name);
}

void EdHttpController::setUrl(string url)
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

	if(mBodyReader == NULL)
	{
		resp->addHdr(HTTPHDR_CONTENT_LEN, "0");
	}

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
	CHECK_DELETE_OBJ(mReqCtype);

	CLEAR_ALL_MEMBERS();

}

void EdHttpController::getSendPacket(packet_buf_t* pinfo)
{
	int pktbufsize;

	pinfo->len = 0;
	pinfo->buf = NULL;

	if(mIsContinueResponse == true) {
#define CONTINUE_MSG "HTTP/1.1 100 Continue\r\n\r\n"
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

void EdHttpController::OnHttpCtrlInit()
{
}

void* EdHttpController::getUserData()
{
	return mUserData;
}

string EdHttpController::getReqUrl()
{
	return mReqMsg.getUrl();
}


bool EdHttpController::checkExpect()
{
	const string ps = mReqMsg.getHdrString("Expect");
	if(!ps.compare("100-continue"))
	{
		dbgd("Expect header exists", ps.c_str());
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
			dbgd("body data is multipart");
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


void EdHttpController::OnHttpDataNew(EdHttpContent* pct)
{
}

void EdHttpController::OnHttpDataContinue(EdHttpContent* pct, const void* buf, int len)
{
}

void EdHttpController::OnHttpDataRecvComplete(EdHttpContent* pct)
{
}

void EdHttpController::OnHttpRequestMsg()
{
}


const int EdHttpController::getReqMethod()
{
	return mReqMethod;
}

} /* namespace edft */
