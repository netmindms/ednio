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

namespace edft
{

EdCurl::EdCurl()
{
	// TODO Auto-generated constructor stub

}

EdCurl::~EdCurl()
{
	// TODO Auto-generated destructor stub
}

void EdCurl::open(EdMultiCurl* pm)
{
	mCurl = curl_easy_init();
	mMCurl = pm;
	pm->addCurl(this);
}

int EdCurl::request(const char* url)
{
	curl_easy_setopt(mCurl, CURLOPT_URL, url);
	//curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, body_cb);
	//curl_easy_setopt(meCurl, CURLOPT_WRITEDATA, (void* )this);

	// for retrieving headers
	//curl_easy_setopt(meCurl, CURLOPT_HEADER, true);
	//curl_easy_setopt(meCurl, CURLOPT_HEADERFUNCTION, header_cb);
	//curl_easy_setopt(meCurl, CURLOPT_WRITEHEADER, this);
	//curl_easy_setopt(meCurl, CURLOPT_HEADERDATA, this);

	int run_handles;
	curl_multi_socket_action(mMCurl->mCurlm, CURL_SOCKET_TIMEOUT, 0, &run_handles);
	dbgd("new curl=%p, runhandle=%d", mCurl, run_handles);
	//check_multi_info();
	return 0;
}

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

} /* namespace edft */
