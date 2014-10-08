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
#include "EdHttpHdr.h"
#include "EdMultipartInfo.h"
#include "EdHttpContent.h"

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
	virtual void OnRequestHeader();
	virtual void OnContentRecvComplete();
	//virtual void OnContentSendComplete();
	virtual void OnComplete(int result);
	virtual void OnNewMultipart(EdMultipartInfo* pinfo);
	virtual void OnMultipartData(EdMultipartInfo* pinfo);
	virtual void OnDataNew(EdHttpContent *pct);
	virtual void OnDataContinue(EdHttpContent *pct, const void *buf, int len);
	virtual void OnDataRecvComplete(EdHttpContent *pct);
	virtual void OnRequestMsg();

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
	bool checkExpect();
	void checkHeaders();

private:
	void* mUserData;


	//EsHttpTrans* mTrans;
	EdHttpCnn* mCnn;
	EdHttpMsg mReqMsg;
	EdHttpMsg mRespMsg;
	EdHdrContentType* mReqCtype;
	bool mIsMultipartBody;

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
	const char* getBoundary();

	void getSendPacket(packet_buf_t* pinfo);
	void initCtrl(EdHttpCnn* pcnn);
};

} /* namespace edft */

#endif /* EDHTTPCONTROLLER_H_ */
