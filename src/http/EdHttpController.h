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
#include "EdHttpMsg.h"
#include "EdHttpWriter.h"
#include "EdHttpReader.h"
#include "EdHttpType.h"

namespace edft
{

#define SEND_BUF_SIZE (8*1024)

class EdHttpCnn;


class EdHttpController : public EdObject
{
	friend class EdHttpTask;
	friend class EdHttpCnn;
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
	const string* getReqHeaderString(const char* name);
	long getReqContentLen();
	void *getUserData();
	const string* getReqUrl();

protected:
	void setHttpResult(const char *code);
private:
	int feedBodyData(void* buf, int len);
	bool checkExpect();

private:
	void* mUserData;

	EdHttpWriter* mWriter;

	//EsHttpTrans* mTrans;
	EdHttpCnn* mCnn;
	EdHttpMsg mReqMsg;
	EdHttpMsg mRespMsg;

	char mStatusCode[4];
	bool mIsFinalResponsed;
	bool mIsContinueResponse;
	bool mTxTrying;
	bool mIsBodyTxComplete;

	std::list<packet_buf_t> mPacketList;

	// header response stream data
	string mHeaderEncStr;

	packet_buf_t mHdrPkt;

	// body data stream
	EdHttpReader* mBodyReader;

	void setConnection(EdHttpCnn* pcnn);
	void addReqHeader(string* name, string* val);
	void setUrl(string *url);
	void encodeResp();

	void getSendPacket(packet_buf_t* pinfo);
#if 0
	int getSendPacketData(void* buf, int len);
#endif
	void initCtrl(EdHttpCnn* pcnn);
};

} /* namespace edft */

#endif /* EDHTTPCONTROLLER_H_ */
