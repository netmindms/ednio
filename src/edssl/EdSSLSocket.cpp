/*
 * EdSSLSocket.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_WARN
#define DBGTAG "sslsc"
#include "../edslog.h"
#include "../EdNio.h"
#include "EdSSLSocket.h"

#include <openssl/err.h>

namespace edft
{

EdSSLSocket::EdSSLSocket()
{
	mSSLCtx = NULL;
	mSSL = NULL;
	mSessionConencted = false;
	mSSLCallback = NULL;
}

EdSSLSocket::~EdSSLSocket()
{
	if (mSSL)
	{
		SSL_free(mSSL);
		mSSL = NULL;
	}
}

void EdSSLSocket::OnRead()
{
	procSSLRead();
}

void EdSSLSocket::OnWrite()
{
}

void EdSSLSocket::OnConnected()
{
	dbgd("socket connected...");
	startHandshake();
}

void EdSSLSocket::OnDisconnected()
{
	dbgd("socket disconnected...");
	sslClose();

	if (mSSLCallback)
		mSSLCallback->IOnSSLSocket(this, SSL_EVENT_DISCONNECTED);
}

void EdSSLSocket::startHandshake()
{
	dbgd("start handshake...");
	mSSLCtx = getContext()->sslCtx;

	if (mSSL == NULL && mSessionConencted == false)
	{

		mSSL = SSL_new(mSSLCtx);
		SSL_set_fd(mSSL, getFd());
/*
		int cret = SSL_connect(mSSL);
		int err = SSL_get_error(mSSL, cret);
		changeSSLSockEvent(err, false);
		dbgd("     start ssl_connect, cret=%d, err=%d", cret, err);
		*/
		procSSLConnect();
	}
}

void EdSSLSocket::changeSSLSockEvent(int err, bool bwrite)
{
	if (err == SSL_ERROR_WANT_WRITE)
	{
		changeEvent(EVT_WRITE);
		dbgd("change write event.......");
	}
	else if (err == SSL_ERROR_WANT_READ)
	{
		changeEvent(EVT_READ);
		dbgd("change read event.......");
	}
	else if (err == SSL_ERROR_ZERO_RETURN)
	{
		dbgd("zero return...");
	}
}

int EdSSLSocket::sslRecv(void* buf, int bufsize)
{

	int rret = SSL_read(mSSL, buf, bufsize);
	if (rret < 0)
	{
		int err = SSL_get_error(mSSL, rret);
		dbgd("sslRecv,rret=%d, err=%d", rret, err);
		changeSSLSockEvent(err, false);
		if (err == SSL_ERROR_ZERO_RETURN)
		{
			SSL_shutdown(mSSL);
			postReserveDisconnect();
		}
		return -1;
	}
	else if (rret == 0)
	{
		int err = SSL_get_error(mSSL, rret);
		dbgd(" rret is zero, err=%d", err);
		SSL_shutdown(mSSL);
		postReserveDisconnect();
		return 0;
	}
	else
	{
		return rret;
	}
}

void EdSSLSocket::procSSLRead(void)
{
	dbgd("proc ssl read...");

	if (mSessionConencted == false)
	{
		procSSLConnect();
	}
	else
	{
		OnSSLRead();
	}

}

SSL* EdSSLSocket::getSSL()
{
	return mSSL;
}

SSL_CTX* EdSSLSocket::getSSLContext()
{
	return mSSLCtx;
}

void EdSSLSocket::OnSSLConnected()
{
	if (mSSLCallback)
		mSSLCallback->IOnSSLSocket(this, SSL_EVENT_CONNECTED);
}

void EdSSLSocket::OnSSLDisconnected()
{
	if (mSSLCallback)
		mSSLCallback->IOnSSLSocket(this, SSL_EVENT_DISCONNECTED);
}

void EdSSLSocket::OnSSLRead()
{
	if (mSSLCallback)
		mSSLCallback->IOnSSLSocket(this, SSL_EVENT_READ);
}

void EdSSLSocket::setSSLCallback(ISSLSocketCb* cb)
{
	mSSLCallback = cb;
}

int EdSSLSocket::sslSend(const void* buf, int bufsize)
{
	int wret = SSL_write(mSSL, buf, bufsize);
	if (wret <= 0)
	{
		int err = SSL_get_error(mSSL, wret);
		changeSSLSockEvent(err, true);

	}
	return wret;
}

void EdSSLSocket::sslClose()
{
	if (mSSL)
	{
		SSL_shutdown(mSSL);
		SSL_free(mSSL);
		mSSL = NULL;
	}
	close();

}

int EdSSLSocket::sslConnect(const char* ipaddr, int port)
{
	if (EdSSLIsInit() == false)
	{
		dbge("##### ssl lib is not initialized...");
		assert(0);
		return -10000;
	}
	return connect(ipaddr, port);
}

void EdSSLSocket::procSSLConnect(void)
{
	int cret = SSL_connect(mSSL);
	dbgd("ssl connect, ret=%d", cret);
	if (cret == 1)
	{
		mSessionConencted = true;
		OnSSLConnected();
	}
	else if (cret == 0)
	{
		dbgd("### session shutdown ...");
		SSL_shutdown(mSSL);
		OnSSLDisconnected();
	}
	else
	{
		int err = SSL_get_error(mSSL, cret);
		dbgd("ssl connect err, err=%d", err);
		if (err == SSL_ERROR_SSL)
		{
			long l = ERR_get_error();
			char errbuf[1024];
			ERR_error_string(l, errbuf);
			dbgd("ssl error ssl  = %s", errbuf);
			SSL_shutdown(mSSL);
		}
		else
		{
			changeSSLSockEvent(err, false);
			if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL )
			{
				postReserveDisconnect();
			}
		}
	}
}

} /* namespace edft */
