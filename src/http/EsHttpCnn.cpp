/*
 * EsHttpCnn.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "htcnn"
#define DBG_LEVEL DBG_DEBUG
#include <stack>
#include <unordered_map>
#include "../edslog.h"
#include "EsHttpCnn.h"
#include "EsHttpTask.h"
#include "http_parser.h"
#include "../EsFile.h"
#include "EsHttpMsg.h"
#include "EdHttp.h"
#include "EsHttpBodyStream.h"

namespace edft {
EsHttpCnn::EsHttpCnn()
{
	dbgd("http cnn const......");

	_rn = rand();
	_hidx = 0;

	mCurHdrName = mCurHdrVal = NULL;

	memset(&mParserSettings, 0, sizeof(mParserSettings));
	mParserSettings.on_message_begin = msg_begin;
	mParserSettings.on_message_complete = msg_end;
	mParserSettings.on_url = on_url;
	mParserSettings.on_status = on_status;
	mParserSettings.on_header_field = head_field_cb;
	mParserSettings.on_header_value = head_val_cb;
	mParserSettings.on_headers_complete = on_headers_complete;
	mParserSettings.on_body = body_cb;

	memset(&mParser, 0, sizeof(mParser));

	mBufSize = 8 * 1024;
	mReadBuf = (char*) malloc(mBufSize);

	mCurCtrl = NULL;
}

EsHttpCnn::~EsHttpCnn()
{
	free(mReadBuf);
}

#if 0
void EsHttpCnn::OnRead()
{
	dbgd("on read");
	char buf[8*1024];
	int rdcnt = recv(buf, sizeof(buf));
	if(rdcnt>0)
	{
		buf[rdcnt] = 0;
		dbgd("   read str=%s", buf);
	}
}

void EsHttpCnn::OnDisconnected()
{
	dbgd("on disco...");
}
#endif

void EsHttpCnn::initCnn(int fd, u32 handle, EsHttpTask *ptask)
{
	//setChildSock(fd);
	openChildSock(fd);
	mTask = ptask;
	mHandle = handle;

	http_parser_init(&mParser, HTTP_REQUEST);
	mParser.data = this;
}

void EsHttpCnn::procRead()
{
	int rdcnt = recv(mReadBuf, mBufSize);
	dbgv("proc read cnt=%d", rdcnt);
	if (rdcnt > 0)
	{
		//mReadBuf[rdcnt] = 0;
		//dbgd("proc read str=%s", mReadBuf);
//		EsFile file;
//		file.openFile("p.dat", EsFile::OPEN_RWC);
//		file.writeFile(mReadBuf, rdcnt);
//		file.closeFile();
		http_parser_execute(&mParser, &mParserSettings, mReadBuf, rdcnt);
	}
}

void EsHttpCnn::procDisconnected()
{
	close();
	deregisterEvent();
}

int EsHttpCnn::head_field_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn* pcnn = (EsHttpCnn*) parser->data;
	return pcnn->headerNameCb(parser, at, length);
//

}

int EsHttpCnn::head_val_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->headerValCb(parser, at, length);

}

int EsHttpCnn::body_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->bodyDataCb(parser, at, length);
}

int EsHttpCnn::msg_begin(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgMsgBeginCb(parser);

}

int EsHttpCnn::msg_end(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgMsgEndCb(parser);
}

int EsHttpCnn::on_url(http_parser* parser, const char* at, size_t length)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgUrlCb(parser, at, length);
}

int EsHttpCnn::on_headers_complete(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgHeaderComp(parser);

}

int EsHttpCnn::headerNameCb(http_parser*, const char* at, size_t length)
{
	//string tmp(at, length);
	//dbgd("name cb, %s", tmp.c_str());
	if (mPs == PS_FIRST_LINE)
	{
		procReqLine();
	}

	if (mIsHdrVal)
	{
		dbgd("header set, name=%s, val=%s", mCurHdrName->c_str(), mCurHdrVal->c_str());
		procHeader();
		mIsHdrVal = false;
	}
	if (mCurHdrName == NULL)
		mCurHdrName = new string();
	mCurHdrName->append(at, length);
	return 0;
}

int EsHttpCnn::headerValCb(http_parser*, const char* at, size_t length)
{
	//string tmp(at, length);
	//dbgd("val cb, %s", tmp.c_str());

	if (mCurHdrVal == NULL)
		mCurHdrVal = new string();

	mCurHdrVal->append(at, length);
	mIsHdrVal = true;
	return 0;

}

int EsHttpCnn::dgHeaderComp(http_parser* parser)
{
	if (mIsHdrVal)
	{
		dbgd("header comp, name=%s, val=%s", mCurHdrName->c_str(), mCurHdrVal->c_str());
		procHeader();
		mIsHdrVal = false;
	}

	mCurCtrl->OnRequest();

	IUriControllerCb *cb = mTask->getController(mCurUrl);
	if (cb)
	{
		mCurTrans->mController = cb;
		cb->IOnNewHttpTrans(mHandle, mCurTrans->mHandle, mCurTrans);
		if (mCurTrans->mIsResponsed)
		{
			mCurTrans->encodeResp();
			transmitReserved();
		}
	}
	return 0;
}

int EsHttpCnn::bodyDataCb(http_parser*, const char* at, size_t length)
{
	dbgd("body data len=%d", length);
	IUriControllerCb *cb = mTask->getController(mCurUrl);
	if (cb)
	{
		//cb->IOnMsgBody(mHandle, mCurTrans->mHandle, mCurTrans, (void*) at, length);
	}
	return 0;
}

int EsHttpCnn::dgMsgBeginCb(http_parser* parser)
{
	assert(mCurTrans == NULL);

	mPs = PS_FIRST_LINE;

	mIsHdrVal = false;
	mCurHdrName = NULL;
	mCurHdrVal = NULL;

	//mCurTrans = new EsHttpTrans(mTrhseed, this);
	//mTransMap.push_back(mCurTrans);
	//_hidx++;

	mCurTrans = allocTrans();

	return 0;
}

int EsHttpCnn::dgMsgEndCb(http_parser*)
{

	return 0;
}

int EsHttpCnn::dgUrlCb(http_parser* parser, const char* at, size_t length)
{
	if (mCurUrl == NULL)
		mCurUrl = new string(at, length);
	else
		mCurUrl->append(at, length);

	return 0;
}

int EsHttpCnn::on_status(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

int EsHttpCnn::statusCb(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

void EsHttpCnn::procHeader()
{

	mCurTrans->addReqHeader(mCurHdrName, mCurHdrVal);

	delete mCurHdrName;
	delete mCurHdrVal;

	mCurHdrName = mCurHdrVal = NULL;
}

void EsHttpCnn::procReqLine()
{
	dbgd("url = %s", mCurUrl->c_str());
	mPs = PS_HEADER;
	//EdHttpController* pctl = mTask->OnNewRequest(http_method_str((http_method)mParser.method), mCurUrl->c_str());
	mCurCtrl = mTask->getRegController(mCurUrl->c_str());

}

void EsHttpCnn::sendResponse(EsHttpTrans* ptrans)
{
	ptrans->mIsResponsed = true;
	transmitReserved();
}

void EsHttpCnn::transmitReserved()
{
	dbgd("transmit reserved..., trcnt=%d", mRespList.size());

	stack<list<EsHttpTrans*>::iterator > dellist;

	//char* buf = (char*) malloc(8 * 1024);
	auto itr = mRespList.begin();
	for (; itr != mRespList.end(); itr++)
	{
		EsHttpTrans* ptrans = (*itr);
		int ret = ptrans->transmitRespStream();
		if (ret == 0)
		{
			dbgd("transmit ok...handle=%0x", ptrans->mHandle);
			ptrans->mController->IOnCloseHttpTrans(mHandle, ptrans->mHandle);
			ptrans->close();
			dellist.push(itr);
		}
		else if (ret == -1)
		{
			break;
		}
	}

	//free(buf);

	dbgd("to remove cnt=%d", dellist.size());
	for(;!dellist.empty();)
	{
		auto itr = dellist.top();
		dellist.pop();
		dbgd("remove tr list, handle=%0x", (*itr)->mHandle);
		mTransMap.erase((*itr)->mHandle);
		mRespList.erase(itr);
		delete (*itr);
	}

	dbgd("resp list cnt=%d, tr list cnt=%d", mRespList.size(), mTransMap.size());

}

bool EsHttpCnn::transmitResponse(EsHttpTrans* ptrans)
{
	int len;
	EsHttpMsg *resp = &ptrans->mRespMsg;
	EsHttpBodyStream *body = ptrans->mBodyStream;
	char tmp[100];

	// status line
	string firstline = string("HTTP/1.1 ") + ptrans->mStatusCode + " " + es_get_http_desp(ptrans->mStatusCode) + "\r\n";
	resp->setStatusLine(&firstline);

	// Date header
	es_get_httpDate(tmp);
	resp->addHdr(HTTPHDR_DATE, tmp);
	resp->addHdr(HTTPHDR_SERVER, "EDNIO/0.2.0");
	if (body != NULL)
	{
		char tmp[100];
		sprintf(tmp, "%d", body->getContentLen());
		resp->addHdr(HTTPHDR_CONTENT_LEN, tmp);
		resp->addHdr(HTTPHDR_CONTENT_TYPE, body->getContentType());
	}

	string outbuf;
	resp->encodeRespMsg(&outbuf);

	if (body)
	{
		body->open();
		char* ptxt = (char*) body->getBuffer();
		outbuf.append(ptxt, body->getContentLen());
		body->close();
	}
	send(outbuf.c_str(), outbuf.size());

}

EsHttpTrans* EsHttpCnn::allocTrans()
{

	EsHttpTrans* ptrans = new EsHttpTrans(mTrhseed, this);
	mTransMap[mTrhseed] = ptrans;
	mRespList.push_back(ptrans);
	_hidx++;
	return ptrans;
}

void EsHttpCnn::freeTrans(EsHttpTrans* ptrans)
{

}

} // namespace edft
