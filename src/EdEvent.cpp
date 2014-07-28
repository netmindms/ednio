/*
 * EdEvent.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: khkim
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "esevt"
#include <string.h>
#include "EdEvent.h"
#include "edslog.h"
#include "EdTask.h"

namespace edft
{

EdEvent::EdEvent()
{
	initMembers();
}

EdEvent::EdEvent(EdContext* ctx)
{
	initMembers();
	mContext = ctx;

}

EdEvent::~EdEvent()
{
	deregisterEvent();
}

void EdEvent::OnEventRead(void)
{
}

void EdEvent::OnEventWrite(void)
{
}

void EdEvent::registerEvent(uint16_t flag)
{
	if (mIsReg == true)
		return;

	if (mContext->mode == MODE_EDEV)
	{
		if (mEvt == NULL)
		{
			//mEvt = es_event_reg(mContext, mFd, flag, esevent_cb, (void*) this);
			mEvt = mTask->regEdEvent(mFd, flag, esevent_cb, (void*) this);
			mIsReg = true;
		}
	}
	else
	{
#ifdef USE_LIBEVENT
		dbgd("register event, fd=%d,", mFd);
		uint16_t evtflag=0;
		if(!(flag & EVT_ONESHOT))
			evtflag |= EV_PERSIST;
		if (flag & EVT_READ)
			evtflag |= EV_READ;
		if (flag & EVT_WRITE)
			evtflag |= EV_WRITE;

		mEvent = event_new(mContext->eventBase, mFd, evtflag, libevent_cb,	(void*) this);
		event_add(mEvent, NULL);
		mIsReg = true;
#endif

	}

}

void EdEvent::deregisterEvent(void)
{
	if (mIsReg == false)
	{
		return;
	}

	if (mContext->mode == MODE_EDEV)
	{
		if (mEvt)
		{
			mTask->deregEdEvent(mEvt);
			mEvt = NULL;
		}
	}
	else
	{
#ifdef USE_LIBEVENT
		if (mEvent)
		{
			dbgd("deregister libevent...");
			event_del(mEvent);
			event_free(mEvent);
			mEvent = NULL;
		}
#endif
	}
	mIsReg = false;
}

void EdEvent::esevent_cb(edevt_t* pevt, int fd, int events)
{
	EdEvent *esevt = (EdEvent*) pevt->user;
	if (events & EVT_READ)
	{
		esevt->OnEventRead();
	}

	if (events & EVT_WRITE)
	{
		esevt->OnEventWrite();
	}
}

#ifdef USE_LIBEVENT
void EdEvent::libevent_cb(int fd, short flags, void* arg)
{
	EdEvent *ev = (EdEvent*) arg;
	dbgv("event proc, base=%p, mfd=%d, fd=%d, flags=%0x, esevent=%p", ev->mContext->eventBase, ev->mFd, fd, flags, ev);
	if (ev->mFd < 0)
	{
		dbge("###### fd error...");
		assert(0);
	}
	if (flags & EV_READ)
	{
		ev->OnEventRead();
	}

	if (flags & EV_WRITE)
	{
		ev->OnEventWrite();
	}
}
#endif

void EdEvent::changeEvent(uint16_t flags)
{
	if (mContext->mode == MODE_EDEV)
	{
		mTask->changeEdEvent(mEvt, flags);
	}
	else
	{
#ifdef USE_LIBEVENT
		event_del(mEvent);
		event_free(mEvent);
		uint16_t evtflag = EV_PERSIST;
		if (flags & EVT_READ)
			evtflag |= EV_READ;
		if (flags & EVT_WRITE)
			evtflag |= EV_WRITE;
		mEvent = event_new(mContext->eventBase, mFd, evtflag, libevent_cb,
				(void*) this);
		event_add(mEvent, NULL);
#endif
	}
}

EdContext* EdEvent::getContext()
{
	return mContext;
}

void EdEvent::setNonBlockMode(void)
{
	int flags = fcntl(mFd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(mFd, F_SETFL, flags);
}

void EdEvent::setContext(EdContext* ctx)
{
	mContext = ctx;
}

void EdEvent::initMembers()
{
#ifdef USE_LIBEVENT
	mEvent = NULL;
#endif
	mIsReg = false;
	mUser = NULL;
	mFd = -1;
	mEvt = NULL;
	mTask = NULL;
	mContext = _tEdContext;
}

void* EdEvent::getUser()
{
	return mUser;
}

void EdEvent::setUser(void* user)
{
	mUser = user;
}

void EdEvent::setDefaultContext()
{
	mContext = _tEdContext;
}

void EdEvent::setFd(int fd)
{
	mFd = fd;
	setNonBlockMode();
	mContext = _tEdContext;
	mTask = _tEdTask;
}


int EdEvent::getFd()
{
	return mFd;
}


} /* namespace edft */