/*
 * EdSignalFd.cpp
 *
 *  Created on: Apr 4, 2015
 *      Author: netmind
 */

#define DBGTAG "SIGFD"
#define DBG_LEVEL DBG_WARN

#include"edslog.h"
#include "EdSignalFd.h"

using namespace std;

namespace edft
{

EdSignalFd::EdSignalFd()
{
}

EdSignalFd::~EdSignalFd()
{
	close();
}

void EdSignalFd::OnEventRead()
{
	dbgd(" signal on read...");
	struct signalfd_siginfo fdsi;
	auto rcnt = read(mFd, &fdsi, sizeof(fdsi));
	dbgd("  read signal, fd=%d, rcnt=%d", mFd, rcnt);
	if(rcnt != sizeof(fdsi))
	{
		close();
		return;
	}

	if(mLis != nullptr)
		mLis(fdsi.ssi_signo);
}

void EdSignalFd::setOnListener(lfvu lis)
{
	mLis = lis;
}

int EdSignalFd::setSignal(vector<int> mask_list)
{
	sigset_t mask;
	sigemptyset(&mask);
	for (auto m : mask_list)
	{
		sigaddset(&mask, m);
	}
	pthread_sigmask(SIG_BLOCK, &mask, nullptr);

	int fd = signalfd(-1, &mask, SFD_NONBLOCK);
	dbgd("signal fd=%d", fd);
	if(fd<0)
		return -1;

	setFd(fd);
	registerEvent(EVT_READ);

	return 0;
}

void EdSignalFd::close()
{
	if(mFd>=0)
	{
		deregisterEvent();
		::close(mFd);
		mFd = -1;
	}
}

}
