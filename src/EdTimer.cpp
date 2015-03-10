/*
 * EdTimer.cpp
 *
 *  Created on: 2014. 02. 10.
 *      Author: khkim
 */
#define DBGTAG "estmr"
#define DBG_LEVEL DBG_WARN

#include <string.h>
#include "EdTimer.h"
#include "edslog.h"

namespace edft
{

EdTimer::EdTimer()
{
	mHitCount = 0;
	mUser = NULL;
	mOnLis = nullptr;
}

EdTimer::~EdTimer()
{
	kill();
}


void EdTimer::setUsec(u64 usec, u64 first_usec)
{
	if (getFd() < 0)
	{
		int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
		setFd(fd);
		registerEvent(EVT_READ);
	}

	mTimerSpec.it_interval.tv_sec = usec / 1000000;
	mTimerSpec.it_interval.tv_nsec = (usec % 1000000) * 1000;
	u64 iusec = (first_usec == 0) ? usec : first_usec;
	mTimerSpec.it_value.tv_sec = iusec / 1000000;
	mTimerSpec.it_value.tv_nsec = (iusec % 1000000) * 1000;

	timerfd_settime(getFd(), 0, &mTimerSpec, NULL);
}

void EdTimer::set(u32 msec, u32 first_msec)
{
	setUsec(msec * 1000, first_msec*1000);
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
	if(mOnLis != nullptr)
		mOnLis(*this);
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
	timerfd_settime(getFd(), 0, &mTimerSpec, NULL);
}

void EdTimer::setOnListener(TimerListener lis)
{
	mOnLis = lis;
}

void EdTimer::OnEventRead(void)
{
	int rcnt = read(getFd(), (void*) &mHitCount, sizeof(mHitCount));
	if (rcnt < 8)
		dbge("#### Error: timer....");
	OnTimer();
}

} // namespace edft
