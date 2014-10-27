/*
 * EdCurl.h
 *
 *  Created on: Aug 04, 2014
 *      Author: netmind
 */

#ifndef EDCURLMULTI_H_
#define EDCURLMULTI_H_

#include "../ednio_config.h"
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

	virtual void IOnTimerEvent(EdTimer* ptimer);

	void open();
	void close();
	int request(const char* url);
	const char* getRespHeader(char *name);
	CURLM* getMultiCurl();
	//void setPipelineing(int ispipeline);

private:
	char* clean_str(char *str);
	void startSingleCurl(EdCurl* pcurl);
	void check_multi_info();
	void setEvent(int evt);
	int sockCb(CURL* e, curl_socket_t s, int what);
	static void closeCurl();
	static int multi_timer_cb(CURLM *multi, long timeout_ms, void *userp);
	static int curl_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
	int dgCurlSockCb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
	void procEventRead(int fd);
	void procEventWrite(int fd);
	void procEventErr(int fd);

private:
	unordered_map<string, string> mRespHeaders;
	EdObjList<EdCurlSocket> mSockList;

protected:
	EdTimer mCurlTimer;
	CURLM *mCurlm;

};

}

#endif /* EDCURLMULTI_H_ */
