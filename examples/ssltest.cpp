//============================================================================
// Name        : ssltest.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#define DBGTAG "*****"
#define DBG_LEVEL DBG_DEBUG

#include "edslog.h"
#include "EdNio.h"
#include "EdTask.h"
#include "EsFile.h"
#include "edssl/EdSSLSocket.h"

using namespace std;
using namespace edft;



class SslTask: public EdTask, public EdSSLSocket::ISSLSocketCb
{
	EdSSLSocket *mssl;
	EsFile wfile;

	virtual int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{
			dbgd("edm init.......");

			wfile.openFile("body.txt", EsFile::OPEN_RWTC);

			mssl = new EdSSLSocket();
			mssl->setSSLCallback(this);
			mssl->sslConnect("127.0.0.1");
			//mssl->sslConnect("173.194.72.94");
		}
		else if(pmsg->msgid == EDM_CLOSE)
		{
			dbgd("edm close.......");
			if(mssl)
			{
				mssl->sslClose();
				delete mssl;
			}
		}
		return 0;
	}

	virtual void IOnSSLSocket(EdSSLSocket *psock, int event)
	{
		if (event == SSL_EVENT_CONNECTED)
		{
			dbgd("ssl connected...");
			char msg[] = "GET /test.dat HTTP/1.1\r\n"
					"User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
					"Host: 127.0.0.1\r\n"
					//"Host: github.com\r\n"
					//"Host: www.google.co.kr\r\n"
					"Accept: */*\r\n"
					"\r\n";
			int wret = mssl->sslSend(msg, strlen(msg));
			dbgd(" send request, ret=%d", wret);

		}
		else if (event == SSL_EVENT_DISCONNECTED)
		{
			dbgd("ssl disconnected...");
			mssl->close();
			delete mssl;
			mssl = NULL;
		}
		else if(event == SSL_EVENT_READ)
		{
			static int recvcnt=0;
			char buf[256+1];
			int rdcnt = mssl->sslRecv(buf, 256);
			//dbgd("rdcnt=%d",rdcnt);
			if(rdcnt>0) {
				wfile.writeFile(buf, rdcnt);
				recvcnt += rdcnt;
			}

			if(recvcnt >= 159350056-203840)
				dbgd("rdcnt=%d", rdcnt);

		}
	}
};

void testsslclient()
{
	SslTask task;
	task.run();
	getchar();
	task.terminate();
}

int main()
{
	EdNioInit();
	EdSSLInit();
	testsslclient();
	return 0;
}
