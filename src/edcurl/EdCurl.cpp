/*
 * EdCurl.cpp
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */
#define DBGTAG "edcrl"
#define DBG_LEVEL DBG_WARN

#include <string.h>

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
	mCallback = NULL;
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

	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, body_cb);
	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, (void* )this);

	//curl_easy_setopt(mCurl, CURLOPT_FOLLOWLOCATION, 1);

	mEdMultiCurl = pm;
}

int EdCurl::request(const char* url)
{
	setUrl(url);
	return request();
}

size_t EdCurl::header_cb(void* buffer, size_t size, size_t nmemb, void* userp)
{
	EdCurl *pcurl = (EdCurl*) userp;
	return pcurl->dgheader_cb((char*)buffer, size, nmemb, userp);
}

size_t EdCurl::body_cb(void* ptr, size_t size, size_t nmemb, void* user)
{
	EdCurl* pcurl = (EdCurl*) user;
	size_t len = size * nmemb;
	dbgd("curl body callback, size=%d, curl=%p", len, (void*)pcurl);
	pcurl->OnCurlBody(ptr, len);
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

	mHeaderList.clear();
}

void EdCurl::setCallback(ICurlCb* cb)
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

void EdCurl::OnCurlHeader()
{
	if(mCallback)
		mCallback->IOnCurlHeader(this);
//	dbgd("OnHeaderComplete...");
//	long code;
//	curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, &code);
//	char codes[4];
//	dbgd("   resp code=%s", convCodeToStr(codes, code));
}

void EdCurl::OnCurlEnd(int errcode)
{
	dbgd("OnCurlEnd...");
}

void EdCurl::OnCurlBody(void* buf, int len)
{
	if(mCallback)
		mCallback->IOnCurlBody(this, buf, len);
}

void EdCurl::setUrl(const char* url)
{
	curl_easy_setopt(mCurl, CURLOPT_URL, url);
}

int EdCurl::request()
{
	//curl_multi_socket_action(mMCurl->mCurlm, CURL_SOCKET_TIMEOUT, 0, &run_handles);
	mEdMultiCurl->addCurl(this);
	dbgd("new curl=%p", mCurl);
	return 0;
}

const char* EdCurl::getHeader(const char* name)
{
	string n(name);
	std::transform(n.begin(), n.end(),n.begin(), ::toupper);

	auto itr = mHeaderList.find(n);
	if(itr != mHeaderList.end())
		return itr->second.c_str();
	else
		return NULL;
}

int EdCurl::getResponseCode()
{
	long code;
	curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, &code);
	return (int)code;
}

void EdCurl::procCurlDone(int result)
{
	dbgd("curl result msg=%d", result);
	OnCurlEnd(result);
}


char* EdCurl::convCodeToStr(char* buf, int code)
{
	int i,s,m;
	for(i=0,s=code;i<3;i++)
	{
		m=s%10;
		s /= 10;
		buf[2-i] = '0'+m;
	}
	buf[3] = 0;
	return buf;
}


size_t EdCurl::dgheader_cb(char* buffer, size_t size, size_t nmemb, void* userp)
{
	dbgv("header: size=%d, n=%d", size, nmemb);
	size_t len = size * nmemb;

	if (mIsRespHeaderComp == true)
		return len;

	if (len <= 2)
	{
		dbgd("header end match");
		mIsRespHeaderComp = true;
		OnCurlHeader();
	}
	else
	{
		char* name_end = buffer;
		char* start_val;
		name_end = strchr(buffer, ':');
		if(!name_end)
			goto parser_end;

		start_val = name_end+1;
		for(;*start_val==' ';start_val++);

		// trim name
		for(name_end--;*name_end==' ';name_end--);
		string hd_name(buffer, name_end-buffer+1);
		std::transform(hd_name.begin(), hd_name.end(),hd_name.begin(), ::toupper);

		// trim header val
		char* val_end = buffer+len-1;
		for(;*val_end=='\r' || *val_end=='\n';val_end--);
		string hd_val(start_val, val_end+1);

		mHeaderList[hd_name] = hd_val;
		dbgd("Header, name=%s, val=%s", hd_name.c_str(), hd_val.c_str());

	}
parser_end:
	return len;
}

} /* namespace edft */
