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
}

int EdMultiCurl::multi_timer_cb(CURLM* multi, long timeout_ms, void* userp)
{
	EdMultiCurl *pmulti = (EdMultiCurl*) userp;
	dbgd("curl timer callback...ms=%ld, curl_user=%p", timeout_ms, userp);
	pmulti->mCurlTimer.set(timeout_ms);
	return 0;
}


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

void EdMultiCurl::check_multi_info()
{
	int msgcount;
	CURLMsg* msg;
	while ((msg = curl_multi_info_read(mCurlm, &msgcount)))
	{
		dbgv("check curl msg ptr=%p, msg=%d, curl=%p, cnt=%d", msg, msg->msg, msg->easy_handle, msgcount);
		if (msg->msg == CURLMSG_DONE)
		{
			EdCurl *pcurl = NULL;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, (char* )&pcurl);
			int curlcode = msg->data.result;

			// Warning: After removing easy handle from multihandle, msg is unavailable.
			// Therefore, you must not access the curlmsg since that.
			curl_multi_remove_handle(mCurlm, msg->easy_handle);
			if (pcurl)
				pcurl->procCurlDone(curlcode);

		}
	}
}

void EdMultiCurl::startSingleCurl(EdCurl* pcurl)
{
	curl_multi_add_handle(mCurlm, pcurl->mCurl);
	int run_handles;
	CURLMcode  code = curl_multi_socket_action(mCurlm, CURL_SOCKET_TIMEOUT, 0, &run_handles);
	dbgd("action code=%d", code);
	check_multi_info();
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

void EdMultiCurl::procEventErr(int fd)
{
	int runhandles;
	curl_multi_socket_action(mCurlm, fd, CURL_CSELECT_ERR, &runhandles);
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
		dbgd("new curl sock, fd=%d, curlsock=%x", s, cs);
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


CURLM* EdMultiCurl::getMultiCurl()
{
	return mCurlm;
}



} // namespae edft
