/*
 * EdCurl.h
 *
 *  Created on: Aug 04, 2014
 *      Author: netmind
 */

#ifndef EDCURLMULTI_H_
#define EDCURLMULTI_H_

#include "../config.h"
#include <map>
#include <unordered_map>
#include <string>
#include <curl/curl.h>
#include "../EdEvent.h"
#include "../EdTimer.h"

namespace edft {

using namespace std;

class EdCurl;

class EdMultiCurl: public EdEvent, public EdTimer::ITimerCb
{
	friend class EdCurl;
public:
	EdMultiCurl();
	virtual ~EdMultiCurl();
	static int curl_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
	static int multi_timer_cb(CURLM *multi, long timeout_ms, void *userp);
	virtual void IOnTimerEvent(EdTimer* ptimer);
	virtual void OnEventRead();
	virtual void OnEventWrite();

	//virtual void OnHeaderComplete();
	void addCurl(EdCurl* pcurl);
	void open();
	void close();
	int request(const char* url);
	void setEvent(int evt);
	int sockCb(CURL* e, curl_socket_t s, int what);
	static void closeCurl();
	static size_t body_cb(void* ptr, size_t size, size_t nmemb, void* user);
	static size_t header_cb(void *buffer, size_t size, size_t nmemb, void *userp);
	virtual int OnBodyData(void* ptr, size_t size, size_t nmemb);

	const char* getRespHeader(char *name);

private:
	char* clean_str(char *str);
	void check_multi_info();

private:
	unordered_map<string, string> mRespHeaders;

protected:
	EdTimer mCurlTimer;
	CURLM *mCurlm;
	CURL *meCurl;
	curl_socket_t mCurlSockFd;
};



}

#endif /* EDCURLMULTI_H_ */
