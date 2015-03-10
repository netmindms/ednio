/*
 * EdSmartSocket.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_WARN
#define DBGTAG "SMSCK"

#include "ednio_config.h"

#include <iostream>
#include "edslog.h"
#include "EdNio.h"
#include "EdSmartSocket.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

using namespace std;

namespace edft
{

EdSmartSocket::EdSmartSocket() : mSock(this)
{
	mMode = 0;
	mPendingBuf = NULL;
	mPendingSize = 0;
	mPendingWriteCnt = 0;
#if USE_SSL
	mEdSSLCtx = NULL;
	mSSLCtx = NULL;
	mSSL = NULL;
	mSessionConencted = false;
	mIsServer = false;
	mSSLWantEvent = 0;
#endif
}

EdSmartSocket::~EdSmartSocket()
{
	close();
}

int EdSmartSocket::openClient(int mode)
{
	int ret;
	mMode = mode;
	if (mMode == SOCKET_NORMAL)
	{
		ret = mSock.openSock(SOCK_TYPE_TCP);
	}
	else
	{
#if USE_SSL
		ret = openSSLClientSock();
#else
		assert(0);
#endif
	}
	return ret;
}

void EdSmartSocket::fireRead()
{
	dbgv("OnRead...");
	if (mMode == SOCKET_NORMAL)
	{
		if(mOnLis != NULL)
		{
			mOnLis(*this, NETEV_READABLE);
		}
	}
	else
	{
#if USE_SSL
		procSSLRead();
#endif
	}
}

void EdSmartSocket::fireWrite()
{
	dbgd("OnWrite");
	if (mMode == SOCKET_NORMAL)
	{
		procNormalOnWrite();
	}
	else
	{
#if USE_SSL
		procSSLOnWrite();
#endif
	}
}

void EdSmartSocket::fireConnected()
{
	dbgd("socket connected...");
	if (mMode == SOCKET_NORMAL)
	{
		if(mOnLis != nullptr)
		{
			mOnLis(*this, NETEV_CONNECTED);
		}
	}
	else
	{
#if USE_SSL
		startHandshake();
#endif
	}
}

void EdSmartSocket::fireDisconnected()
{
	dbgd("socket disconnected..., fd=%d", mSock.getFd());
#if USE_SSL
	bool sscnn = mSessionConencted;
#endif
	// TODO: socketClose();

	if (mOnLis != NULL)
	{
		if (mMode == SOCKET_NORMAL)
		{
			mOnLis(*this, NETEV_DISCONNECTED);
		}

#if USE_SSL
		else if (sscnn == true)
		{
			mOnLis(*this, NETEV_DISCONNECTED);
		}
#endif

	}
}



void EdSmartSocket::setOnListener(SmartSocketLis lis)
{
	mOnLis = lis;
}


int EdSmartSocket::recvPacket(void* buf, int bufsize)
{
	int rret;
	if (mMode == SOCKET_NORMAL)
	{
		rret = mSock.recv(buf, bufsize);
		return rret;
	}
	else
	{
#if USE_SSL
		mSSLWantEvent = 0;
		rret = SSL_read(mSSL, buf, bufsize);
		dbgd("  ssl read cnt=%d", rret);
		if (rret < 0)
		{
			int err = SSL_get_error(mSSL, rret);
			dbgd("sslRecv,rret=%d, err=%d", rret, err);
			changeSSLSockEvent(err, false);
			if (err == SSL_ERROR_ZERO_RETURN)
			{
				//SSL_shutdown(mSSL);
				//postReserveDisconnect();
				mSock.getTask()->lastSockErrorNo = EINPROGRESS;
			}
			return -1;
		}
		else if (rret == 0)
		{
			int err = SSL_get_error(mSSL, rret);
			dbgd(" rret is zero, err=%d", err);

//			SSL_shutdown(mSSL);
//			postReserveDisconnect();
			mSock.getTask()->lastSockErrorNo = EINPROGRESS;
			return 0;
		}
		else
		{
			mSock.getTask()->lastSockErrorNo = 0;
			return rret;
		}
#else
		return -1;
#endif
	}
}


void EdSmartSocket::OnSSLConnected()
{
	if (mOnLis != NULL)
		mOnLis(*this, NETEV_CONNECTED);
}

void EdSmartSocket::OnSSLDisconnected()
{
	if (mOnLis != NULL)
		mOnLis(*this, NETEV_DISCONNECTED);
}

void EdSmartSocket::OnSSLRead()
{
	if (mOnLis != NULL)
		mOnLis(*this, NETEV_READABLE);
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
		wret = mSock.send(buf, bufsize);
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
			mSock.changeEvent(EVT_READ | EVT_WRITE | EVT_HANGUP);

			return SEND_PENDING;
		}
		else
		{
			return SEND_FAIL;
		}
	}
	else
	{
#if USE_SSL
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
#else
		assert(0);
#endif
	}
}

int EdSmartSocket::connect(const string& addr, int port)
{
	return mSock.connect(addr.c_str(), port);
}


void EdSmartSocket::close()
{
#if USE_SSL
	if (mSSL != NULL)
	{
		dbgd("ssl close, free ssl=%p, state=%d", mSSL, SSL_state(mSSL));
		SSL_shutdown (mSSL);
		SSL_free(mSSL);
		mSSL = NULL;
		mSSLCtx = NULL;
		mSessionConencted = false;
	}
#endif
	mSock.close();

	if (mPendingBuf != NULL)
	{
		free(mPendingBuf);
		mPendingBuf = NULL;
	}

}



int EdSmartSocket::openChild(int fd, int mode)
{
	mMode = mode;
	if (mode == SOCKET_NORMAL)
	{
		mSock.openChildSock(fd);
	}
	else
	{
#if USE_SSL
		openSSLChildSock(fd, NULL);
#endif
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
	mSock.changeEvent(EVT_WRITE | EVT_HANGUP | EVT_READ);
}

void EdSmartSocket::procNormalOnWrite()
{
	if (mPendingBuf != NULL)
	{
		int wret;
		wret = mSock.send((u8*) mPendingBuf + mPendingWriteCnt, mPendingSize - mPendingWriteCnt);

		if (wret > 0)
		{
			mPendingWriteCnt += wret;
			if (mPendingWriteCnt == mPendingSize)
			{
				mSock.changeEvent(EVT_READ | EVT_HANGUP);
				mPendingSize = 0;
				mPendingWriteCnt = 0;
				free(mPendingBuf);
				mPendingBuf = NULL;
				if (mOnLis != NULL)
				{
					mOnLis(*this, NETEV_WRITABLE);
				}
			}
		}
	}
	else
	{
		dbgd("no pending buffer on write...");
		mSock.changeEvent(EVT_READ | EVT_HANGUP);
		if (mOnLis != NULL)
		{
			mOnLis(*this, NETEV_WRITABLE);
		}
	}
}


#if USE_SSL
void EdSmartSocket::startHandshake()
{
	dbgd("start handshake...");

	if (mSSL == NULL && mSessionConencted == false)
	{
		mSSL = SSL_new(mSSLCtx);
		SSL_set_fd(mSSL, mSock.getFd());
		procSSLConnect();
	}
}

void EdSmartSocket::changeSSLSockEvent(int err, bool bwrite)
{
	if (err == SSL_ERROR_WANT_WRITE)
	{
		mSSLWantEvent = EVT_WRITE;
		mSock.changeEvent(EVT_WRITE|EVT_HANGUP);
		dbgd("ssl want write event.......");
	}
	else if (err == SSL_ERROR_WANT_READ)
	{
		mSSLWantEvent = EVT_READ;
		mSock.changeEvent(EVT_READ|EVT_HANGUP);
		dbgd("ssl want read event.......");
	}
	else if (err == SSL_ERROR_ZERO_RETURN)
	{
		dbgd("ssl error zero return...");
		mSSLWantEvent = 0;
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

void EdSmartSocket::procSSLErrCloseNeedEnd()
{
	mSock.getTask()->lastSockErrorNo = EINPROGRESS;
	OnSSLDisconnected();
	if (EdTask::getCurrentTask()->lastSockErrorNo != 0)
	{
		dbgd("*** auto closing ssl socket... ");
		close();
	}
}

void EdSmartSocket::procSSLConnect(void)
{
	int cret;
	if (mIsServer == false)
		cret = SSL_connect(mSSL);
	else
		cret = SSL_accept(mSSL);

	dbgd("ssl connect, ret=%d", cret);
	if (cret == 1)
	{
		mSessionConencted = true;
		mSock.getTask()->lastSockErrorNo = 0;
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
				mSock.getTask()->lastSockErrorNo = 0;
			}
		}
	}
}

void EdSmartSocket::sslAccept()
{
	procSSLConnect();
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
				mSock.changeEvent(EVT_READ | EVT_HANGUP);
				mPendingSize = 0;
				mPendingWriteCnt = 0;
				free(mPendingBuf);
				mPendingBuf = NULL;
				if (mOnLis != NULL)
				{
					mOnLis(*this, NETEV_WRITABLE);
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
		if (mSSLWantEvent != EVT_WRITE)
			mSock.changeEvent(EVT_READ | EVT_HANGUP);
		if (mOnLis != NULL)
		{
			mOnLis(*this, NETEV_WRITABLE);
		}
	}
}

#if 0 // TODO: alpn support
int EdSmartSocket::sm_ssl_alpn_cb(SSL *ssl,
					   const unsigned char **out,
					   unsigned char *outlen,
					   const unsigned char *in,
					   unsigned int inlen,
					   void *arg)
{

	vector<string> client_protos;
	string ps;
	for(u8 i=0;i<inlen;) {
		u8 len = in[i++];
		ps.assign((char*)in+i, len);
		dbgd("client alpn proto: %s", ps.c_str());
		client_protos.push_back(move(ps));
		i += len;
	}
	auto smsck = (EdSmartSocket*)arg;
	int idx=-1;
	smsck->OnAlpnSelect(&idx, client_protos);
	if(idx>=0)
	{
		smsck->mAlpnSelectProto = move(client_protos[idx]);
		dbgd("determined alpn protocol=%s", smsck->mAlpnSelectProto.c_str());
		*out = (u8*)smsck->mAlpnSelectProto.c_str();
		*outlen = smsck->mAlpnSelectProto.size();
		return SSL_TLSEXT_ERR_OK;
	}
	else
	{
		return SSL_TLSEXT_ERR_NOACK;
	}
	return SSL_TLSEXT_ERR_NOACK;
}
#endif

void EdSmartSocket::openSSLChildSock(int fd, EdSSLContext* pctx)
{
	mSock.openChildSock(fd);
	mIsServer = true;
	if (pctx == NULL)
	{
		//mSSLCtx = EdTask::getCurrentTask()->getSSLContext();
		mEdSSLCtx = EdSSLContext::getDefaultEdSSL();
		mSSLCtx = mEdSSLCtx->getContext();
		// TODO: SSL_CTX_set_alpn_select_cb(mSSLCtx, sm_ssl_alpn_cb, (void*)this);
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
	int fd = mSock.openSock(SOCK_TYPE_TCP);
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

	mIsServer = false;
	return fd;
}
#endif


#if 0 // TODO: alpn support
void EdSmartSocket::OnAlpnSelect(int* selecton_idx, const vector<string>& protos)
{
	*selecton_idx = 1;
}


string EdSmartSocket::getSelectedAlpnProto()
{
	const unsigned char *client_proto;
	unsigned int client_proto_len=0;
	SSL_get0_alpn_selected(mSSL, &client_proto, &client_proto_len);
	if(client_proto_len>0)
	{
		string ret;
		ret.assign((char*)client_proto, client_proto_len);
		dbgd(" selected alpn protocol: %s", ret.c_str());
		return ret;
	}
	else
		return "";
}
#endif


void EdSmartSocket::RawSocket::OnRead()
{
	mSmartSock->fireRead();
}

void EdSmartSocket::RawSocket::OnWrite()
{
	mSmartSock->fireWrite();
}

void EdSmartSocket::RawSocket::OnConnected()
{
	mSmartSock->fireConnected();
}

void EdSmartSocket::RawSocket::OnDisconnected()
{
	mSmartSock->fireDisconnected();
}



}
/* namespace edft */
