/*
 * EsHttpTrans.h
 *
 *  Created on: Jun 19, 2014
 *      Author: khkim
 */

#ifndef ESHTTPTRANS_H_
#define ESHTTPTRANS_H_


#include <string>
#include <stdexcept>
#include <unordered_map>

#include "EsHttpBodyStream.h"
#include "EdHttpMsg.h"
#include "http_parser.h"

using namespace std;
using namespace edft;

class EsHttpCnn;
class IUriControllerCb;

class EsHttpTrans
{
	friend class EsHttpCnn;
public:
	EsHttpTrans(u32 handle, EsHttpCnn *pcnn);
	virtual ~EsHttpTrans();
	void setUrl(string *url);
	void addReqHeader(string *name, string *val);
	//void addHeader(char *name, char *val);
	const char* getReqHeader(char* name);

	long getReqContentLen();


	void setRespBody(EsHttpBodyStream* body);
	void sendResp(char* code, void* textbody, int len, char* cont_type);

	void close();

	u32 mHandle;
	http_parser_url mUrl;

	EsHttpMsg mReqMsg;
	EsHttpMsg mRespMsg;
	char mStatusCode[4];

private:
	EsHttpCnn *mCnn;
	bool mIsResponsed;
	bool mIsReleased;
	bool mIsRespReserved;
	bool mIsRespComplete;

	// send response
	void encodeResp();
	int transmitRespStream();
	int getRespEncodeStream(void* buf, int len);

	char* mEncodeStream;
	int mEncSize;
	int mEncReadCnt;
	EsHttpBodyStream *mBodyStream;
	EsHttpBodyStream *mUserBodyStream;

	IUriControllerCb *mController;
};


#endif /* ESHTTPTRANS_H_ */
