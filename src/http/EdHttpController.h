/*
 * EdHttpController.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPCONTROLLER_H_
#define EDHTTPCONTROLLER_H_

#include "../config.h"

#include <list>
#include "../EdObject.h"
#include "EsHttpMsg.h"
#include "EdHttpWriter.h"
#include "EdHttpReader.h"
#include "EdHttpType.h"

namespace edft
{

#define SEND_BUF_SIZE (16*1024)
class EsHttpCnn;


class EdHttpController : public EdObject
{
	friend class EsHttpTask;
	friend class EsHttpCnn;
public:
	EdHttpController();
	virtual ~EdHttpController();
	virtual void OnInit();
	virtual void OnRequest();
	virtual void OnContentRecvComplete();
	//virtual void OnContentSendComplete();
	virtual void OnComplete(int result);
	void close();
	void setReqBodyWriter(EdHttpWriter* writer);
	void setRespBodyReader(EdHttpReader* reader, const char *type);
	const char* getReqHeader(char* name);
	long getReqContentLen();
	void *getUserData();
protected:
	void setHttpResult(const char *code);

private:
	void* mUserData;

	EdHttpWriter* mWriter;

	//EsHttpTrans* mTrans;
	EsHttpCnn* mCnn;
	EsHttpMsg mReqMsg;
	EsHttpMsg mRespMsg;

	char mStatusCode[4];
	bool mIsFinalResponsed;

	std::list<packet_buf_t> mPacketList;

	// header response stream data
	char *mEncStartStream;
	int mEncHeaderReadCnt;
	int mEncHeaderSize;
	packet_buf_t mHdrPkt;

	// body data stream
	EdHttpReader* mBodyReader;

	void setConnection(EsHttpCnn* pcnn);
	void addReqHeader(string* name, string* val);
	void setUrl(string *url);
	void sendResp(char* code, void *textbody, int len, char* cont_type);
	void encodeResp();
	int getRespEncodeStream(void* buf, int len);
	int transmitRespStream();
	packet_buf_t getSendPacket();
	int getSendPacketData(void* buf, int len);
	void initCtrl(EsHttpCnn* pcnn);
};

} /* namespace edft */

#endif /* EDHTTPCONTROLLER_H_ */
