/*
 * EdPipe.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: khkim
 */
#define DBGTAG "EDPIP"
#define DBG_LEVEL DBG_WARN

#include <unistd.h>
#include "EdPipe.h"
#include "edslog.h"

namespace edft
{

EdPipe::EdPipe()
{
	mRecvFd = mSendFd = -1;
}

EdPipe::~EdPipe()
{
	close();
	dbgd("dest : pipe...");
}

void EdPipe::OnEventRead(void)
{
	if (mLis) {
		mLis(EVT_READ);
	}
}

void EdPipe::OnEventWrite(void)
{
	if (mLis)
		mLis(EVT_WRITE);
}

int EdPipe::open(Lis lis) {
	if(lis) mLis = lis;
	int fds[2];
	int ret = pipe(fds);
	if (ret == 0)
	{
		mRecvFd = fds[0], mSendFd = fds[1];
		if (mContext == NULL)
		{
			setDefaultContext();
		}
		setFd(mRecvFd);
		registerEvent(EVT_READ);
	}

	return ret;
}

void EdPipe::close()
{
	if (mRecvFd != -1)
	{
		deregisterEvent();
		::close(mRecvFd);
		::close(mSendFd);
		mRecvFd = mSendFd = -1;
	}

}

int EdPipe::send(const void* buf, int size)
{
	return write(mSendFd, buf, size);
}

int EdPipe::recv(void* buf, int size)
{
	return read(mRecvFd, buf, size);
}

void EdPipe::setOnListener(Lis lis) {
	mLis = lis;
}

}
