/*
 * EsHttpCnn.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "htcnn"
#define DBG_LEVEL DBG_WARN
#include <stack>
#include <unordered_map>
#include "../edslog.h"
#include "EsHttpCnn.h"
#include "EsHttpTask.h"
#include "http_parser.h"
#include "../EdFile.h"
#include "EsHttpMsg.h"
#include "EdHttp.h"
#include "EsHttpBodyStream.h"

namespace edft
{

EsHttpCnn::EsHttpCnn()
{
	dbgd("http cnn const......");

	mTask = NULL;
	mHandle = 0;

	_rn = rand();
	_hidx = 0;

	// init controller
	mCurSendCtrl = NULL;
	mCurCtrl = NULL;

	// init parser
	mPs = PS_INIT;
	mCurUrl = NULL;
	mIsHdrVal = false;
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

	//mBufSize = 8 * 1024;
	//mReadBuf = (char*) malloc(mBufSize);
	mBufSize = 0;
	mReadBuf = NULL;

	mTxTrying = false;
}

EsHttpCnn::~EsHttpCnn()
{
	if (mReadBuf != NULL)
	{
		free(mReadBuf);
		mReadBuf = NULL;
	}
}

void EsHttpCnn::close()
{
	mSock.socketClose();
	closeAllCtrls();

	if (mReadBuf != NULL)
	{
		free(mReadBuf);
		mReadBuf = NULL;
	}

}

int EsHttpCnn::initCnn(int fd, u32 handle, EsHttpTask *ptask, int socket_mode)
{
	mSock.setOnNetListener(this);
	mSock.socketOpenChild(fd, socket_mode);

	mTask = ptask;
	mHandle = handle;

	http_parser_init(&mParser, HTTP_REQUEST);
	mParser.data = this;

	mBufSize = 16 * 1024;
	mReadBuf = malloc(mBufSize);
	if (mReadBuf == NULL)
	{
		dbge("### Fail: memory allocation error for ssl read buffer...");
		goto error_exit;
	}

	return 0;
	error_exit: mSock.socketClose();
	return -1;
}

void EsHttpCnn::procRead()
{
	int rdcnt = mSock.recvPacket(mReadBuf, mBufSize);
	dbgv("proc read cnt=%d", rdcnt);
	if (rdcnt > 0)
	{
		http_parser_execute(&mParser, &mParserSettings, (char*) mReadBuf, rdcnt);
	}
}

void EsHttpCnn::procDisconnected()
{
	close();
	mTask->freeConnection(this);
	// after this line, any code not allowed.
	//
	//
}

int EsHttpCnn::head_field_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn* pcnn = (EsHttpCnn*) parser->data;
	return pcnn->headerNameCb(parser, at, length);
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
	string tmp(at, length);
	dbgv("name cb, %s", tmp.c_str());
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
	{
		mCurHdrName = new string();
	}

	mCurHdrName->append(at, length);
	return 0;
}

int EsHttpCnn::headerValCb(http_parser*, const char* at, size_t length)
{
	string tmp(at, length);
	dbgv("val cb, %s", tmp.c_str());

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

	if (mCurCtrl != NULL)
	{
		mTxTrying = true;
		mCurCtrl->OnRequest();
		mTxTrying = false;
		if(mCurCtrl->mIsFinalResponsed==true)
		{
			scheduleTransmit();
		}
	}
	return 0;
}

int EsHttpCnn::bodyDataCb(http_parser*, const char* at, size_t length)
{
	dbgd("body data len=%d", length);
	if(mCurCtrl != NULL) {

	}
	return length;
}

int EsHttpCnn::dgMsgBeginCb(http_parser* parser)
{

	mPs = PS_FIRST_LINE;

	mIsHdrVal = false;
	mCurHdrName = NULL;
	mCurHdrVal = NULL;

	//mCurTrans = new EsHttpTrans(mTrhseed, this);
	//mTransMap.push_back(mCurTrans);
	//_hidx++;

	return 0;
}

int EsHttpCnn::dgMsgEndCb(http_parser* parser)
{
	dbgd("http msg end...");
	if(mCurUrl != NULL)
	{
		delete mCurUrl;mCurUrl = NULL;
	}
	CHECK_DELETE_OBJ(mCurHdrName);
	CHECK_DELETE_OBJ(mCurHdrVal);
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
	if (mCurCtrl != NULL)
		mCurCtrl->addReqHeader(mCurHdrName, mCurHdrVal);

	delete mCurHdrName;
	delete mCurHdrVal;

	mCurHdrName = mCurHdrVal = NULL;
}

void EsHttpCnn::procReqLine()
{
	dbgd("url = %s", mCurUrl->c_str());
	mPs = PS_HEADER;
	//EdHttpController* pctl = mTask->OnNewRequest(http_method_str((http_method)mParser.method), mCurUrl->c_str());
	mCurCtrl = mTask->allocController(mCurUrl->c_str());
	if (mCurCtrl != NULL)
	{
		mCurCtrl->initCtrl(this);
		mCurCtrl->setUrl(mCurUrl);
		mCurCtrl->OnInit();
		mCtrlList.push_back(mCurCtrl);
	}
}


int EsHttpCnn::scheduleTransmit()
{
	dbgd("scheduling transmit..., ready ctrl cnt=%d", mCtrlList.size());
	//if (mCurSendCtrl != NULL)
	//	return 1;

	if (mCtrlList.size() == 0)
	{
		dbgd("    no controls to send");
		return 0;
	}

	//mCurSendCtrl = mCtrlList.front();

	stack<list<EdHttpController*>::iterator> dellist;

	int sr;
	auto itr = mCtrlList.begin();
	for (; itr != mCtrlList.end(); itr++)
	{
		mCurSendCtrl = (*itr);
		sr = sendCtrlStream(mCurSendCtrl, 16 * 1024);
		if (sr == SEND_OK)
		{
			dellist.push(itr);
			mCurSendCtrl->OnComplete(0);
			mCurSendCtrl->close();
			mTask->freeController(mCurSendCtrl);
			mCurSendCtrl = NULL;
		}
		else
		{
			break;
		}
	}

	dbgd("to remove cnt=%d", dellist.size());
	for (; !dellist.empty();)
	{
		auto itr = dellist.top();
		mCtrlList.erase(itr);
		dellist.pop();
	}

	dbgd("ctrl list cnt=%d", mCtrlList.size());
	return mCtrlList.size();
}

void EsHttpCnn::IOnNet(EdSmartSocket* psock, int event)
{
	if (event == NETEV_READ)
	{
		procRead();
	}
	else if (event == NETEV_SENDCOMPLETE)
	{
		//mCtrlList.pop_front();
		//mTask->freeController(mCurSendCtrl);
		//mCurSendCtrl = NULL;
		scheduleTransmit();
	}
	else if (event == NETEV_DISCONNECTED)
	{
		procDisconnected();
	}
}

int EsHttpCnn::sendCtrlStream(EdHttpController* pctl, int maxlen)
{
	int retVal = SEND_FAIL;
	packet_buf_t bf;

	int sentlen=0;
	for (;;)
	{
		if (mSock.isWritable() == false) {
			retVal = SEND_PENDING;
			break;
		}

		pctl->getSendPacket(&bf);

		if (bf.len > 0)
		{
			retVal = mSock.sendPacket(bf.buf, bf.len, true);
			//dbgd("send packet, wret=%d, inlen=%d", retVal, bf.len);
			//free(bf.buf);
			//dbgd("free buf, ptr=%0x", bf.buf);
			if (retVal != SEND_OK) {
				dbgd("*** send packet nok, ret=%d", retVal);
				break;
			} else {
				sentlen += bf.len;
				if(sentlen >= maxlen) {
					dbgd("=================== reserve write, sentlen=%d, maxlen=%d", sentlen, maxlen);
					mSock.reserveWrite();
					retVal = SEND_PENDING;
					break;
				}
			}
		}
		else
		{
			retVal = SEND_OK;
			break;
		}
	}

	return retVal;
}


void EsHttpCnn::closeAllCtrls()
{
	EdHttpController* pctrl;
	for (; mCtrlList.size() > 0;)
	{
		pctrl = mCtrlList.front();
		mCtrlList.pop_front();
		pctrl->OnComplete(-1);
		pctrl->close();
		mTask->freeController(pctrl);
	}
	mCurSendCtrl = NULL;
}


void EsHttpCnn::reqTx(EdHttpController* pctl)
{
	if(mTxTrying==false)
	{
		scheduleTransmit();
	}
}

} // namespace edft
