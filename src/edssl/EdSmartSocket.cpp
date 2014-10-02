/*
 * EdSmartSocket.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "smsck"
#include "../edslog.h"
#include "../EdNio.h"
#include "EdSmartSocket.h"

#include <openssl/err.h>

namespace edft
{

EdSmartSocket::EdSmartSocket()
{
	mEdSSLCtx = NULL;
	mSSLCtx = NULL;
	mSSL = NULL;
	mSessionConencted = false;
	//mSSLCallback = NULL;
	mIsSSLServer = false;
	mOnLis = NULL;
	mMode = 0;
	mPendingBuf = NULL;
	mPendingSize = 0;
	mPendingWriteCnt = 0;
	mSSLWantEvent = 0;
}

EdSmartSocket::~EdSmartSocket()
{
	socketClose();
}

int EdSmartSocket::socketOpen(int mode)
{
	int ret;
	mMode = mode;
	if (mMode == SOCKET_NORMAL)
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
	//dbgd("OnRead...");
	if (mMode == SOCKET_NORMAL)
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
	dbgd("OnWrite");
	if (mMode == SOCKET_NORMAL)
	{
		procNormalOnWrite();
	}
	else
	{
		procSSLOnWrite();
	}
}

void EdSmartSocket::OnConnected()
{
	dbgd("socket connected...");
	if (mMode == SOCKET_SSL)
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

	if (mOnLis != NULL)
	{
		if (mMode == false)
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
		mSSLWantEvent = EVT_WRITE;
		changeEvent(EVT_WRITE);
		dbgd("change write event.......");
	}
	else if (err == SSL_ERROR_WANT_READ)
	{
		mSSLWantEvent = EVT_READ;
		changeEvent(EVT_READ);
		dbgd("change read event.......");
	}
	else if (err == SSL_ERROR_ZERO_RETURN)
	{
		mSSLWantEvent = 0;
		dbgd("zero return...");
	}
}

int EdSmartSocket::recvPacket(void* buf, int bufsize)
{
	int rret;
	if (mMode == false)
	{
		rret = recv(buf, bufsize);
		return rret;
	}
	else
	{
		mSSLWantEvent = 0;
		rret = SSL_read(mSSL, buf, bufsize);
		if (rret < 0)
		{
			int err = SSL_get_error(mSSL, rret);
			dbgd("sslRecv,rret=%d, err=%d", rret, err);
			changeSSLSockEvent(err, false);
			if (err == SSL_ERROR_ZERO_RETURN)
			{
				//SSL_shutdown(mSSL);
				//postReserveDisconnect();
				mTask->lastSockErrorNo = EINPROGRESS;
			}
			return -1;
		}
		else if (rret == 0)
		{
			int err = SSL_get_error(mSSL, rret);
			dbgd(" rret is zero, err=%d", err);

//			SSL_shutdown(mSSL);
//			postReserveDisconnect();
			mTask->lastSockErrorNo = EINPROGRESS;
			return 0;
		}
		else
		{
			mTask->lastSockErrorNo = 0;
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
		mOnLis->IOnNet(this, NETEV_DISCONNECTED);
}

void EdSmartSocket::OnSSLRead()
{
	if (mOnLis != NULL)
		mOnLis->IOnNet(this, NETEV_READ);
}

int EdSmartSocket::sendPacket(const void* buf, int bufsize, bool takebuffer)
{
	int wret;
	if (mPendingBuf != NULL)
	{
		dbgd("*** send packet fail, pending buf not null");
		return SEND_FAIL;
	}

	if (mMode == SOCKET_NORMAL)
	{
		wret = send(buf, bufsize);
		bool ispending;

		if (wret == bufsize)
		{
			if (takebuffer == true)
			{
				free((void*) buf);
			}
			return SEND_OK;
		}
		else if (wret > 0)
		{
			dbgd("partial write, input size=%ld, write size=%ld", bufsize, wret);
			ispending = true;
		}
		else
		{
			dbgd("*** send fail, error=%d", errno);
			if (errno == EAGAIN)
			{
				ispending = true;
			}
			else
			{
				ispending = false;
			}
			wret = 0;
		}

		if (ispending == true)
		{
			if (takebuffer == false)
			{
				mPendingSize = bufsize - wret;
				mPendingWriteCnt = 0;
				mPendingBuf = malloc(mPendingSize);
				if (mPendingBuf == NULL)
				{
					dbge("### Fail: memory allocation error for peinding buffer");
					return SEND_FAIL;
				}
				memcpy(mPendingBuf, (u8*) buf + wret, mPendingSize);
			}
			else
			{
				if (wret < 0)
					wret = 0;
				mPendingSize = bufsize;
				mPendingBuf = (void*) buf;
				mPendingWriteCnt = bufsize - wret;
			}
			changeEvent(EVT_READ | EVT_WRITE | EVT_HANGUP);

			return SEND_PENDING;
		}
		else
		{
			return SEND_FAIL;
		}
	}
	else
	{
		if (takebuffer == false)
		{
			mPendingBuf = malloc(bufsize);
			assert(mPendingBuf != NULL);
			memcpy(mPendingBuf, buf, bufsize);
			mPendingWriteCnt = 0;
			mPendingSize = bufsize;
		}
		else
		{
			mPendingBuf = (void*) buf;
			mPendingSize = bufsize;
			mPendingWriteCnt = 0;
		}
		mSSLWantEvent = 0;
		wret = SSL_write(mSSL, mPendingBuf, mPendingSize);
		if (wret == bufsize)
		{
			//dbgd("ssl write cnt=%d, input cnt=%d", wret, bufsize);
			free(mPendingBuf);
			mPendingBuf = NULL;
			mPendingWriteCnt = 0;
			mPendingSize = 0;
			return SEND_OK;
		}
		else
		{
			dbgd("** ssl write ret=%d", wret);
			if (wret <= 0)
			{
				dbgd(" ssl write error, input size=%d, wret=%d", bufsize, wret);
				int err = SSL_get_error(mSSL, wret);
				changeSSLSockEvent(err, true);
			}

			return SEND_PENDING;
		}
	}
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

	if (mPendingBuf != NULL)
	{
		free(mPendingBuf);
		mPendingBuf = NULL;
	}

}

void EdSmartSocket::procSSLErrCloseNeedEnd()
{
	mTask->lastSockErrorNo = EINPROGRESS;
	OnSSLDisconnected();
	if (EdTask::getCurrentTask()->lastSockErrorNo != 0)
	{
		dbgd("*** auto closing ssl socket... ");
		socketClose();
	}
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
		mTask->lastSockErrorNo = 0;
		OnSSLConnected();
	}
	else if (cret == 0)
	{
		dbgd("### session shutdown ...");
		procSSLErrCloseNeedEnd();
//		mTask->lastSockErrorNo = EINPROGRESS;
//		OnSSLDisconnected();
//		if (EdTask::getCurrentTask()->lastSockErrorNo == EINPROGRESS)
//		{
//			dbgd("*** auto closing ssl socket... ");
//			socketClose();
//		}
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
			dbgd("ssl error string  = %s", errbuf);
			procSSLErrCloseNeedEnd();
//			mTask->lastSockErrorNo = EINPROGRESS;
//			OnSSLDisconnected();
//			if (EdTask::getCurrentTask()->lastSockErrorNo == EINPROGRESS)
//			{
//				dbgd("*** auto closing ssl socket... ");
//				socketClose();
//			}
		}
		else
		{
			changeSSLSockEvent(err, false);
			if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL)
			{
				//postReserveDisconnect();
				procSSLErrCloseNeedEnd();
//				mTask->lastSockErrorNo = EINPROGRESS;
//				OnSSLDisconnected();
//				if (EdTask::getCurrentTask()->lastSockErrorNo == EINPROGRESS)
//				{
//					dbgd("*** auto closing ssl socket... ");
//					socketClose();
//				}
			}
			else
			{
				dbgd("##### unknown ssl state, sslerr=%d", err);
				mTask->lastSockErrorNo = 0;
			}
		}
	}
}

void EdSmartSocket::openSSLChildSock(int fd, EdSSLContext* pctx)
{
	openChildSock(fd);
	mIsSSLServer = true;
	if (pctx == NULL)
	{
		//mSSLCtx = EdTask::getCurrentTask()->getSSLContext();
		mEdSSLCtx = EdSSLContext::getDefaultEdSSL();
		mSSLCtx = mEdSSLCtx->getContext();
	}
	else
	{
		mEdSSLCtx = pctx;
		mSSLCtx = mEdSSLCtx->getContext();
	}
	mSSL = SSL_new(mSSLCtx);
	SSL_set_fd(mSSL, fd);

}

int EdSmartSocket::openSSLClientSock(EdSSLContext* pctx)
{
	int fd = openSock(SOCK_TYPE_TCP);
	if (fd < 0)
		return fd;
	if (pctx == NULL)
	{
		//mSSLCtx = EdTask::getCurrentTask()->getSSLContext();
		mEdSSLCtx = EdSSLContext::getDefaultEdSSL();
		mSSLCtx = mEdSSLCtx->getContext();
	}
	else
	{
		mEdSSLCtx = pctx;
		mSSLCtx = mEdSSLCtx->getContext();
	}

	mIsSSLServer = false;
	return fd;
}

void EdSmartSocket::sslAccept()
{
	procSSLConnect();
}

int EdSmartSocket::socketOpenChild(int fd, int mode)
{
	mMode = mode;
	if (mode == SOCKET_NORMAL)
	{
		openChildSock(fd);
	}
	else
	{
		openSSLChildSock(fd, NULL);
	}
	return 0;
}

bool EdSmartSocket::isWritable()
{
	if (mPendingBuf == NULL)
		return true;
	else
		return false;
}

void EdSmartSocket::reserveWrite()
{
	dbgd("reserve write...");
	changeEvent(EVT_WRITE | EVT_HANGUP | EVT_READ);
}

void EdSmartSocket::procNormalOnWrite()
{
	if (mPendingBuf != NULL)
	{
		int wret;
		wret = send((u8*) mPendingBuf + mPendingWriteCnt, mPendingSize - mPendingWriteCnt);

		if (wret > 0)
		{
			mPendingWriteCnt += wret;
			if (mPendingWriteCnt == mPendingSize)
			{
				changeEvent(EVT_READ | EVT_HANGUP);
				mPendingSize = 0;
				mPendingWriteCnt = 0;
				free(mPendingBuf);
				mPendingBuf = NULL;
				if (mOnLis != NULL)
				{
					mOnLis->IOnNet(this, NETEV_SENDCOMPLETE);
				}
			}
		}
	}
	else
	{
		dbgd("no pending buffer on write...");
		changeEvent(EVT_READ | EVT_HANGUP);
		if (mOnLis != NULL)
		{
			mOnLis->IOnNet(this, NETEV_SENDCOMPLETE);
		}
	}
}

void EdSmartSocket::procSSLOnWrite()
{
	if (mSessionConencted == false)
	{
		procSSLConnect();
		return;
	}

	if (mPendingBuf != NULL)
	{
		int wret;
		mSSLWantEvent = 0;
		wret = SSL_write(mSSL, (u8*) mPendingBuf + mPendingWriteCnt, mPendingSize - mPendingWriteCnt);
		if (wret > 0)
		{
			mPendingWriteCnt += wret;
			if (mPendingWriteCnt == mPendingSize)
			{
				changeEvent(EVT_READ | EVT_HANGUP);
				mPendingSize = 0;
				mPendingWriteCnt = 0;
				free(mPendingBuf);
				mPendingBuf = NULL;
				if (mOnLis != NULL)
				{
					mOnLis->IOnNet(this, NETEV_SENDCOMPLETE);
				}
			}
		}
		else if (wret <= 0)
		{
			int err = SSL_get_error(mSSL, wret);
			changeSSLSockEvent(err, true);
		}

	}
	else
	{
		dbgd("no pending buffer on write...");
		changeEvent(EVT_READ | EVT_HANGUP);
		if (mOnLis != NULL)
		{
			mOnLis->IOnNet(this, NETEV_SENDCOMPLETE);
		}
	}
}

} /* namespace edft */
