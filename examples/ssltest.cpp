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
#include "edssl/EdSSL.h"
#include "EdNio.h"
#include "EdTask.h"
#include "edssl/EdSSLSocket.h"

using namespace std;
using namespace edft;

class SslTask: public EdTask, public EdSSLSocket::ISSLSocketCb
{
	EdSSLSocket *mssl;
	SSL_CTX *mSSLCtx;

	virtual int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{
			dbgd("edm init.......");


			mssl = new EdSSLSocket();
			mSSLCtx = EdSSL::buildClientCtx(SSL_VER_TLSV1);
			mssl->openSSLClientSock(mSSLCtx);
			mssl->setSSLCallback(this);
			mssl->connect("127.0.0.1", 8000);
		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			dbgd("edm close.......");
			if (mssl)
			{
				mssl->close();
				delete mssl;
			}

			EdSSL::freeCtx(mSSLCtx);
		}
		return 0;
	}

	virtual void IOnSSLSocket(EdSSLSocket *psock, int event)
	{
		if (event == SSL_EVENT_CONNECTED)
		{
			dbgd("ssl connected...");
			check_cert(psock->getSSL(), "netsvr");
			if(1)
			{
				char msg[] = "GET /test.dat HTTP/1.1\r\n"
						"User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
						"Host: 127.0.0.1\r\n"
						//"Host: github.com\r\n"
						//"Host: www.google.co.kr\r\n"
						"Accept: */*\r\n"
						"\r\n";
				int wret = mssl->send(msg, strlen(msg));
				dbgd(" send request, ret=%d", wret);
			}


		}
		else if (event == SSL_EVENT_DISCONNECTED)
		{
			dbgd("ssl disconnected...");
			mssl->close();
			delete mssl;
			mssl = NULL;
		}
		else if (event == SSL_EVENT_READ)
		{
			static int recvcnt = 0;
			char buf[256 + 1];
			int rdcnt = mssl->recv(buf, 256);

			if (rdcnt > 0)
			{
				recvcnt += rdcnt;
			}

			if (recvcnt >= 159350056 - 203840)
				dbgd("rdcnt=%d", rdcnt);

		}
	}

	void check_cert(SSL *ssl, char *host)
	{
		X509 *peer;
		char peer_CN[256];
		if (SSL_get_verify_result(ssl) != X509_V_OK)
			printf("### Certificate doesn’t verify\n");

		/*Check the cert chain. The chain length

		 is automatically checked by OpenSSL when

		 we set the verify depth in the ctx */

		/*Check the common name*/

		peer = SSL_get_peer_certificate(ssl);
		X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, 256);

		if (strcasecmp(peer_CN, host))
			printf("### Common name doesn’t match host name, peer=%s\n", peer_CN);
		X509_free(peer);

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
