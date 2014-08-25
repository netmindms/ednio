/*
 * EdCurlSocket.cpp
 *
 *  Created on: Aug 10, 2014
 *      Author: netmind
 */
#define DBGTAG "cusck"
#define DBG_LEVEL DBG_WARN
#include "EdCurlSocket.h"
#include "EdMultiCurl.h"
#include "../edslog.h"

namespace edft
{
EdCurlSocket::EdCurlSocket()
{
	mEdMultiCurl = NULL;
}

EdCurlSocket::~EdCurlSocket()
{
}

void EdCurlSocket::setEvent(int eventflag)
{
	if (mIsReg)
		changeEvent (eventflag);
	else
		registerEvent(eventflag);
}

void EdCurlSocket::OnEventRead()
{
	mEdMultiCurl->procEventRead(mFd);
}

void EdCurlSocket::OnEventWrite()
{
	//logd("on event write...");
	mEdMultiCurl->procEventWrite(mFd);
}

void EdCurlSocket::OnEventHangup()
{
	mEdMultiCurl->procEventErr(mFd);
}


void EdCurlSocket::open(EdMultiCurl* edmcurl, int fd)
{
	mEdMultiCurl = edmcurl;
	setFd(fd);
}

void EdCurlSocket::curlSockCb(int what)
{

	if (what == CURL_POLL_NONE)
	{
		dbgd("fd=%d, curl poll none...", mFd);
	}
	else if (what == CURL_POLL_IN)
	{
		dbgd("fd=%d, curl poll in...", mFd);
		setEvent(EVT_READ);
	}
	else if (what == CURL_POLL_OUT)
	{
		dbgd("fd=%d, curl poll out...", mFd);
		setEvent(EVT_WRITE);
	}
	else if (what == CURL_POLL_INOUT)
	{
		dbgd("fd=%d, curl poll inout...", mFd);
		setEvent(EVT_WRITE | EVT_READ);
	}
	else if (what == CURL_POLL_REMOVE)
	{
		dbgd("fd=%d, curl poll remove...", mFd);
		deregisterEvent();
	}
}



} // namespace edft
