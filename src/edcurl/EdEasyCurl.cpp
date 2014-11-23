
/*
 * EdCurl.cpp
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */
#define DBGTAG "edcrl"
#define DBG_LEVEL DBG_WARN

#include <string.h>

#include "EdEasyCurl.h"
#include "EdMultiCurl.h"
#include "../edslog.h"
#include <algorithm>

using namespace std;

namespace edft
{

EdEasyCurl::EdEasyCurl()
{
	mCurl = NULL;
	mEdMultiCurl = NULL;
	mIsRespHeaderComp = false;
	//mCallback = NULL;
	mOnResult = NULL;
	mOnHeader = NULL;
	mOnBody = NULL;
	mRespTimer = NULL;
	mUser = NULL;
	mStatus = EDCURL_IDLE;
}

EdEasyCurl::~EdEasyCurl()
{
	close();
}

void EdEasyCurl::open(EdMultiCurl* pm)
{
	mCurl = curl_easy_init();
	setCurlCallback();
	//curl_easy_setopt(mCurl, CURLOPT_VERBOSE, 1);

	//curl_easy_setopt(mCurl, CURLOPT_FOLLOWLOCATION, 1);

	mEdMultiCurl = pm;

	mRespTimer = new EdTimer;
	mRespTimer->setOnListener(this);
}

int EdEasyCurl::request(const char* url, int cnn_timeout_sec)
{
	setUrl(url);
	return request(cnn_timeout_sec);
}

size_t EdEasyCurl::header_cb(void* buffer, size_t size, size_t nmemb, void* userp)
{
	EdEasyCurl *pcurl = (EdEasyCurl*) userp;
	return pcurl->dgheader_cb((char*) buffer, size, nmemb, userp);
}

size_t EdEasyCurl::body_cb(void* ptr, size_t size, size_t nmemb, void* user)
{
	EdEasyCurl* pcurl = (EdEasyCurl*) user;
	size_t len = size * nmemb;
	dbgd("curl body callback, size=%d, curl=%p", len, (void* )pcurl);
	pcurl->OnCurlBody(ptr, len);
	return len;
}

void EdEasyCurl::close()
{
	if(mCurl == NULL)
		return;

	reset();
	if (mCurl != NULL)
	{
		curl_easy_cleanup(mCurl);
		mCurl = NULL;
	}
	mEdMultiCurl = NULL;

	if(mRespTimer != NULL)
	{
		delete mRespTimer; mRespTimer = NULL;
	}
}

void EdEasyCurl::reset()
{
	if (mStatus != EDCURL_IDLE)
	{
		curl_multi_remove_handle(mEdMultiCurl, mCurl);
		curl_easy_reset(mCurl);
		setCurlCallback();
	}

	mRespTimer->kill();
	mStatus = EDCURL_IDLE;

	mHeaderList.clear();
	mIsRespHeaderComp = false;

}

/*
 void EdCurl::setOnListener(ICurlCb* cb)
 {
 mCallback = cb;
 }
 */

CURL* EdEasyCurl::getCurl()
{
	return mCurl;
}

CURLM* EdEasyCurl::getMultiCurl()
{
	return mEdMultiCurl->getMultiCurl();
}

void EdEasyCurl::OnCurlHeader()
{
	if (mOnHeader != NULL)
		mOnHeader->IOnCurlHeader(this);
	//mCallback->IOnCurlHeader(this);
//	dbgd("OnHeaderComplete...");
//	long code;
//	curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, &code);
//	char codes[4];
//	dbgd("   resp code=%s", convCodeToStr(codes, code));
}

void EdEasyCurl::OnCurlEnd(int errcode)
{
	dbgd("OnCurlEnd...");
	if (mOnResult != NULL)
		mOnResult->IOnCurlResult(this, errcode);
}

void EdEasyCurl::OnCurlBody(void* buf, int len)
{
	if (mOnBody != NULL)
	{
		mOnBody->IOnCurlBody(this, buf, len);
	}
}

void EdEasyCurl::setUrl(const char* url)
{
	curl_easy_setopt(mCurl, CURLOPT_URL, url);
}

int EdEasyCurl::request(int cnn_timeout_sec)
{
	dbgd("request edcurl=%p, handle=%p", this, mCurl);
	if (mStatus != EDCURL_IDLE)
	{
		dbgd("    ### already curl working...");
		return -1;
	}
	mStatus = EDCURL_REQUESTING;

	mRespTimer->set(cnn_timeout_sec * 1000); // this must be called before curl action.
	mEdMultiCurl->startSingleCurl(this);

	return 0;
}

const char* EdEasyCurl::getHeader(const char* name)
{
	string n(name);
	std::transform(n.begin(), n.end(), n.begin(), ::toupper);

	auto itr = mHeaderList.find(n);
	if (itr != mHeaderList.end())
		return itr->second.c_str();
	else
		return NULL;
}

int EdEasyCurl::getResponseCode()
{
	long code;
	curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, &code);
	return (int) code;
}

void EdEasyCurl::setUser(void* user)
{
	mUser = user;
}

void* EdEasyCurl::getUser()
{
	return mUser;
}

void EdEasyCurl::procCurlDone(int result)
{
	dbgd("curl result msg=%d", result);
	mRespTimer->kill();
	OnCurlEnd(result);
}

char* EdEasyCurl::convCodeToStr(char* buf, int code)
{
	int i, s, m;
	for (i = 0, s = code; i < 3; i++)
	{
		m = s % 10;
		s /= 10;
		buf[2 - i] = '0' + m;
	}
	buf[3] = 0;
	return buf;
}

size_t EdEasyCurl::dgheader_cb(char* buffer, size_t size, size_t nmemb, void* userp)
{
	dbgv("header: size=%d, n=%d", size, nmemb);
	size_t len = size * nmemb;

	if (mStatus == EDCURL_REQUESTING)
	{
		mStatus = EDCURL_RESPONSED;
		mRespTimer->kill();
	}

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
		if (!name_end)
			goto parser_end;

		start_val = name_end + 1;
		for (; *start_val == ' '; start_val++)
			;

		// trim name
		for (name_end--; *name_end == ' '; name_end--)
			;
		string hd_name(buffer, name_end - buffer + 1);
		std::transform(hd_name.begin(), hd_name.end(), hd_name.begin(), ::toupper);

		// trim header val
		char* val_end = buffer + len - 1;
		for (; *val_end == '\r' || *val_end == '\n'; val_end--)
			;
		string hd_val(start_val, val_end + 1);

		mHeaderList[hd_name] = hd_val;
		dbgv("Header, name=%s, val=%s", hd_name.c_str(), hd_val.c_str());

	}
	parser_end: return len;
}

void EdEasyCurl::IOnTimerEvent(EdTimer* ptimer)
{
	if (ptimer == mRespTimer)
	{
		dbgd("connection timeout, curl=%x", this);
		ptimer->kill();
		OnCurlEnd(-1000);
	}
	else
	{
		dbge("### invalid curl timer...");
		assert(0);
	}
}

void EdEasyCurl::setOnCurlListener(ICurlResult* iresult, ICurlBody* ibody, ICurlHeader* iheader)
{
	mOnResult = iresult;
	mOnBody = ibody;
	mOnHeader = iheader;
}

void EdEasyCurl::setCurlCallback()
{
	curl_easy_setopt(mCurl, CURLOPT_PRIVATE, this);
	curl_easy_setopt(mCurl, CURLOPT_HEADERFUNCTION, header_cb);
	curl_easy_setopt(mCurl, CURLOPT_WRITEHEADER, this);

	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, body_cb);
	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, (void* )this);
}


long EdEasyCurl::getContentLength()
{
	const char* ptr = getHeader("Content-Length");
	if(ptr != NULL)
	{
		return atol(ptr);
	}
	else
	{
		// In transferring as chunked encoding method, Content-Length doesn't exist.
		// So, if there is no Content_Length, return negative
		return -1;
	}
}

} /* namespace edft */
