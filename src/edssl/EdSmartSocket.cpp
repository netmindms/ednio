/*
 * EdSmartSocket.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "sslsc"
#include "../edslog.h"
#include "../EdNio.h"
#include "EdSmartSocket.h"

#include <openssl/err.h>

namespace edft
{

EdSmartSocket::EdSmartSocket()
{
	mSSLCtx = NULL;
	mSSL = NULL;
	mSessionConencted = false;
	//mSSLCallback = NULL;
	mIsSSLServer = false;
	mOnLis = NULL;
	mIsSSL = false;
}

EdSmartSocket::~EdSmartSocket()
{
	socketClose();
}

int EdSmartSocket::socketOpen(bool ssl)
{
	int ret;
	mIsSSL = ssl;
	if (mIsSSL == false)
	{
		ret = openSock(SOCK_TYPE_TCP);
	}
	else
	{
		ret = openSSLClientSock();
	}
	return ret;
}

void EdSmartSocket::OnRead()
{
	if (mIsSSL == false)
	{
		if (mOnLis != NULL)
		{
			mOnLis->IOnNet(this, NETEV_READ);
		}
	}
	else
	{
		procSSLRead();
	}
}

void EdSmartSocket::OnWrite()
{
}

void EdSmartSocket::OnConnected()
{
	dbgd("socket connected...");
	if (mIsSSL == true)
	{
		startHandshake();
	}
	else
	{
		if (mOnLis != NULL)
		{
			mOnLis->IOnNet(this, NETEV_CONNECTED);
		}
	}
}

void EdSmartSocket::OnDisconnected()
{
	dbgd("socket disconnected...");
	bool sscnn = mSessionConencted;
	socketClose();
#if 0
	if (mSSLCallback)
	mSSLCallback->IOnSSLSocket(this, SSL_EVENT_DISCONNECTED);
#endif
	if (mOnLis != NULL)
	{
		if (mIsSSL == false)
		{
			mOnLis->IOnNet(this, NETEV_DISCONNECTED);
		}
		else if (sscnn == true)
		{
			mOnLis->IOnNet(this, NETEV_DISCONNECTED);
		}
	}
}

void EdSmartSocket::startHandshake()
{
	dbgd("start handshake...");

	if (mSSL == NULL && mSessionConencted == false)
	{
		mSSL = SSL_new(mSSLCtx);
		SSL_set_fd(mSSL, getFd());
		procSSLConnect();
	}
}

void EdSmartSocket::setOnNetListener(INet* lis)
{
	mOnLis = lis;
}

void EdSmartSocket::changeSSLSockEvent(int err, bool bwrite)
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

int EdSmartSocket::recvPacket(void* buf, int bufsize)
{
	int rret;
	if (mIsSSL == false)
	{
		rret = recv(buf, bufsize);
	}
	else
	{
		rret = SSL_read(mSSL, buf, bufsize);
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
}

void EdSmartSocket::procSSLRead(void)
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

SSL* EdSmartSocket::getSSL()
{
	return mSSL;
}

SSL_CTX* EdSmartSocket::getSSLContext()
{
	return mSSLCtx;
}

void EdSmartSocket::OnSSLConnected()
{
	if (mOnLis != NULL)
		mOnLis->IOnNet(this, NETEV_CONNECTED);
}

void EdSmartSocket::OnSSLDisconnected()
{
	if (mOnLis != NULL)
		mOnLis->IOnNet(this, NETEV_CONNECTED);
}

void EdSmartSocket::OnSSLRead()
{
	if (mOnLis != NULL)
		mOnLis->IOnNet(this, NETEV_READ);
}

int EdSmartSocket::sendPacket(const void* buf, int bufsize)
{
	int wret;
	if (mIsSSL == false)
	{
		wret = send(buf, bufsize);
	}
	else
	{
		wret = SSL_write(mSSL, buf, bufsize);
		if (wret <= 0)
		{
			int err = SSL_get_error(mSSL, wret);
			changeSSLSockEvent(err, true);

		}
	}
	return wret;
}

void EdSmartSocket::socketClose()
{
	if (mSSL)
	{
		dbgd("ssl close, free ssl=%p, state=%d", mSSL, SSL_state(mSSL));
		SSL_shutdown(mSSL);
		SSL_free(mSSL);
		mSSL = NULL;
		mSSLCtx = NULL;
		mSessionConencted = false;
	}
	EdSocket::close();

}

void EdSmartSocket::procSSLConnect(void)
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

void EdSmartSocket::openSSLChildSock(int fd, SSL_CTX* pctx)
{
	openChildSock(fd);
	mIsSSLServer = true;
	if (pctx == NULL)
	{
		mSSLCtx = getCurrentTask()->getSSLContext();
	}
	else
	{
		mSSLCtx = pctx;
	}
	mSSL = SSL_new(mSSLCtx);
	SSL_set_fd(mSSL, fd);

}

int EdSmartSocket::openSSLClientSock(SSL_CTX* pctx)
{
	int fd = openSock(SOCK_TYPE_TCP);
	if (fd < 0)
		return fd;
	if (pctx == NULL)
	{
		mSSLCtx = getCurrentTask()->getSSLContext();
	}
	else
	{
		mSSLCtx = pctx;
	}

	mIsSSLServer = false;
	return fd;
}

void EdSmartSocket::sslAccept()
{
	procSSLConnect();
}

} /* namespace edft */
