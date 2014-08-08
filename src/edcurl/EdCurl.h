/*
 * EdCurl.h
 *
 *  Created on: Aug 8, 2014
 *      Author: netmind
 */

#ifndef EDCURL_H_
#define EDCURL_H_
#include <curl/curl.h>
//#include "EdMultiCurl.h"

namespace edft
{
class EdMultiCurl;

class EdCurl
{
friend class EdMultiCurl;
//friend class EdCurlFactory<>;
public:
	EdCurl();
	virtual ~EdCurl();
	void open(EdMultiCurl* pm);
	int request(const char* url);

private:
	CURL* mCurl;
	EdMultiCurl *mMCurl;

	size_t header_cb(void* buffer, size_t size, size_t nmemb, void* userp);
};



} /* namespace edft */

#endif /* EDCURL_H_ */
