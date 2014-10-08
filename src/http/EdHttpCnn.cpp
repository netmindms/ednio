/*
 * EsHttpCnn.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */
#include "../config.h"

#define DBGTAG "HTCNN"
#define DBG_LEVEL DBG_DEBUG

#include <stack>
#include <unordered_map>
#include "../edslog.h"
#include "EdHttpCnn.h"
#include "EdHttpTask.h"
#include "http_parser.h"
#include "../EdFile.h"
#include "EdHttpMsg.h"
#include "EdHttp.h"

namespace edft
{

static http_parser_settings _parserSettings;

EdHttpCnn::EdHttpCnn()
{
	dbgv("Http connection construct......");

	mTask = NULL;
	mHandle = 0;

	_rn = rand();
	_hidx = 0;

	// init controller
	mCurCtrl = NULL;

	// init parser
	mPs = PS_INIT;
	mCurUrl = NULL;
	mIsHdrVal = false;
	mCurHdrName = mCurHdrVal = NULL;
	mCurContent = NULL;
	memset(&mParser, 0, sizeof(mParser));

	//mBufSize = 8 * 1024;
	//mReadBuf = (char*) malloc(mBufSize);
	mBufSize = 0;
	mReadBuf = NULL;

	mTxTrying = false;
	mMpParser = NULL;
}

EdHttpCnn::~EdHttpCnn()
{
	CHECK_FREE_MEM(mReadBuf);
}

void EdHttpCnn::close()
{
	mSock.socketClose();
	closeAllCtrls();

	if (mReadBuf != NULL)
	{
		free(mReadBuf);
		mReadBuf = NULL;
	}

}

int EdHttpCnn::initCnn(int fd, u32 handle, EdHttpTask *ptask, int socket_mode)
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

void EdHttpCnn::procRead()
{
	int rdcnt = mSock.recvPacket(mReadBuf, mBufSize);
	dbgv("proc read cnt=%d", rdcnt);
	if (rdcnt > 0)
	{
		http_parser_execute(&mParser, &_parserSettings, (char*) mReadBuf, rdcnt);
	}
}

void EdHttpCnn::procDisconnectedNeedEnd()
{
	dbgd("Removing Http connection, fd=%d", mSock.getFd());
	close();
	mTask->removeConnection(this);
}

int EdHttpCnn::head_field_cb(http_parser* parser, const char *at, size_t length)
{
	EdHttpCnn* pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgHeaderNameCb(parser, at, length);
}

int EdHttpCnn::head_val_cb(http_parser* parser, const char *at, size_t length)
{
	EdHttpCnn *pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgHeaderValCb(parser, at, length);
}

int EdHttpCnn::body_cb(http_parser* parser, const char *at, size_t length)
{
	EdHttpCnn *pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgbodyDataCb(parser, at, length);
}

int EdHttpCnn::msg_begin(http_parser* parser)
{
	EdHttpCnn *pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgMsgBeginCb(parser);

}

int EdHttpCnn::msg_end(http_parser* parser)
{
	EdHttpCnn *pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgMsgEndCb(parser);
}

int EdHttpCnn::on_url(http_parser* parser, const char* at, size_t length)
{
	EdHttpCnn *pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgUrlCb(parser, at, length);
}

int EdHttpCnn::on_headers_complete(http_parser* parser)
{
	EdHttpCnn *pcnn = (EdHttpCnn*) parser->data;
	return pcnn->dgHeaderComp(parser);

}

int EdHttpCnn::dgHeaderNameCb(http_parser*, const char* at, size_t length)
{
	dbgv("parser hdr name cb, str=%s", string(at, length).c_str());
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

int EdHttpCnn::dgHeaderValCb(http_parser*, const char* at, size_t length)
{
	dbgv("parser hdr val cb, str=%s", string(at, length).c_str());

	if (mCurHdrVal == NULL)
		mCurHdrVal = new string();

	mCurHdrVal->append(at, length);
	mIsHdrVal = true;
	return 0;

}

int EdHttpCnn::dgHeaderComp(http_parser* parser)
{

	if (mIsHdrVal)
	{
		dbgv("header comp, name=%s, val=%s", mCurHdrName->c_str(), mCurHdrVal->c_str());
		procHeader();
		mIsHdrVal = false;
	}
	dbgd("parser header complete,...");
	if (mCurCtrl != NULL)
	{
		checkHeaders();
		mTxTrying = true;
		mCurCtrl->OnRequestHeader();
		mTxTrying = false;
		mCurCtrl->checkExpect(); // check Expect header...
		if (mCurCtrl->mIsFinalResponsed == true || mCurCtrl->mIsContinueResponse == true)
		{
			scheduleTransmitNeedEnd();
		}
	}
	return 0;
}

int EdHttpCnn::dgbodyDataCb(http_parser* parser, const char* at, size_t length)
{
	dbgd("body data len=%d", length);
	if (mCurCtrl == NULL)
		return 0;

	if (mCurCtrl->mIsMultipartBody == false)
	{
		if (mPs == PS_HEADER)
		{
			mPs = PS_BODY;
			mCurCtrl->OnDataNew(NULL);
		}
		mCurCtrl->OnDataContinue(NULL, at, length);
	}
	else
	{
		dbgd("\n%s", string(at, length).c_str());
		mMpParser->feed(at, length);
	}
	return 0;
}

int EdHttpCnn::dgMsgBeginCb(http_parser* parser)
{
	mPs = PS_FIRST_LINE;
	mIsHdrVal = false;
	mCurHdrName = NULL;
	mCurHdrVal = NULL;
	return 0;
}

int EdHttpCnn::dgMsgEndCb(http_parser* parser)
{
	dbgd("http parser ending ...");

	if (mCurCtrl != NULL)
	{
		if (mPs == PS_BODY)
		{
			dbgd("body data end...");
			mCurCtrl->OnDataRecvComplete(NULL);
		}
		mTxTrying = true;
		mCurCtrl->OnRequestMsg();
		mTxTrying = false;
		if (mCurCtrl->mIsFinalResponsed == true || mCurCtrl->mIsContinueResponse == true)
		{
			scheduleTransmitNeedEnd();
		}

	}

	if (mCurUrl != NULL)
	{
		delete mCurUrl;
		mCurUrl = NULL;
	}
	CHECK_DELETE_OBJ(mCurHdrName);
	CHECK_DELETE_OBJ(mCurHdrVal);
	closeMultipart();
	mPs = PS_INIT;
	return 0;
}

int EdHttpCnn::dgUrlCb(http_parser* parser, const char* at, size_t length)
{
	if (mCurUrl == NULL)
		mCurUrl = new string(at, length);
	else
		mCurUrl->append(at, length);

	return 0;
}

int EdHttpCnn::on_status(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

int EdHttpCnn::statusCb(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

void EdHttpCnn::procHeader()
{
	if (mCurCtrl != NULL)
		mCurCtrl->addReqHeader(mCurHdrName, mCurHdrVal);

	delete mCurHdrName;
	delete mCurHdrVal;

	mCurHdrName = mCurHdrVal = NULL;
}

void EdHttpCnn::procReqLine()
{
	dbgd("parser reuest line url = %s", mCurUrl->c_str());
	mPs = PS_HEADER;
	mCurCtrl = mTask->allocController(mCurUrl->c_str());
	if (mCurCtrl != NULL)
	{
		mCurCtrl->initCtrl(this);
		mCurCtrl->setUrl(mCurUrl);
		mCurCtrl->OnInit();
		mCtrlList.push_back(mCurCtrl);
	}
}

int EdHttpCnn::scheduleTransmitNeedEnd()
{
	dbgd("scheduling transmit..., pending ctrl cnt=%d", mCtrlList.size());

	if (mCtrlList.size() == 0)
	{
		dbgw("#####    no controls to send");
		return 0;
	}

	stack<list<EdHttpController*>::iterator> dellist;

	int sr;
	EdHttpController *sctrl=NULL;
	auto itr = mCtrlList.begin();
	for (; itr != mCtrlList.end(); itr++)
	{
		sctrl = (*itr);
		sr = sendCtrlStream(sctrl, 16 * 1024);
		if (sr == SEND_OK)
		{
			dellist.push(itr);
			sctrl->OnComplete(0);
			sctrl->close();
			mTask->freeController(sctrl);
			if (sctrl == mCurCtrl)
			{
				mCurCtrl = NULL;
			}
			sctrl = NULL;
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

void EdHttpCnn::IOnNet(EdSmartSocket* psock, int event)
{
	if (event == NETEV_READ)
	{
		procRead();
	}
	else if (event == NETEV_SENDCOMPLETE)
	{
		scheduleTransmitNeedEnd();
	}
	else if (event == NETEV_DISCONNECTED)
	{
		dbgd("smsocket disconnected...");
		procDisconnectedNeedEnd();
	}
}

int EdHttpCnn::sendCtrlStream(EdHttpController* pctl, int maxlen)
{
	int retVal = SEND_FAIL;
	packet_buf_t bf;

	int sentlen = 0;
	for (;;)
	{
		if (mSock.isWritable() == false)
		{
			retVal = SEND_PENDING;
			break;
		}

		pctl->getSendPacket(&bf);

		if (bf.len > 0)
		{
			retVal = mSock.sendPacket(bf.buf, bf.len, true);
			dbgd("send packet, wret=%d, inlen=%d", retVal, bf.len);
			//free(bf.buf);
			//dbgd("free buf, ptr=%0x", bf.buf);
			if (retVal != SEND_OK)
			{
				dbgd("*** send packet nok, ret=%d", retVal);
				break;
			}
			else
			{
				sentlen += bf.len;
				if (sentlen >= maxlen)
				{
					dbgd("=================== reserve write, sentlen=%d, maxlen=%d", sentlen, maxlen);
					mSock.reserveWrite();
					retVal = SEND_PENDING;
					break;
				}
			}
		}
		else
		{
			if (pctl->mIsBodyTxComplete == true)
				retVal = SEND_OK;
			else
				retVal = SEND_PENDING;
			break;
		}
	}

	return retVal;
}

void EdHttpCnn::closeAllCtrls()
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
}

void EdHttpCnn::reqTx(EdHttpController* pctl)
{
	if (mTxTrying == false)
	{
		scheduleTransmitNeedEnd();
	}
}

void EdHttpCnn::checkHeaders()
{
	mCurCtrl->checkHeaders();
	if (mCurCtrl->mIsMultipartBody == true)
	{
		dbgd("mutlipart parser init, boundary=%s", mCurCtrl->getBoundary());
		initMultipart(mCurCtrl->getBoundary());

	}
}

// multipart parser
//
void EdHttpCnn::mpPartBeginCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpCnn *dg = (EdHttpCnn*) userData;
	dg->dgMpPartBeginCb(buffer, start, end);
}

void EdHttpCnn::mpHeaderFieldCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpCnn *dg = (EdHttpCnn*) userData;
	dg->dgMpHeaderFieldCb(buffer, start, end);

}

void EdHttpCnn::mpHeaderValueCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpCnn *dg = (EdHttpCnn*) userData;
	dg->dgMpHeaderValueCb(buffer, start, end);
}

void EdHttpCnn::mpPartDataCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpCnn *dg = (EdHttpCnn*) userData;
	dg->dgMpPartDataCb(buffer, start, end);
}

void EdHttpCnn::mpPartEndCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpCnn *dg = (EdHttpCnn*) userData;
	dg->dgMpPartEndCb(buffer, start, end);
}

void EdHttpCnn::mpEndCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpCnn* mp = (EdHttpCnn*) userData;
	mp->dgMpEndCb(buffer, start, end);

}

void EdHttpCnn::dgMpPartBeginCb(const char* buffer, size_t start, size_t end)
{
	dbgd("part begin...");
	mPs = PS_MP_HEADER;
	mCurContent = new EdHttpContent(true);
}

void EdHttpCnn::dgMpHeaderFieldCb(const char* buffer, size_t start, size_t end)
{
	//dbgd("hdr filed cb, slice=%s", string(buffer+start, end-start).c_str());
	if (mCurMpHdrVal.size() != 0)
	{
		dbgd("part header complete, name=%s, val=%s", mCurMpHdrName.c_str(), mCurMpHdrVal.c_str());
		mCurContent->addHdr(&mCurMpHdrName, &mCurMpHdrVal);
		mMpHdrList[mCurMpHdrName] = mCurMpHdrVal;
		mCurMpHdrName.clear();
		mCurMpHdrVal.clear();
	}

	mCurMpHdrName.append(buffer + start, end - start);
	//dbgd("onHeaderField: (%s)\n", string(buffer + start, end - start).c_str());
}

void EdHttpCnn::dgMpHeaderValueCb(const char* buffer, size_t start, size_t end)
{

	dbgd("hdr val cb, hdr slice=%s", string(buffer + start, end - start).c_str());
	mCurMpHdrVal.append(buffer + start, end - start);

}

void EdHttpCnn::dgMpPartDataCb(const char* buffer, size_t start, size_t end)
{
	dbgd("part data cb...");
	if (mCurMpHdrName.size() > 0)
	{
		dbgd("last header complete, name=%s, val=%s", mCurMpHdrName.c_str(), mCurMpHdrVal.c_str());
		mCurContent->addHdr(&mCurMpHdrName, &mCurMpHdrVal);
		mMpHdrList[mCurMpHdrName] = mCurMpHdrVal;
		mCurMpHdrName.clear();
		mCurMpHdrVal.clear();
	}

	if (mPs == PS_MP_HEADER)
	{
		mPs = PS_MP_DATA;
		if (mCurContent->isValidMp() == true)
		{
			mCurCtrl->OnDataNew(mCurContent);
		}
		else
		{
			delete mCurContent;
			mCurContent = NULL;
		}

	}

	if (mCurContent != NULL)
		mCurCtrl->OnDataContinue(mCurContent, buffer + start, end - start);
}

void EdHttpCnn::dgMpPartEndCb(const char* buffer, size_t start, size_t end)
{
	dbgd("part end cb...");
	if (mCurContent != NULL)
		mCurCtrl->OnDataRecvComplete(mCurContent);
	CHECK_DELETE_OBJ(mCurContent);
}

void EdHttpCnn::dgMpEndCb(const char* buffer, size_t start, size_t end)
{
	dbgd("mp end cb...");
	mCurMpHdrVal.clear();
	mCurMpHdrName.clear();
}

void EdHttpCnn::initMultipart(const char* boundary)
{
	dbgd("multi part parser init, size=%d, cnn size=%d", sizeof(MultipartParser), sizeof(EdHttpCnn));
	mMpParser = new MultipartParser;
	// set callback
	mMpParser->onPartBegin = mpPartBeginCb;
	mMpParser->onHeaderField = mpHeaderFieldCb;
	mMpParser->onHeaderValue = mpHeaderValueCb;
	mMpParser->onPartData = mpPartDataCb;
	mMpParser->onPartEnd = mpPartEndCb;
	mMpParser->onEnd = mpEndCb;

	mMpParser->setBoundary(boundary);
	mMpParser->userData = (void*) this;
	mCurMpHdrName.clear();
	mCurMpHdrVal.clear();
}

void EdHttpCnn::closeMultipart()
{
	if (mMpParser != NULL)
	{
		dbgd("multi part parser close ...");
		delete mMpParser;
		mMpParser = NULL;
		mCurMpHdrName.clear();
		mCurMpHdrVal.clear();
	}
}

void EdHttpCnn::initHttpParser()
{
	// initialize callbacks just one time because static callback doesn't change.
	_parserSettings.on_message_begin = msg_begin;
	_parserSettings.on_message_complete = msg_end;
	_parserSettings.on_url = on_url;
	_parserSettings.on_status = on_status;
	_parserSettings.on_header_field = head_field_cb;
	_parserSettings.on_header_value = head_val_cb;
	_parserSettings.on_headers_complete = on_headers_complete;
	_parserSettings.on_body = body_cb;
}

} // namespace edft
