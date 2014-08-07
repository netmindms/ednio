/*
 * EdCurl.cpp
 *
 *  Created on: Aug 04, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "ecurl"

#include <stdexcept>
#include <string>
#include <string.h>

#include "EdCurl.h"
#include "../edslog.h"

namespace edft
{

__thread CURLM* _tCurlM = NULL;

EdCurl::EdCurl()
{
	meCurl = NULL;
	mCurlSockFd = -1;
	mCurlm = NULL;

}

EdCurl::~EdCurl()
{
	close();
}

void EdCurl::IOnTimerEvent(EdTimer* ptimer)
{
	int runhandles;
	dbgd("time out...");
	curl_multi_socket_action(mCurlm, mFd, CURL_SOCKET_TIMEOUT, &runhandles);
}

void EdCurl::OnEventRead()
{
	int runhandles;
	curl_multi_socket_action(mCurlm, mFd, CURL_CSELECT_IN, &runhandles);
}

void EdCurl::OnEventWrite()
{
	//logd("on event write...");
	int runhandles;
	curl_multi_socket_action(mCurlm, mFd, CURL_CSELECT_OUT, &runhandles);
}

void EdCurl::open()
{
	setDefaultContext();

	mCurlTimer.setCallback(this);

	if (_tCurlM == NULL)
		_tCurlM = curl_multi_init();
	mCurlm = _tCurlM;

	curl_multi_setopt(mCurlm, CURLMOPT_SOCKETFUNCTION, curl_sock_cb);
	curl_multi_setopt(mCurlm, CURLMOPT_SOCKETDATA, this);
	curl_multi_setopt(mCurlm, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
	curl_multi_setopt(mCurlm, CURLMOPT_TIMERDATA, this);
}

int EdCurl::curl_sock_cb(CURL* e, curl_socket_t s, int what, void* cbp, void* sockp)
{
	EdCurl *c = (EdCurl*) cbp;
	dbgd("curl sock cb, s=%d, what=%d, e=%p", s, what, e);
	return c->sockCb(e, s, what);

}

int EdCurl::multi_timer_cb(CURLM* multi, long timeout_ms, void* userp)
{
	EdCurl *pcurl = (EdCurl*) userp;
	dbgd("multimer callback...ms=%ld", timeout_ms);
	pcurl->mCurlTimer.set(timeout_ms);
	pcurl->check_multi_info();
	return 0;
}

void EdCurl::setEvent(int evt)
{
	if (mIsReg)
		changeEvent(evt);
	else
		registerEvent(evt);
}

int EdCurl::request(const char* url)
{
	meCurl = curl_easy_init();
	curl_easy_setopt(meCurl, CURLOPT_URL, url);
	curl_easy_setopt(meCurl, CURLOPT_WRITEFUNCTION, body_cb);
	curl_easy_setopt(meCurl, CURLOPT_WRITEDATA, (void*)this);

	// for retrieving headers
	//curl_easy_setopt(meCurl, CURLOPT_HEADER, true);
	curl_easy_setopt(meCurl, CURLOPT_HEADERFUNCTION,  header_cb);
	//curl_easy_setopt(meCurl, CURLOPT_WRITEHEADER, this);
	curl_easy_setopt(meCurl, CURLOPT_HEADERDATA, this);

	curl_multi_add_handle(mCurlm, meCurl);
	int run_handles;
	curl_multi_socket_action(mCurlm, CURL_SOCKET_TIMEOUT, 0, &run_handles);
	dbgd("new curl=%p", meCurl);
	return 0;
}

int EdCurl::sockCb(CURL* e, curl_socket_t s, int what)
{

	mCurlSockFd = s;

	if (mIsReg == false)
	{
		setFd(s);
	}

	if (what == CURL_POLL_NONE)
	{

	}
	else if (what == CURL_POLL_IN)
	{
		setEvent(EVT_READ);
	}
	else if (what == CURL_POLL_OUT)
	{
		setEvent(EVT_WRITE);
	}
	else if (what == CURL_POLL_INOUT)
	{
		setEvent(EVT_WRITE | EVT_READ);
	}
	else if (what == CURL_POLL_REMOVE)
	{
		deregisterEvent();
	}

	check_multi_info();

	return 0;
}

void EdCurl::closeCurl()
{
	if(_tCurlM) {
		curl_multi_cleanup(_tCurlM);
		_tCurlM = NULL;
	}
}

size_t EdCurl::body_cb(void* ptr, size_t size, size_t nmemb, void* user)
{
	EdCurl* curl = (EdCurl*)user;
	return curl->OnBodyData(ptr, size, nmemb);
}

/*
 * header callback is called not only for original header but also for trailer of chunked-encoding transfering.
 * must separate between orignal header and trailer of body.
 */
size_t EdCurl::header_cb(void* buffer, size_t size, size_t nmemb, void* userp)
{
	EdCurl *curl = (EdCurl*)userp;

	dbgd("header: size=%d, n=%d", size, nmemb);
	char buf[512];
	memcpy(buf, buffer, size*nmemb);
	buf[size*nmemb]=0;
	dbgd("    str=%s", buf);

#if 0

	size_t len = size * nmemb;
	char *bufname = (char*)malloc(len);
	char *bufval = (char*)malloc(len);
	char *tbuf = (char*)malloc(len);
	*bufname = 0;
	*bufval = 0;

	char delc;
	char *ptr;
	int datalen;
	memcpy(tbuf, buffer, len);
	ptr = tbuf;
	ptr = krstrtoken(bufname, ptr, ":\r\n", &delc, &datalen, len-1);
	if(delc==':' && ptr) {
		//mRespHeaders[dest] =
		char *rp=bufname;
		for(rp=bufname+datalen-1;*rp==' ';rp--);
		*(rp+1) = 0;

		for(;*ptr == ' ';ptr++);
		ptr = krstrtoken(bufval, ptr, "\r\n", &delc, &datalen, len-1);
		curl->mRespHeaders[bufname] = bufval;
	}
	else {
		if(*bufname)
			curl->mRespHeaders["resp"] = bufname;
	}
	free(bufname);
	free(bufval);
	free(tbuf);

	//dbgv("header name=%s, val=%s", dest, val);

	//dbgv("    header=%s", hs.c_str());
#endif
	return nmemb*size;
}

int EdCurl::OnBodyData(void* ptr, size_t size, size_t nmemb)
{
	//char *buf =(char*)malloc(size*nmemb);
	//memcpy(buf, ptr, size*nmemb);
	dbgd("body data len=%d", size*nmemb);
	return (size*nmemb);
}

void EdCurl::close()
{
	dbgd("close.......");
	if(meCurl) {
		curl_easy_cleanup(meCurl);
		meCurl = NULL;
	}
}

const char* EdCurl::getRespHeader(char* name)
{
//	string ps = mRespHeaders.A
//	if(ps) {
//		return ps->c_str();
//	}
//	else
//		return NULL;
	try {
		string &s  = mRespHeaders.at(name);
		return s.c_str();
	} catch(out_of_range &range_err) {

		return NULL;
	}
}


void EdCurl::check_multi_info()
{
	int msgcount;
	CURLMsg* msg;
	dbgd("check complete msg");
	for(msg = curl_multi_info_read(mCurlm, &msgcount);msg;)
	{
		dbgd("msg ptr=%p, msg=%d, curl=%p, cnt=%d", msg, msg->msg, msg->easy_handle, msgcount);
		if(msg->msg == CURLMSG_DONE)
		{
			curl_multi_remove_handle(mCurlm, msg->easy_handle);
			//curl_easy_cleanup(msg->easy_handle);
		}
	}
}

} // namespae kes2
