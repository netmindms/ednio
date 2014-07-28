/*
 * EdTimer.cpp
 *
 *  Created on: 2014. 02. 10.
 *      Author: khkim
 */
#define DBGTAG "estmr"
#define DBG_LEVEL DBG_DEBUG


#include <string.h>
#include "EdTimer.h"
#include "edslog.h"

namespace edft
{

EdTimer::EdTimer()
{
	mInterval = 0;
	mHitCount = 0;
	miCallback = NULL;
	mUser = NULL;

}

EdTimer::~EdTimer()
{
	kill();
}

void EdTimer::setCallback(ITimerCb* itimer)
{
	dbgd("setcallback, cb = %p", itimer);
	miCallback = itimer;
}

void EdTimer::set(u32 msec, u32 first_msec)
{
	if (mFd < 0)
	{
		int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
		setFd(fd);
		registerEvent(EVT_READ);
	}


	mInterval = msec;

	struct itimerspec newtm;

	newtm.it_interval.tv_sec = msec / 1000;
	newtm.it_interval.tv_nsec = (msec % 1000) * 1000 * 1000;

	u32 imsec = (first_msec==0) ? msec:first_msec;
	newtm.it_value.tv_sec = imsec / 1000;
	newtm.it_value.tv_nsec = (imsec % 1000) * 1000 * 1000;

	timerfd_settime(mFd, 0, &newtm, NULL);

}

void EdTimer::kill(void)
{
	if (mFd < 0)
		return;
	close(mFd);
	deregisterEvent();
	mFd = -1;
}

void EdTimer::OnTimer()
{
	if (miCallback)
	{
		miCallback->IOnTimerEvent(this);
	}
}

void EdTimer::reset(void)
{
	pause();
	resume();
}

bool EdTimer::isActive(void)
{
	return (mFd >= 0 ? true : false);
}

void EdTimer::pause(void)
{
	struct itimerspec ts;
	memset(&ts, 0, sizeof(ts));
	timerfd_settime(mFd, 0, &ts, NULL);
}

void EdTimer::resume(void)
{
	struct itimerspec ts;
	ts.it_interval.tv_sec = mInterval / 1000;
	ts.it_interval.tv_nsec = (mInterval % 1000) * 1000 * 1000;
	ts.it_value.tv_sec = mInterval / 1000;
	ts.it_value.tv_nsec = ts.it_interval.tv_nsec;
	timerfd_settime(mFd, 0, &ts, NULL);
}

void EdTimer::OnEventRead(void)
{
	int rcnt = read(mFd, (void*) &mHitCount, sizeof(mHitCount));
	if (rcnt < 8)
		dbge("#### Error: timer....");
	OnTimer();
}

}
