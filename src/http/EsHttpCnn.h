/*
 * EsHttpCnn.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPCNN_H_
#define ESHTTPCNN_H_


#include <unordered_map>
#include <vector>
#include <string>
#include <list>
#include "../EdSocket.h"
#include "http_parser.h"
#include "EsHttpTrans.h"
#include "../EdSocketChannel.h"

using namespace std;

namespace edft {


enum PARSER_STATUS_E {
	PS_FIRST_LINE,
	PS_HEADER,
	PS_BODY,
};

enum SEND_RESULT_E {
	HTTP_SEND_FAIL=-1,
	HTTP_SEND_OK =0,
	HTTP_SEND_PENDING,
};

class EsHttpTask;

class EsHttpCnn : public EdObject, public EdSmartSocket::INet
{
	friend class EsHttpTask;
	friend class EsHttpTrans;
	friend class EdHttpController;
public:
	EsHttpCnn();
	virtual ~EsHttpCnn();


#if 0
	virtual void OnRead();
	virtual void OnDisconnected();
#endif
	virtual void IOnNet(EdSmartSocket* psock, int event);

	void initCnn(int fd, u32 handle, EsHttpTask* ptask);
	void procRead();
	void procDisconnected();



private:
	static int head_field_cb(http_parser*, const char *at, size_t length);
	int headerNameCb(http_parser*, const char *at, size_t length);
	static int head_val_cb(http_parser*, const char *at, size_t length);
	int headerValCb(http_parser*, const char *at, size_t length);
	static int on_headers_complete(http_parser *parser);
	int dgHeaderComp(http_parser *parser);
	static int body_cb(http_parser*, const char *at, size_t length);
	int bodyDataCb(http_parser*, const char *at, size_t length);
	static int msg_begin(http_parser* parser);
	int dgMsgBeginCb(http_parser* parser);
	static int msg_end(http_parser*);
	int dgMsgEndCb(http_parser*);
	static int on_url(http_parser *parser, const char *at, size_t length);
	int dgUrlCb(http_parser *parser, const char *at, size_t length);
	static int on_status(http_parser *parser, const char *at, size_t length);
	int statusCb(http_parser *parser, const char *at, size_t length);
	void procHeader();
	void procReqLine();

	void sendResponse(EsHttpTrans* ptrans);
	void transmitReserved();
	bool transmitResponse(EsHttpTrans* ptrans);
	EsHttpTrans* allocTrans();

	void freeTrans(EsHttpTrans* ptrans);
	int sendHttpPakcet(void* buf, int size);
	void scheduleTransmit();
	int sendCtrlStream(EdHttpController* pctl, int maxlen);

private:
	EsHttpTask* mTask;
	u32 mHandle;
	int mBufSize;
	char *mReadBuf;
	EsHttpTrans *mCurTrans;

	union {
		struct {
		u16 _hidx;
		u16 _rn;
		};
		u32 mTrhseed;
	};


	unordered_map<u32, EsHttpTrans*> mTransMap;
	std::list<EsHttpTrans *> mRespList;

	//std::string mCurHdrName, mCurHdrVal;
	string *mCurHdrName, *mCurHdrVal, *mCurUrl;
	bool mIsHdrVal;
	PARSER_STATUS_E mPs;

	int mParsingStatus;
	http_parser mParser;
	http_parser_settings mParserSettings;

	// controller list
	std::list<EdHttpController*> mCtrlList;
	EdHttpController* mCurCtrl;
	EdHttpController* mCurSendCtrl;
	EdSmartSocket mSock;

};

} // namespace edft

#endif /* ESHTTPCNN_H_ */
