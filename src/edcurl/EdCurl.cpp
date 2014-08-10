/*
 * EdCurl.cpp
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */
#define DBGTAG "edcrl"
#define DBG_LEVEL DBG_DEBUG
#include "EdCurl.h"
#include "EdMultiCurl.h"
#include "../edslog.h"
#include <algorithm>

using namespace std;

namespace edft
{

EdCurl::EdCurl()
{
	mCurl = NULL;
	mEdMultiCurl = NULL;
	mIsRespHeaderComp = false;
}

EdCurl::~EdCurl()
{
	close();
}

void EdCurl::open(EdMultiCurl* pm)
{
	mCurl = curl_easy_init();
	curl_easy_setopt(mCurl, CURLOPT_PRIVATE, this);
	curl_easy_setopt(mCurl, CURLOPT_HEADERFUNCTION, header_cb);
	curl_easy_setopt(mCurl, CURLOPT_WRITEHEADER, this);
	//curl_easy_setopt(mCurl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP);
//	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, body_cb);
//	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, (void* )this);

//curl_easy_setopt(mCurl, CURLOPT_FOLLOWLOCATION, 1);

	mEdMultiCurl = pm;
	//pm->addCurl(this);
}

int EdCurl::request(const char* url)
{
	setUrl(url);
	return request();
}

size_t EdCurl::header_cb(void* buffer, size_t size, size_t nmemb, void* userp)
{
	EdCurl *pcurl = (EdCurl*) userp;
	dbgd("header: size=%d, n=%d", size, nmemb);
	size_t len = size * nmemb;

	if (pcurl->mIsRespHeaderComp == true)
		return len;
	if (len <= 2)
	{
		pcurl->mIsRespHeaderComp = true;
		pcurl->OnHeaderComplete();
	}

	char buf[512];
	memcpy(buf, buffer, size * nmemb);
	buf[size * nmemb] = 0;
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
	if(delc==':' && ptr)
	{
		//mRespHeaders[dest] =
		char *rp=bufname;
		for(rp=bufname+datalen-1;*rp==' ';rp--);
		*(rp+1) = 0;

		for(;*ptr == ' ';ptr++);
		ptr = krstrtoken(bufval, ptr, "\r\n", &delc, &datalen, len-1);
		curl->mRespHeaders[bufname] = bufval;
	}
	else
	{
		if(*bufname)
		curl->mRespHeaders["resp"] = bufname;
	}
	free(bufname);
	free(bufval);
	free(tbuf);

	//dbgv("header name=%s, val=%s", dest, val);

	//dbgv("    header=%s", hs.c_str());
#endif
	return nmemb * size;
}

size_t EdCurl::body_cb(void* ptr, size_t size, size_t nmemb, void* user)
{
	EdCurl* curl = (EdCurl*) user;
	char *buf;
	size_t len = size * nmemb;
	dbgd("body cb, len=%d", len);
	OnBodyData(ptr, len);
	return len;
}

void EdCurl::close()
{
	if (mEdMultiCurl)
	{
		curl_multi_remove_handle(mEdMultiCurl, mCurl);
		mEdMultiCurl = NULL;
	}

	if (mCurl)
	{
		curl_easy_cleanup(mCurl);
		mCurl = NULL;
	}
}

void EdCurl::setCallback(ICurlStatusCb* cb)
{
	mCallback = cb;
}

CURL* EdCurl::getCurl()
{
	return mCurl;
}

CURLM* EdCurl::getMultiCurl()
{
	return mEdMultiCurl;
}

void EdCurl::OnHeaderComplete()
{
	dbgd("OnHeaderComplete...");
}

void EdCurl::OnCurlEnd(int errcode)
{
	dbgd("OnCurlEnd...");
}

void EdCurl::OnBodyData(void* buf, int len)
{

}

void EdCurl::setUrl(const char* url)
{
	curl_easy_setopt(mCurl, CURLOPT_URL, url);
}

int EdCurl::request()
{
	int run_handles;
	//curl_multi_socket_action(mMCurl->mCurlm, CURL_SOCKET_TIMEOUT, 0, &run_handles);
	mEdMultiCurl->addCurl(this);
	dbgd("new curl=%p, runhandle=%d", mCurl, run_handles);
	return 0;
}

void EdCurl::procCurlDone(int result)
{
	dbgd("curl done..........result=%d", result);
	OnCurlEnd(result);
}

} /* namespace edft */
