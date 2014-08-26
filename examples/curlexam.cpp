//============================================================================
// Name        : curlexam.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Asynchronous Curl Example with libednio
//============================================================================

#define DBGTAG "appcl"
#define DBG_LEVEL DBG_DEBUG
#include <string>
#include <algorithm>

#include "edslog.h"
#include "EdNio.h"
#include "EdTask.h"
#include "EdObjList.h"
#include "edssl/EdSSL.h"
#include "edssl/EdSSLSocket.h"
#include "edcurl/EdCurl.h"
#include "edcurl/EdMultiCurl.h"

using namespace std;
using namespace edft;

class MainTask: public EdTask, public EdCurl::ICurlCb
{
	EdMultiCurl* multi;
	EdCurl *edcurl;
	virtual int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{

			// create multi curl
			multi = new EdMultiCurl;
			multi->open();

			// create single curl to register to mutli curl
			edcurl = new EdCurl();
			// To get headers and body data and curl status, set call back.
			edcurl->setCallback(this);
			edcurl->open(multi);

			// You may set your specific curl option by getting curl handle.
			//CURL *curl = edcurl->getCurl();
			//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

			// set url
			edcurl->setUrl("http://127.0.0.1");
			edcurl->request();

		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			if(edcurl) {
				edcurl->close();
			}
			delete edcurl;

			if(multi) {
				multi->close();
			}
			delete multi;
		}

		return 0;
	}


	// implements ICurlCb interface abstract functions.
	virtual void IOnCurlStatus(EdCurl* pcurl, int status)
	{
		dbgd("curl result, code=%d", status);
		pcurl->close();
		delete pcurl;
		edcurl = NULL;
	}
	virtual void IOnCurlHeader(EdCurl* pcurl)
	{
		dbgd("response code=%03d", pcurl->getResponseCode());

		const char *val = pcurl->getHeader("Content-Type");
		if(val)
			dbgd("Content-Type=%s", val);


	}
	virtual void IOnCurlBody(EdCurl* pcurl, void* ptr, int size)
	{
		dbgd("body data, cnt=%d", size);
	}

};

void testcurl()
{
	MainTask maintask;
	maintask.run();
	getchar();
	maintask.terminate();
}

int main()
{
	EdCurl* curl = new EdCurl;
	void *ptr = curl;
	delete ptr;
	return 0;



	EdNioInit();
	testcurl();
	return 0;
}
