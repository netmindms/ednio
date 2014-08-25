/*
 * EdCurl.h
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */

#ifndef EDCURL_H_
#define EDCURL_H_
#include "../config.h"

#include <string>
#include <unordered_map>
#include <curl/curl.h>
//#include "EdMultiCurl.h"
#include "../EdEvent.h"
#include "../EdObjList.h"
#include "EdCurlSocket.h"

using namespace std;

namespace edft
{
class EdMultiCurl;

class EdCurl
{
friend class EdMultiCurl;

public:
	class ICurlCb {
	public:
		virtual void IOnCurlStatus(EdCurl* pcurl, int status)=0;
		virtual void IOnCurlHeader(EdCurl* pcurl)=0;
		virtual void IOnCurlBody(EdCurl* pcurl, void* ptr, int size)=0;
	};

public:
	EdCurl();
	virtual ~EdCurl();
	void open(EdMultiCurl* pm);
	void setUrl(const char* url);
	int request(const char* url);
	int request();
	void close();
	void setCallback(ICurlCb* cb);
	CURL* getCurl();
	CURLM* getMultiCurl();
	virtual void OnCurlHeader();
	virtual void OnCurlBody(void *buf, int len);
	virtual void OnCurlEnd(int errcode);
	char* convCodeToStr(char *buf, int code);
	const char* getHeader(const char *name);
	int getResponseCode();
	void setUser(void* user);
	void* getUser();

private:
	EdMultiCurl *mEdMultiCurl;
	ICurlCb *mCallback;
	CURL* mCurl;
	bool mIsRespHeaderComp;
	void* mUser;

	unordered_map<string, string> mHeaderList;
	static size_t header_cb(void* buffer, size_t size, size_t nmemb, void* userp);
	size_t dgheader_cb(char* buffer, size_t size, size_t nmemb, void* userp);
	static size_t body_cb(void* ptr, size_t size, size_t nmemb, void* user);
	void procCurlDone(int result);
};



} /* namespace edft */

#endif /* EDCURL_H_ */
