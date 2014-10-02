/*
 * EsHttpCnn.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPCNN_H_
#define ESHTTPCNN_H_

#include "../config.h"

#include <unordered_map>
#include <string>
#include <list>
#include "../EdSocket.h"
#include "../edssl/EdSmartSocket.h"
#include "http_parser.h"
#include "EdHttpController.h"

using namespace std;

namespace edft {


enum PARSER_STATUS_E {
	PS_INIT,
	PS_FIRST_LINE,
	PS_HEADER,
	PS_BODY,
};

enum SEND_RESULT_E {
	HTTP_SEND_FAIL=-1,
	HTTP_SEND_OK =0,
	HTTP_SEND_PENDING,
};

class EdHttpTask;

class EdHttpCnn : public EdObject, public EdSmartSocket::INet
{
	friend class EdHttpTask;
	friend class EdHttpController;
public:
	EdHttpCnn();
	virtual ~EdHttpCnn();
	virtual void IOnNet(EdSmartSocket* psock, int event);

private:
	static int head_field_cb(http_parser*, const char *at, size_t length);
	int headerNameCb(http_parser*, const char *at, size_t length);
	static int head_val_cb(http_parser*, const char *at, size_t length);
	int headerValCb(http_parser*, const char *at, size_t length);
	static int on_headers_complete(http_parser *parser);
	int dgHeaderComp(http_parser *parser);
	static int body_cb(http_parser*, const char *at, size_t length);
	int dgbodyDataCb(http_parser*, const char *at, size_t length);
	static int msg_begin(http_parser* parser);
	int dgMsgBeginCb(http_parser* parser);
	static int msg_end(http_parser*);
	int dgMsgEndCb(http_parser* parser);
	static int on_url(http_parser *parser, const char *at, size_t length);
	int dgUrlCb(http_parser *parser, const char *at, size_t length);
	static int on_status(http_parser *parser, const char *at, size_t length);
	int statusCb(http_parser *parser, const char *at, size_t length);
	void procHeader();
	void procReqLine();

	int initCnn(int fd, u32 handle, EdHttpTask* ptask, int socket_mode);
	void procRead();
	void procDisconnectedNeedEnd();
	int scheduleTransmit();
	int sendCtrlStream(EdHttpController* pctl, int maxlen);
	void reqTx(EdHttpController* pctl);

	void close();
	void closeAllCtrls();

private:
	EdHttpTask* mTask;
	u32 mHandle;
	int mBufSize;
	void *mReadBuf;

	union {
		struct {
		u16 _hidx;
		u16 _rn;
		};
		u32 mTrhseed;
	};

	string *mCurHdrName, *mCurHdrVal, *mCurUrl;
	bool mIsHdrVal;
	PARSER_STATUS_E mPs;

	http_parser mParser;
	http_parser_settings mParserSettings;

	// controller list
	std::list<EdHttpController*> mCtrlList;
	EdHttpController* mCurCtrl;
	EdHttpController* mCurSendCtrl;
	EdSmartSocket mSock;
	long mReceivedBodySize;

	bool mTxTrying;

};

} // namespace edft

#endif /* ESHTTPCNN_H_ */
