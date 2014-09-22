/*
 * EsHttpTrans.h
 *
 *  Created on: Jun 19, 2014
 *      Author: khkim
 */

#if 0
#ifndef ESHTTPTRANS_H_
#define ESHTTPTRANS_H_

#include "../config.h"

#include <string>
#include <stdexcept>
#include <unordered_map>

#include "../EdType.h"
#include "EsHttpBodyStream.h"
#include "EsHttpMsg.h"
#include "EdHttpController.h"
#include "http_parser.h"

using namespace std;
namespace edft {

class EsHttpCnn;
class IUriControllerCb;

class EsHttpTrans
{
	friend class EsHttpCnn;
	friend class EdHttpController;

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
	EdHttpController* mUrlCtrl;
};

} // namespace edft
#endif /* ESHTTPTRANS_H_ */
#endif
