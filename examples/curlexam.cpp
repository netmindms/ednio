//============================================================================
// Name        : curlexam.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#define DBGTAG "appcl"
#define DBG_LEVEL DBG_DEBUG

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

class MainTask: public EdTask, public EdCurl::ICurlStatusCb
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
			//edcurl->request("http://www.google.com");
			//edcurl->request("http://www.naver.com");
			edcurl->request("http://www.yahoo.com");
			//edcurl->request("http://127.0.0.1");
			//setTimer(1, 1, 1);

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
