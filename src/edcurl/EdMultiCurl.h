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
#include "../EdObjList.h"
#include "EdCurlSocket.h"

namespace edft {

using namespace std;

class EdCurl;

class EdMultiCurl:  public EdTimer::ITimerCb
{
	friend class EdCurl;
	friend class EdCurlSocket;
public:
	EdMultiCurl();
	virtual ~EdMultiCurl();
	static int curl_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
	int dgCurlSockCb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);

	static int multi_timer_cb(CURLM *multi, long timeout_ms, void *userp);
	virtual void IOnTimerEvent(EdTimer* ptimer);

	void addCurl(EdCurl* pcurl);
	void open();
	void close();
	int request(const char* url);
	void setEvent(int evt);
	int sockCb(CURL* e, curl_socket_t s, int what);
	static void closeCurl();
	const char* getRespHeader(char *name);

private:
	char* clean_str(char *str);
	void check_multi_info();
	void procEventRead(int fd);
	void procEventWrite(int fd);

private:
	unordered_map<string, string> mRespHeaders;
	EdObjList<EdCurlSocket> mSockList;

protected:
	EdTimer mCurlTimer;
	CURLM *mCurlm;

};

}

#endif /* EDCURLMULTI_H_ */
