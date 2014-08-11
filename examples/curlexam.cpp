//============================================================================
// Name        : curlexam.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
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

			multi = new EdMultiCurl;
			dbgd("new multi=%p", multi);
			multi->open();
			edcurl = new EdCurl();
			dbgd("new alloc curl=%p", edcurl);
			edcurl->setCallback(this);
			edcurl->open(multi);
			CURL *curl = edcurl->getCurl();
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			//edcurl->setUrl("http://127.0.0.1");
			edcurl->setUrl("http://www.google.com");
			edcurl->request();
			edcurl->request("http://www.google.com");

		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			if(edcurl) {
				edcurl->close();
			}
			delete edcurl;
			delete multi;
		}
		else if(pmsg->msgid == EDM_TIMER)
		{
			dbgd("cancel timer.......");
			killTimer(1);
			edcurl->close();
		}
		return 0;
	}

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
	EdNioInit();
	testcurl();
	return 0;
}
