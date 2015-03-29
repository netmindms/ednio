/*
 * EdSocket.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: khkim
 */
#define DBGTAG "EDSCK"
#define DBG_LEVEL DBG_WARN

#include <string>
#include "EdTask.h"
#include "EdSocket.h"
#include "edslog.h"
#include "EdType.h"

namespace edft
{

EdSocket::EdSocket()
{
	mOnLis = nullptr;
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
	registerEvent(EVT_READ | EVT_HANGUP);
}

int EdSocket::acceptSock(EdSocket* pchild, SocketListener lis)
{
	int fd = accept();
	if (fd < 0)
		return fd;
	pchild->openChildSock(fd);
	if (lis != nullptr)
		pchild->setOnListener(lis);
	return fd;
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
	else if (mType == SOCK_TYPE_UNIXSTREAM || mType == SOCK_TYPE_UNIXDGRAM)
	{
		struct sockaddr_un uaddr;
		memset(&uaddr, 0, sizeof(uaddr));
		uaddr.sun_family = AF_UNIX;
		strcpy(uaddr.sun_path, addr);
		int len = strlen(addr) + sizeof(uaddr.sun_family);
		retVal = bind(mFd, (struct sockaddr*) &uaddr, len);
		if (!retVal)
		{
			mIsBinded = true;
			registerEvent(EVT_READ);
		}
	}
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
		if (mTask != NULL)
		{
			mTask->lastSockErrorNo = 0;
		}
		deregisterEvent();
		mFd = -1;
		mStatus = SOCK_STATUS_DISCONNECTED;
	}

	mIsBinded = false;
}

int EdSocket::connect(uint32_t ip, int port)
{
	if (mStatus != SOCK_STATUS_DISCONNECTED)
	{
		return EALREADY;
	}

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

int EdSocket::connect(const char* addr, int port)
{
	if (mType == SOCK_TYPE_TCP || mType == SOCK_TYPE_UDP)
	{
		uint32_t ip = inet_addr(addr);
		return connect(ip, port);
	}
	else if (mType == SOCK_TYPE_UNIXSTREAM || mType == SOCK_TYPE_UNIXDGRAM)
	{
		if (mStatus != SOCK_STATUS_DISCONNECTED)
		{
			return EALREADY;
		}
		struct sockaddr_un sckaddr;
		int len;
		sckaddr.sun_family = AF_UNIX;
		strcpy(sckaddr.sun_path, addr);
		len = sizeof(sckaddr.sun_family) + strlen(addr);
		int cnnret = ::connect(getFd(), (struct sockaddr*) &sckaddr, len);
		dbgd("unix connect, ret=%d, err=%d", cnnret, errno);
		if (cnnret != 0)
		{
			if (errno == EINPROGRESS)
			{
				mStatus = SOCK_STATUS_CONNECTING;
				registerEvent(EVT_READ | EVT_HANGUP);
			}
			else
			{
				mStatus = SOCK_STATUS_DISCONNECTED;
				// TODO: have to determine return value in this case.
			}
		}
		else
		{
			mStatus = SOCK_STATUS_CONNECTED;
			registerEvent(EVT_READ | EVT_HANGUP);
		}

		return cnnret;
	}

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

	if (mType == SOCK_TYPE_TCP || mType == SOCK_TYPE_UNIXSTREAM)
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
	else if (type == SOCK_TYPE_UNIXDGRAM)
		fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	else if (type == SOCK_TYPE_UNIXSTREAM)
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
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
		mTask->lastSockErrorNo = EBADFD;
		int sockerr;
		socklen_t socklen = sizeof(sockerr);
		getsockopt(mFd, SOL_SOCKET, SO_ERROR, &sockerr, &socklen);
		dbgd("*** read error: ret=%d, sock err=%d, bufsize=%d, errno=%d", ret, sockerr, size, errno);
	}

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

int EdSocket::sendto(const char* destaddr, unsigned int addrlen, const void* buf, int len)
{
	sockaddr_un un;
	if (addrlen > sizeof(un.sun_path) - 2)
		return -1;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strncpy(un.sun_path, destaddr, addrlen);
	int slen = sizeof(un.sun_family) + addrlen;
	int wcnt = ::sendto(getFd(), buf, len, 0, (sockaddr*) &un, slen);
	return wcnt;
}

int EdSocket::sendto(const char* destaddr, const void* buf, int len)
{
	return sendto(destaddr, strlen(destaddr), buf, len);
}

void EdSocket::OnRead(void)
{
	if(mOnLis != nullptr)
		mOnLis(*this, SOCK_EVENT_READ);
}

void EdSocket::OnDisconnected(void)
{
	if(mOnLis != nullptr)
		mOnLis(*this, SOCK_EVENT_DISCONNECTED);
}

void EdSocket::OnWrite(void)
{
	if(mOnLis != nullptr)
		mOnLis(*this, SOCK_EVENT_WRITE);
}

void EdSocket::OnConnected(void)
{
	if(mOnLis != nullptr)
		mOnLis(*this, SOCK_EVENT_CONNECTED);
}

void EdSocket::OnIncomingConnection(void)
{
	if(mOnLis != nullptr)
		mOnLis(*this, SOCK_EVENT_INCOMING_ACCEPT);
}

void EdSocket::OnEventRead()
{
	if (mIsListen == false)
	{
		mRaiseDisconnect = 0;
		OnRead();
#if 1
		/* Note:
		 */
		if (EdTask::getCurrentTask()->lastSockErrorNo != 0)
		{
			OnDisconnected();
			if (EdTask::getCurrentTask()->lastSockErrorNo != 0)
			{
				dbgd("  socket not closed by user in disconnected status !!!, closing socket automatically...");
				close();
			}
		}
#else
		if (mRaiseDisconnect != 0)
		{
			close();
			OnDisconnected();
		}
#endif
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

void EdSocket::setOnListener(SocketListener dg)
{
	mOnLis = dg;
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

ssize_t EdSocket::recvFromUnix(void* buf, int size, string* fromaddr)
{
	sockaddr_un un;
	socklen_t slen = sizeof(un);
	int rcnt = recvfrom(getFd(), buf, size, 0, (sockaddr*) &un, &slen);
	fromaddr->append(un.sun_path, strlen(un.sun_path));
	return rcnt;
}


ssize_t EdSocket::recvFrom(void* buf, int size, unsigned int* ipaddr, unsigned short * port)
{
	sockaddr_in inaddr;
	socklen_t slen = sizeof(inaddr);
	ssize_t rcnt = recvfrom(getFd(), buf, size, 0, (sockaddr*)&inaddr, &slen);
	if(ipaddr != NULL)
		*ipaddr = inaddr.sin_addr.s_addr;
	if(port != NULL)
		*port = ntohs(inaddr.sin_port);
	return rcnt;
}

} /* namespace edft */
