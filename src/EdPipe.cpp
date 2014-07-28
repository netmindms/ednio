/*
 * EdPipe.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: khkim
 */

#include <unistd.h>
#include "EdPipe.h"

namespace edft
{

EdPipe::EdPipe()
{
	mPipeCb = NULL;
	mRecvFd = mSendFd = -1;
}

EdPipe::~EdPipe()
{
	close();
}

void EdPipe::OnEventRead(void)
{
	if (mPipeCb)
		mPipeCb->IOnPipeEvent(this, EVT_READ);
		//mPipeCb->IOnPipeRead(this);
}

void EdPipe::OnEventWrite(void)
{
	if (mPipeCb)
		mPipeCb->IOnPipeEvent(this, EVT_READ);
		//mPipeCb->IOnPipeWrite(this);
}

void EdPipe::open()
{
	int fds[2];
	pipe(fds);
	mRecvFd = fds[0], mSendFd = fds[1];
	if (mContext == NULL)
	{
		setDefaultContext();
	}
	setFd(mRecvFd);
	registerEvent(EVT_READ);
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

void EdPipe::setCallback(IPipeCb* cb)
{
	mPipeCb = cb;
}

}
