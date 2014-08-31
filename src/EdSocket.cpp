/*
 * EdSocket.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: khkim
 */
#define DBG_LEVEL DBG_WARN
#define DBGTAG "edsck"

#include "EdSocket.h"
#include "edslog.h"
#include "EdType.h"

namespace edft
{

EdSocket::EdSocket()
{
	mSockCallback = NULL;
	clearInternal();
}

EdSocket::~EdSocket()
{
	close();
}

int EdSocket::accept()
{
	struct sockaddr_in inaddr;
	socklen_t slen;
	slen = sizeof(inaddr);
	return ::accept(mFd, (struct sockaddr*) &inaddr, &slen);
}

void EdSocket::openChildSock(int fd)
{
	setDefaultContext();
	setFd(fd);
	mStatus = SOCK_STATUS_CONNECTED;
	mType = SOCK_TYPE_TCP;
	registerEvent(EVT_READ);
}

void EdSocket::acceptSock(EdSocket* pchild, ISocketCb *cb)
{
	int fd = accept();
	pchild->openChildSock(fd);
	if (cb)
		pchild->setOnListener(cb);
#if 0
	struct sockaddr_in inaddr;
	int ret;
	socklen_t slen;
	slen = sizeof(inaddr);
	int fd;
	fd = ::accept(mFd, (struct sockaddr*) &inaddr, &slen);
	if (fd > 0)
	{
		pchild->setContext(mContext);
		if (cb)
		pchild->setOnListener(cb);
		pchild->setFd(fd);
		pchild->mStatus = SOCK_STATUS_CONNECTED;
		pchild->registerEvent(EVT_READ);
		ret = 0;
	}
	else
	{
		ret = errno;
		dbge("### accept error....listen fd=%d, ret=%d", mFd, fd);
	}
#endif
}

int EdSocket::bindSock(int port, const char* addr)
{

	int retVal = -1;
	if (mFd < 0)
	{
		openSock(SOCK_TYPE_TCP);
	}


	if (mType == SOCK_TYPE_TCP || mType == SOCK_TYPE_UDP)
	{
		struct sockaddr_in inaddr;
		inaddr.sin_family = AF_INET;
		inaddr.sin_port = htons(port);
		inaddr.sin_addr.s_addr = inet_addr(addr);
		retVal = bind(mFd, (struct sockaddr*) &inaddr, sizeof(inaddr));
		if (!retVal)
		{
			mIsBinded = true;
			registerEvent(EVT_READ);
		}
	}
#if 0
	else if (mType == SOCK_TYPE_UNIXSTREAM || mType == SOCK_TYPE_UNIXDGRAM)
	{
		struct sockaddr_un uaddr;
		uaddr.sun_family = AF_UNIX;
		strcpy(uaddr.sun_path, addr);
		int len = strlen(addr) + sizeof(uaddr.sun_family);
		retVal = bind(mFd, (struct sockaddr*) &uaddr, len);
		if (!retVal)
			mIsBinded = true;
	}
#endif
	else
	{
		retVal = -1;
	}
	return retVal;
}

void EdSocket::close()
{
	if (mFd >= 0)
	{
		dbgd("close socket, fd=%d", mFd);
		::close(mFd);
		deregisterEvent();
		mFd = -1;
		mStatus = SOCK_STATUS_DISCONNECTED;
	}

	mIsBinded = false;
}

int EdSocket::connect(const char* ipaddr, int port)
{
	if (mType == SOCK_TYPE_TCP || mType == SOCK_TYPE_UDP)
	{
		uint32_t ip = inet_addr(ipaddr);
		return connect(ip, port);
	}
#if 0
	else if (mType == SOCK_TYPE_UNIXSTREAM || mType == SOCK_TYPE_UNIXDGRAM)
	{
		// TODO: connect unix socket
	}
#endif

	return -1;
}

int EdSocket::listenSock(int port, const char* ip)
{
	int retval;
	mIsListen = true;
	if (mIsBinded == false)
	{
		retval = bindSock(port, ip);
		if (retval < 0)
			return retval;
	}

	if (mType == SOCK_TYPE_TCP)
	{
		retval = listen(mFd, 10);
		if (!retval)
		{
			registerEvent(EVT_READ);
		}
	}
	else
	{
		retval = -1;
	}
	return retval;
}

int EdSocket::openSock(int type)
{
	mType = type;
	int fd;

	if (type == SOCK_TYPE_TCP)
		fd = socket(AF_INET, SOCK_STREAM, 0);
	else if (type == SOCK_TYPE_UDP)
		fd = socket(AF_INET, SOCK_DGRAM, 0);
#if 0
	else if (type == SOCK_TYPE_UNIXDGRAM)
		fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	else if (type == SOCK_TYPE_UNIXSTREAM)
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
#endif
	else
	{
		dbge("### invalid socket type=%d", type);
		assert(0);
		return -1;
	}
	setDefaultContext();
	setFd(fd);
	setNoTimewait();

	dbgd("open sock=%d, type=%d", fd, type);

	return fd;
}

int EdSocket::recv(void* buf, int size)
{
	int ret = read(mFd, buf, size);
	if (ret == 0)
	{
		int sockerr;
		socklen_t socklen = sizeof(sockerr);
		getsockopt(mFd, SOL_SOCKET, SO_ERROR, &sockerr, &socklen);
		dbgv("### read error: sock err=%d, bufsize=%d", sockerr, size);
		if (1)
		{ //sockerr == ECONNRESET) {
			postReserveDisconnect();
#if 0
			if(mType == SOCK_TYPE_TCP)
			{
				mStatus = SOCK_STATUS_DISCONNECTED;
				mRaiseDisconnect = 1; // disconnected by remote
			}
#endif
		}
	}
	/*
	 else if(ret<0)
	 {
	 int sockerr;
	 socklen_t socklen = sizeof(sockerr);
	 getsockopt(mFd, SOL_SOCKET, SO_ERROR, &sockerr, &socklen);
	 dbgv("### read ret negtive: sock err=%d, bufsize=%d", sockerr, size);

	 if(sockerr == 0) {
	 mStatus = SOCK_STATUS_DISCONNECTED;
	 mRaiseDisconnect = 1; // disconnected by remote
	 }
	 }*/
	return ret;
}

void EdSocket::rejectSock(void)
{
	int fd;
	socklen_t slen;
	struct sockaddr_in inaddr;

	fd = ::accept(mFd, (struct sockaddr*) &inaddr, &slen);
	::close(fd);
}

int EdSocket::send(const void* buf, int size)
{
	return write(mFd, buf, size);
}

void EdSocket::OnRead(void)
{
	if (mSockCallback)
		mSockCallback->IOnSocketEvent(this, SOCK_EVENT_READ);
}

void EdSocket::OnDisconnected(void)
{
	dbgv("### ondis essocket esevent=%p, cb=%p", this, mSockCallback);
	if (mSockCallback)
		mSockCallback->IOnSocketEvent(this, SOCK_EVENT_DISCONNECTED);
}

void EdSocket::OnWrite(void)
{
	if (mSockCallback)
		mSockCallback->IOnSocketEvent(this, SOCK_EVENT_WRITE);
}

void EdSocket::OnConnected(void)
{
	if (mSockCallback)
		mSockCallback->IOnSocketEvent(this, SOCK_EVENT_CONNECTED);
}

void EdSocket::OnIncomingConnection(void)
{
	if (mSockCallback)
		mSockCallback->IOnSocketEvent(this, SOCK_EVENT_INCOMING_ACCEPT);
}

void EdSocket::OnEventRead()
{
	if (mIsListen == false)
	{
		mRaiseDisconnect = 0;
		OnRead();

		if (mRaiseDisconnect != 0)
		{
			close();
			OnDisconnected();
		}

	}
	else
	{
		OnIncomingConnection();
	}
}

void EdSocket::OnEventWrite()
{
	int sockerr;
	socklen_t socklen = sizeof(sockerr);
	if (mStatus == SOCK_STATUS_CONNECTING)
	{
		getsockopt(mFd, SOL_SOCKET, SO_ERROR, &sockerr, &socklen);
		dbgv("    sock error=%d", sockerr);
		if (sockerr != 0)
		{
			if (sockerr == ECONNREFUSED)
			{
				dbgd("### connection refused.....err=%d", sockerr);
			}
			else if (sockerr == ETIMEDOUT)
			{
				dbgd("### connection timeout......err=%d", sockerr);
			}
			else if (sockerr == EHOSTUNREACH)
			{
				dbgd("### connection host_unreach...err=%d", sockerr);
			}
			else
			{
				dbgd("### connection etc error...err=%d", sockerr);
			}
			close();
			OnDisconnected();
		}
		else
		{
			dbgd("=== connected, fd=%d, ", mFd);
			mStatus = SOCK_STATUS_CONNECTED;
			changeEvent(EVT_READ | EVT_HANGUP);
			OnConnected();
		}
	}
	else
	{
		OnWrite();
	}
}

void EdSocket::OnEventHangup(void)
{
	dbgd("#### on event hangup");
	int sockerr;
	socklen_t socklen = sizeof(sockerr);
	getsockopt(mFd, SOL_SOCKET, SO_ERROR, &sockerr, &socklen);
	dbgv("    sock error=%d", sockerr);
	if (sockerr != 0)
	{
		if (sockerr == ECONNREFUSED)
		{
			dbgd("### connection refused.....err=%d", sockerr);
		}
		else if (sockerr == ETIMEDOUT)
		{
			dbgd("### connection timeout......err=%d", sockerr);
		}
		else if (sockerr == EHOSTUNREACH)
		{
			dbgd("### connection host_unreach...err=%d", sockerr);
		}
		else
		{
			dbgd("### connection etc error...err=%d", sockerr);
		}
	}

	close();
	OnDisconnected();
}

int EdSocket::connect(uint32_t ip, int port)
{
	if (mFd < 0)
		openSock(SOCK_TYPE_TCP);

	struct sockaddr_in sckaddr;

	sckaddr.sin_family = AF_INET;
	sckaddr.sin_port = htons(port);
	sckaddr.sin_addr.s_addr = ip;

	int cnnret = ::connect(mFd, (struct sockaddr*) &sckaddr, sizeof(sckaddr));
	dbgd("	connecting to %0x ......., ret=%d, errno=%d", ip, cnnret, errno);
	if (mType == SOCK_TYPE_TCP)
	{
		registerEvent(EVT_WRITE | EVT_HANGUP);
		mStatus = SOCK_STATUS_CONNECTING;

	}
	else if (mType == SOCK_TYPE_UDP)
	{

	}
	else
	{
		cnnret = -1;
	}
	return cnnret;
}

void EdSocket::setNoTimewait()
{

	struct linger lingerdata =
	{ 1, 0 };
	setsockopt(mFd, SOL_SOCKET, SO_LINGER, &lingerdata, sizeof(lingerdata));

}

void EdSocket::getPeerAddr(char* ipaddr, u16* port)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	getpeername(mFd, (sockaddr*) &addr, &len);
	strcpy(ipaddr, inet_ntoa(addr.sin_addr));
	*port = ntohs(addr.sin_port);
}

void EdSocket::setOnListener(ISocketCb* cb)
{
	mSockCallback = cb;
	dbgv("setcb, cb=%p, msockcallback=%p", cb, mSockCallback);
}

EdSocket::ISocketCb* EdSocket::getCallback()
{
	return mSockCallback;
}

void EdSocket::clearInternal()
{
	mStatus = SOCK_STATUS_DISCONNECTED;
	mFamily = AF_INET;
	mIsListen = false;
	mRaiseDisconnect = 0;
	mType = SOCK_TYPE_TCP;
	mIsBinded = false;
}

void EdSocket::postReserveDisconnect()
{
	if (mType == SOCK_TYPE_TCP)
	{
		mStatus = SOCK_STATUS_DISCONNECTED;
		mRaiseDisconnect = 1; // disconnected by remote
	}
}

} /* namespace edft */
