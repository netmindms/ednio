/*
 * EdCurl.h
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */

#ifndef EDCURL_H_
#define EDCURL_H_
#include "../ednio_config.h"

#include <string>
#include <unordered_map>
#include <curl/curl.h>
//#include "EdMultiCurl.h"
#include "../EdEvent.h"
#include "../EdObjList.h"
#include "EdCurlSocket.h"
#include "../EdObject.h"
#include "../EdTimer.h"

using namespace std;

namespace edft
{

enum {
	EDCURL_IDLE=0,
	EDCURL_REQUESTING,
	EDCURL_RESPONSED,
	EDCURL_COMPLETE,
};

class EdMultiCurl;

class EdEasyCurl : public EdObject, public EdTimer::ITimerCb
{
friend class EdMultiCurl;

public:
/*
	class ICurlCb {
	public:
		virtual void IOnCurlStatus(EdCurl* pcurl, int status)=0;
		virtual void IOnCurlHeader(EdCurl* pcurl)=0;
		virtual void IOnCurlBody(EdCurl* pcurl, void* ptr, int size)=0;
	};
*/
	class ICurlResult {
	public:
		virtual void IOnCurlResult(EdEasyCurl* pcurl, int status)=0;
	};
	class ICurlHeader {
	public:
		virtual void IOnCurlHeader(EdEasyCurl* pcurl)=0;
	};
	class ICurlBody {
	public:
		virtual void IOnCurlBody(EdEasyCurl* pcurl, void* ptr, int size)=0;
	};


public:
	EdEasyCurl();
	virtual ~EdEasyCurl();
	void open(EdMultiCurl* pm);
	void setUrl(const char* url);
	int request(const char* url, int cnn_timeout_sec=30);
	int request(int cnn_timeout_sec=30);
	void close();
	void reset();
	//void setOnListener(ICurlCb* cb);
	void setOnCurlListener(ICurlResult* iresult, ICurlBody* ibody=NULL, ICurlHeader* iheader=NULL);

	CURL* getCurl();
	CURLM* getMultiCurl();
	virtual void OnCurlHeader();
	virtual void OnCurlBody(void *buf, int len);
	virtual void OnCurlEnd(int errcode);
	char* convCodeToStr(char *buf, int code);
	const char* getHeader(const char *name);
	long getContentLength();
	int getResponseCode();
	void setUser(void* user);
	void* getUser();
private:
	virtual void IOnTimerEvent(EdTimer* ptimer);

private:
	int mStatus;
	EdMultiCurl *mEdMultiCurl;
	ICurlResult* mOnResult;
	ICurlBody* mOnBody;
	ICurlHeader* mOnHeader;
	EdTimer* mRespTimer;

	CURL* mCurl;
	bool mIsRespHeaderComp;


	void* mUser;

	unordered_map<string, string> mHeaderList;
	static size_t header_cb(void* buffer, size_t size, size_t nmemb, void* userp);
	size_t dgheader_cb(char* buffer, size_t size, size_t nmemb, void* userp);
	static size_t body_cb(void* ptr, size_t size, size_t nmemb, void* user);
	void procCurlDone(int result);
	void setCurlCallback();
};



} /* namespace edft */

#endif /* EDCURL_H_ */
