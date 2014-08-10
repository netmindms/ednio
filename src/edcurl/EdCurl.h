/*
 * EdCurl.h
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */

#ifndef EDCURL_H_
#define EDCURL_H_
#include <curl/curl.h>
//#include "EdMultiCurl.h"
#include "../EdEvent.h"
#include "../EdObjList.h"
#include "EdCurlSocket.h"

namespace edft
{
class EdMultiCurl;

class EdCurl
{
friend class EdMultiCurl;

public:
	class ICurlStatusCb {
	public:
		virtual void IOnCurlStatus(EdCurl* pcurl, int status)=0;
	};
public:
	EdCurl();
	virtual ~EdCurl();
	void open(EdMultiCurl* pm);
	void setUrl(const char* url);
	int request(const char* url);
	int request();
	void close();
	void setCallback(ICurlStatusCb* cb);
	CURL* getCurl();
	CURLM* getMultiCurl();
	//void OnEventRead();
	//void OnEventWrite();
	virtual void OnHeaderComplete();
	virtual void OnBodyData(void *buf, int len);
	virtual void OnCurlEnd(int errcode);

private:
	EdMultiCurl *mEdMultiCurl;
	ICurlStatusCb *mCallback;
	CURL* mCurl;
	bool mIsRespHeaderComp;
	static size_t header_cb(void* buffer, size_t size, size_t nmemb, void* userp);
	static size_t body_cb(void* ptr, size_t size, size_t nmemb, void* user);
	//int curlSockCb(int fd, void *psock, int what);
	void procCurlDone(int result);
	//void setEvent(int evt);
};



} /* namespace edft */

#endif /* EDCURL_H_ */
