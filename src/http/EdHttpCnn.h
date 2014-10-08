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
#include "MultipartParser.h"

using namespace std;

namespace edft
{

enum PARSER_STATUS_E
{
	PS_INIT, PS_FIRST_LINE, PS_HEADER, PS_BODY, PS_MP_HEADER, PS_MP_DATA,
};

enum SEND_RESULT_E
{
	HTTP_SEND_FAIL = -1, HTTP_SEND_OK = 0, HTTP_SEND_PENDING,
};

class EdHttpTask;

class EdHttpCnn: public EdObject, public EdSmartSocket::INet
{
	friend class EdHttpTask;
	friend class EdHttpController;

public:

	EdHttpCnn();
	virtual ~EdHttpCnn();
	virtual void IOnNet(EdSmartSocket* psock, int event);
	static void initHttpParser();

private:
	static int head_field_cb(http_parser*, const char *at, size_t length);
	int dgHeaderNameCb(http_parser*, const char *at, size_t length);
	static int head_val_cb(http_parser*, const char *at, size_t length);
	int dgHeaderValCb(http_parser*, const char *at, size_t length);
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
	void checkHeaders();

	// multipart parser
	static void mpPartBeginCb(const char *buffer, size_t start, size_t end, void *userData);
	static void mpHeaderFieldCb(const char *buffer, size_t start, size_t end, void *userData);
	static void mpHeaderValueCb(const char *buffer, size_t start, size_t end, void *userData);
	static void mpPartDataCb(const char *buffer, size_t start, size_t end, void *userData);
	static void mpPartEndCb(const char *buffer, size_t start, size_t end, void *userData);
	static void mpEndCb(const char *buffer, size_t start, size_t end, void *userData);
	void dgMpPartBeginCb(const char *buffer, size_t start, size_t end);
	void dgMpHeaderFieldCb(const char *buffer, size_t start, size_t end);
	void dgMpHeaderValueCb(const char *buffer, size_t start, size_t end);
	void dgMpPartDataCb(const char *buffer, size_t start, size_t end);
	void dgMpPartEndCb(const char *buffer, size_t start, size_t end);
	void dgMpEndCb(const char *buffer, size_t start, size_t end);
	void initMultipart(const char* boundary);
	void feedMultipart(const char* buf, int len);

	int initCnn(int fd, u32 handle, EdHttpTask* ptask, int socket_mode);
	void procRead();
	void procDisconnectedNeedEnd();
	int scheduleTransmitNeedEnd();
	int sendCtrlStream(EdHttpController* pctl, int maxlen);
	void initMultipart();
	void closeMultipart();
	void reqTx(EdHttpController* pctl);

	void close();
	void closeAllCtrls();

private:
	EdHttpTask* mTask;
	u32 mHandle;
	int mBufSize;
	void *mReadBuf;

	union
	{
		struct
		{
			u16 _hidx;
			u16 _rn;
		};
		u32 mTrhseed;
	};

	string *mCurHdrName, *mCurHdrVal, *mCurUrl;
	bool mIsHdrVal;
	PARSER_STATUS_E mPs;

	http_parser mParser;
	//static http_parser_settings mParserSettings;

	// multipart parser variable
	MultipartParser *mMpParser;
	string mCurMpHdrName;
	string mCurMpHdrVal;
	unordered_map<string, string> mMpHdrList;

	// controller list
	std::list<EdHttpController*> mCtrlList;
	EdHttpController* mCurCtrl;
	//EdHttpController* mCurSendCtrl;
	EdHttpContent* mCurContent;
	EdSmartSocket mSock;
	bool mTxTrying;
};

} // namespace edft

#endif /* ESHTTPCNN_H_ */
