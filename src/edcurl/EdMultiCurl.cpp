/*
 * EdCurl.cpp
 *
 *  Created on: Aug 04, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "mcurl"

#include <stdexcept>
#include <string>
#include <string.h>

#include "EdMultiCurl.h"
#include "EdCurl.h"

#include "../edslog.h"

namespace edft
{

EdMultiCurl::EdMultiCurl()
{
	mCurlm = NULL;

}

EdMultiCurl::~EdMultiCurl()
{
	close();
}

void EdMultiCurl::IOnTimerEvent(EdTimer* ptimer)
{
	int runhandles;
	dbgd("time out...");
	curl_multi_socket_action(mCurlm, CURL_SOCKET_TIMEOUT, 0, &runhandles);
	check_multi_info();
}

void EdMultiCurl::open()
{
	mCurlTimer.setCallback(this);

	mCurlm = curl_multi_init();
	curl_multi_setopt(mCurlm, CURLMOPT_SOCKETFUNCTION, curl_sock_cb);
	curl_multi_setopt(mCurlm, CURLMOPT_SOCKETDATA, this);
	curl_multi_setopt(mCurlm, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
	curl_multi_setopt(mCurlm, CURLMOPT_TIMERDATA, this);
}

int EdMultiCurl::curl_sock_cb(CURL* e, curl_socket_t s, int what, void* cbp, void* sockp)
{
	EdMultiCurl *mc = (EdMultiCurl*) cbp;
	mc->dgCurlSockCb(e, s, what, cbp, sockp);
	return 0;
#if 0
	EdCurl *pcurl = NULL;
	curl_easy_getinfo(e, CURLINFO_PRIVATE, (char* )&pcurl);

	EdCurlSocket* cs = (EdCurlSocket*) sockp;
	if (sockp)
	{
		cs = mc->mSockList.allocObj();
		mc->mSockList.push_back(cs);
		dbgd("new curl sock, fd=%d, curlsock=%d", s, cs);
		curl_multi_assign(mc->mCurlm, s, cs);
	}

	dbgd("curl sock cb, s=%d, what=%d, e=%p, curl=%p", s, what, e, pcurl);
	cs->curlSockCb(what);
	return 0;
#endif

}

int EdMultiCurl::multi_timer_cb(CURLM* multi, long timeout_ms, void* userp)
{
	EdMultiCurl *pmulti = (EdMultiCurl*) userp;
	dbgd("multimer callback...ms=%ld", timeout_ms);
	pmulti->mCurlTimer.set(timeout_ms);
	return 0;
}

#if 0
int EdMultiCurl::sockCb(CURL* e, curl_socket_t s, int what)
{

	mCurlSockFd = s;

	if (mIsReg == false)
	{
		setFd(s);
	}

	if (what == CURL_POLL_NONE)
	{
		dbgd("fd=%d, curl poll none...", s);
	}
	else if (what == CURL_POLL_IN)
	{
		dbgd("fd=%d, curl poll in...", s);
		setEvent(EVT_READ);
	}
	else if (what == CURL_POLL_OUT)
	{
		dbgd("fd=%d, curl poll out...", s);
		setEvent(EVT_WRITE);
	}
	else if (what == CURL_POLL_INOUT)
	{
		dbgd("fd=%d, curl poll inout...", s);
		setEvent(EVT_WRITE | EVT_READ);
	}
	else if (what == CURL_POLL_REMOVE)
	{
		dbgd("fd=%d, curl poll remove...", s);
		deregisterEvent();
	}

	return 0;
}
#endif

void EdMultiCurl::close()
{
	dbgd("close.......");
	if (mCurlm)
	{
		curl_multi_cleanup(mCurlm);
		mCurlm = NULL;
	}

	mCurlTimer.kill();
}

const char* EdMultiCurl::getRespHeader(char* name)
{
//	string ps = mRespHeaders.A
//	if(ps) {
//		return ps->c_str();
//	}
//	else
//		return NULL;
	try
	{
		string &s = mRespHeaders.at(name);
		return s.c_str();
	} catch (out_of_range &range_err)
	{

		return NULL;
	}
}

void EdMultiCurl::check_multi_info()
{
	int msgcount;
	CURLMsg* msg;
	while ((msg = curl_multi_info_read(mCurlm, &msgcount)))
	{
		dbgv("msg ptr=%p, msg=%d, curl=%p, cnt=%d", msg, msg->msg, msg->easy_handle, msgcount);
		if (msg->msg == CURLMSG_DONE)
		{
			curl_multi_remove_handle(mCurlm, msg->easy_handle);
			EdCurl *pcurl = NULL;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, (char* )&pcurl);
			if (pcurl)
				pcurl->procCurlDone(msg->data.result);
		}
	}
}

void EdMultiCurl::addCurl(EdCurl* pcurl)
{
	curl_multi_add_handle(mCurlm, pcurl->mCurl);
	int run_handles;
	curl_multi_socket_action(mCurlm, CURL_SOCKET_TIMEOUT, 0, &run_handles);
}

void EdMultiCurl::procEventRead(int fd)
{
	int runhandles;
	curl_multi_socket_action(mCurlm, fd, CURL_CSELECT_IN, &runhandles);
	dbgd("on read event, run=%d", runhandles);
	check_multi_info();
}

void EdMultiCurl::procEventWrite(int fd)
{
	int runhandles;
	curl_multi_socket_action(mCurlm, fd, CURL_CSELECT_OUT, &runhandles);
	dbgd("on wrte event, run=%d", runhandles);
	check_multi_info();
}

int EdMultiCurl::dgCurlSockCb(CURL* e, curl_socket_t s, int what, void* cbp, void* sockp)
{
	EdCurl *pcurl = NULL;
	curl_easy_getinfo(e, CURLINFO_PRIVATE, (char* )&pcurl);

	EdCurlSocket* cs = (EdCurlSocket*) sockp;
	if (sockp == NULL)
	{
		cs = mSockList.allocObj();
		cs->open(this, s);
		mSockList.push_back(cs);
		dbgd("new curl sock, fd=%d, curlsock=%d", s, cs);
		curl_multi_assign(mCurlm, s, cs);
	}

	dbgd("curl sock cb, s=%d, what=%d, e=%p, curl=%p", s, what, e, pcurl);
	cs->curlSockCb(what);
	if(what == CURL_POLL_REMOVE)
	{
		mSockList.remove(cs);
		mSockList.freeObj(cs);
		dbgd("  sock list cnt=%d", mSockList.size());
	}
	return 0;
}

} // namespae edft
