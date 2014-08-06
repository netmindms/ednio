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
	mIsSSLServer = false;
}

EdSSLSocket::~EdSSLSocket()
{
	close();
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
	close();

	if (mSSLCallback)
		mSSLCallback->IOnSSLSocket(this, SSL_EVENT_DISCONNECTED);
}

void EdSSLSocket::startHandshake()
{
	dbgd("start handshake...");

	if (mSSL == NULL && mSessionConencted == false)
	{
		mSSL = SSL_new(mSSLCtx);
		SSL_set_fd(mSSL, getFd());
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

int EdSSLSocket::recv(void* buf, int bufsize)
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

int EdSSLSocket::send(const void* buf, int bufsize)
{
	int wret = SSL_write(mSSL, buf, bufsize);
	if (wret <= 0)
	{
		int err = SSL_get_error(mSSL, wret);
		changeSSLSockEvent(err, true);

	}
	return wret;
}

void EdSSLSocket::close()
{
	if (mSSL)
	{
		dbgd("ssl close, free ssl=%p, state=%d", mSSL,SSL_state(mSSL));
		SSL_shutdown(mSSL);
		SSL_free(mSSL);
		mSSL = NULL;
		mSSLCtx = NULL;
		mSessionConencted = false;
	}
	EdSocket::close();

}

int EdSSLSocket::connect(const char* ipaddr, int port)
{
	uint32_t ip = inet_addr(ipaddr);
	return connect(ip, port);
}

int EdSSLSocket::connect(uint32_t ip, int port)
{
	if (EdSSLIsInit() == false)
	{
		dbge("##### ssl lib is not initialized...");
		assert(0);
		return -10000;
	}
	return EdSocket::connect(ip, port);
}

void EdSSLSocket::procSSLConnect(void)
{
	int cret;
	if (mIsSSLServer == false)
		cret = SSL_connect(mSSL);
	else
		cret = SSL_accept(mSSL);

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
			if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL)
			{
				postReserveDisconnect();
			}
		}
	}
}

void EdSSLSocket::openSSLChildSock(int fd, SSL_CTX* psslctx)
{
	openChildSock(fd);
	mIsSSLServer = true;
	mSSLCtx = psslctx;
	mSSL = SSL_new(mSSLCtx);
	SSL_set_fd(mSSL, fd);

}

int EdSSLSocket::openSSLClientSock(SSL_CTX* pctx)
{
	int fd = openSock(SOCK_TYPE_TCP);
	if (fd < 0)
		return fd;
	mSSLCtx = pctx;
	mIsSSLServer = false;
	return fd;
}

void EdSSLSocket::sslAccept()
{
	procSSLConnect();
}

} /* namespace edft */
