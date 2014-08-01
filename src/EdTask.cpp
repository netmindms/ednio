/*
 * EdTask.cpp
 *
 *  Created on: 2014. 02. 10.
 *      Author: khkim
 */
#include "config.h"

#define DBG_LEVEL DBG_WARN
#define DBGTAG "etask"


#if USE_LIBEVENT
#include <event2/util.h>
#include <event2/event.h>
#endif

#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <cassert>
#include <cstdlib>
#include <sys/eventfd.h>
#include <stdexcept>
#include <string.h>
#include <fcntl.h>

#include "EdTask.h"
#include "edslog.h"
#include "EdEvent.h"


namespace edft
{

__thread class EdTask *_tEdTask;

EdTask::EdTask(int nmsgq)
{
	mTid = 0;
	mMaxMsqQueSize = nmsgq;
	memset(&mCtx, 0, sizeof(mCtx));
#if USE_LIBEVENT
	mLibMsgEvent = NULL;
#endif
	mEdMsgEvt = NULL;
	mMsgFd = -1;
}

EdTask::~EdTask()
{
	closeMsg();
}

int EdTask::run(int mode)
{

	initMsg();
	mCtx.mode = mode;
	if (mode == MODE_EDEV)
	{

		pthread_create(&mTid, NULL, esev_thread, this);
		return sendMsg(EDM_INIT);
	}
	else
	{
#if USE_LIBEVENT
		pthread_create(&mTid, NULL, libevent_thread, this);
		return sendMsg(EDM_INIT);
#else
		dbge("### libevent not included in this build.......");
		assert(0);
		return -1;
#endif
	}
}

void EdTask::wait(void)
{
	pthread_join(mTid, NULL);
}

void EdTask::terminate(void)
{
	if (mCtx.mode == MODE_EDEV)
	{
		postMsg(EDM_EXIT, 0, 0);
		pthread_join(mTid, NULL);
	}
	else
	{
#if USE_LIBEVENT
		postExit();
		wait();
#endif
	}
}

void* EdTask::esev_thread(void* arg)
{
	EdTask* ptask = (EdTask*) arg;
	ptask->threadMain();
	return NULL;

}


#if USE_LIBEVENT
void* EdTask::libevent_thread(void* arg)
{
	dbgd("libevent thread proc start...");
	EdTask *ptask = (EdTask*) arg;
	EdContext* pctx = &ptask->mCtx;
	_tEdContext = &ptask->mCtx;

	pctx->eventBase = event_base_new();
	ptask->mLibMsgEvent = event_new(pctx->eventBase, ptask->mMsgFd, EV_READ | EV_PERSIST, libevent_cb, (void*) ptask);
	event_add(ptask->mLibMsgEvent, NULL);
	//event_base_dispatch(_gEdContext.eventBase);
	event_base_loop(pctx->eventBase, 0);
	ptask->cleanupAllTimer();
	event_del(ptask->mLibMsgEvent);
	event_free(ptask->mLibMsgEvent);
	ptask->mLibMsgEvent = NULL;
	ptask->callMsgClose();

	event_base_free(pctx->eventBase);
	pctx->eventBase = NULL;

	return NULL;
}
#endif


int EdTask::postEdMsg(u16 msgid, u64 data)
{

	mMsgMutex.lock();

	int ret = 0;

	EdMsg* pmsg;
	pmsg = allocMsgObj();

	if (pmsg)
	{
		uint64_t t = 1;
		pmsg->msgid = msgid;
		pmsg->data = data;
		pmsg->sync = 0;
#ifdef MSGLIST_ED
		mQuedMsgs.push_back(pmsg);
#else
		mQuedMsgs.push_back(pmsg);
#endif
		ret = write(mMsgFd, &t, sizeof(t));
		ret = (ret == 8) ? 0 : -2;
	}
	else
	{
		dbge("### Error: message queue full !!!....., eventfd=%d", mMsgFd);
		{
			// event_fd 가 0 인경우 즉, close_msg_proc 가 호출이 된 경우는 정상적이라 판단한다.
			// event_fd가 살아 있는 경우에도 이쪽으로 빠졌으면 message que full이라 판단한다.
			assert(mMsgFd == 0);
		}
		ret = -1;
	}

	mMsgMutex.unlock();
	return ret;
}

int EdTask::postMsg(u16 msgid, u32 p1, u32 p2)
{
	return postEdMsg(msgid, ((u64) p2 << 32) | p1);
}

int EdTask::postObj(u16 msgid, void *obj)
{
	return postEdMsg(msgid, (u64) obj);
}

int EdTask::sendEdMsg(u16 msgid, u64 data)
{
	mMsgMutex.lock();

	int ret;
	EdMsg *pmsg;
	pmsg = allocMsgObj();
	if (pmsg)
	{
		pthread_cond_t cond;
		pthread_mutex_t mutex;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
		pmsg->pmsg_sync_mutex = &mutex;
		pmsg->pmsg_sig = &cond;

		uint64_t t = 1;
		pmsg->msgid = msgid;

		pmsg->data = data;
		pmsg->sync = 1;
		pmsg->result = 0;

		pthread_mutex_lock(&mutex);
#ifdef MSGLIST_ED
		mQuedMsgs.push_back(pmsg);
#else
		mQuedMsgs.push_back(pmsg);
#endif
		mMsgMutex.unlock();

		ret = write(mMsgFd, &t, sizeof(t));
		if (ret == 8)
		{
			pthread_cond_wait(&cond, &mutex);
			pthread_mutex_unlock(&mutex);
			ret = pmsg->result;
		}
		else
		{
			pthread_mutex_unlock(&mutex);
			dbge("### send msg error, ret=%d", ret);
			assert(0);
			ret = -1;
		}
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);

	}
	else
	{
		dbge("### Error: message queue full !!!....., eventfd=%d", mMsgFd);
		assert(mMsgFd == 0);

		mMsgMutex.unlock();

		ret = -1;
	}

	return ret;
}

int EdTask::sendMsg(u16 msgid, u32 p1, u32 p2)
{

	return sendEdMsg(msgid, (((u64) p2 << 32) | p1));

}

void EdTask::postCloseTask(void)
{
	postMsg(EDM_EXIT, 0, 0);
}

void EdTask::initMsg()
{
	for (int i = 0; i < mMaxMsqQueSize; i++)
	{
#ifdef MSGLIST_ED
		EdMsg* pmsg = mEmptyMsgs.allocObj();
		mEmptyMsgs.push_back(pmsg);
#else
		EdMsg* pmsg = new EdMsg;
		mEmptyMsgs.push_back(pmsg);
#endif
	}
	dbgd("msg que size=%d", mEmptyMsgs.size());
	dbgd("    msg que num=%d", mMaxMsqQueSize);
	mMsgFd = eventfd(0, EFD_NONBLOCK);
}

void EdTask::closeMsg()
{
	dbgv("close task msg, task=%p, empty=%d, qued=%d", this, mEmptyMsgs.size(), mQuedMsgs.size());
	if (mMsgFd >= 0)
	{
		close(mMsgFd);
		mMsgFd = -1;
	}

#ifdef MSGLIST_ED
	EdMsg* pmsg;
	if(mQuedMsgs.size() > 0) {
		dbgw("#### qued msg exist......cnt=%d", mQuedMsgs.size());
		for(;;)
		{
			pmsg = mQuedMsgs.pop_front();
			if(pmsg)
			{
				EdObjList<EdMsg>::freeObj(pmsg);
			}
			else
				break;
		}

	}

	for(;;)
	{

		pmsg = mEmptyMsgs.pop_front();
		if(pmsg)
		{
			EdObjList<EdMsg>::freeObj(pmsg);
		}
		else
			break;
	}


#else
	if (mQuedMsgs.size() != 0)
	{
		dbgw("#### qued msg exist......cnt=%d", mQuedMsgs.size());
		for (auto itr = mQuedMsgs.begin(); itr != mQuedMsgs.end(); itr++)
		{
			delete (*itr);
		}
	}

	for (auto itr = mEmptyMsgs.begin(); itr != mEmptyMsgs.end(); itr++)
	{
		delete (*itr);
	}
#endif
}

int EdTask::setTimer(u32 id, u32 msec, u32 inittime)
{
	TaskTimer *pt;
	try
	{
		TaskTimer* &p = mTimerMap.at(id);
		pt = p;

	} catch (std::out_of_range &exp)
	{
		pt = new TaskTimer(id);
		mTimerMap[id] = pt;
		pt->setUser((void*) this);
	}
	pt->set(msec, inittime);
	return 0;

}

void EdTask::killTimer(u32 id)
{
	unordered_map<u32, TaskTimer*>::iterator it = mTimerMap.find(id);
	if (it != mTimerMap.end())
	{
		TaskTimer *p = it->second;
		p->kill();
		mTimerMap.erase(it);
		delete p;
		dbgv("kill task timer, id=%u, timercount=%d", id, mTimerMap.size());
	}
}

void EdTask::cleanupAllTimer()
{
	TaskTimer *pt;
	unordered_map<u32, TaskTimer*>::iterator it;

	for (it = mTimerMap.begin(); it != mTimerMap.end(); it++)
	{
		pt = it->second;
		dbgw("### cleanup pending timer, id=%u", pt->timerId);
		pt->kill();
		delete pt;
	}

	mTimerMap.clear();
}

int EdTask::sendObj(u16 msgid, void* obj)
{
	return sendEdMsg(msgid, (u64) obj);
}

int EdTask::OnEventProc(EdMsg* pmsg)
{
	return 0;
}

void EdTask::dispatchMsgs(int cnt)
{
	dbgv("dispatch msgs.....cnt=%d", cnt);
	for (int i = 0; i < cnt; i++)
	{
		EdMsg *pmsg;
		mMsgMutex.lock();
#ifdef MSGLIST_ED
		pmsg = mQuedMsgs.pop_front();
#else
		if (!mQuedMsgs.empty())
		{
			pmsg = mQuedMsgs.front();
			mQuedMsgs.pop_front();
		}
#endif
		mMsgMutex.unlock();
		if (pmsg)
		{
			if (pmsg->msgid == EDM_EXIT)
			{
				mCtx.exit_flag = 1;
#if USE_LIBEVENT
				if (mCtx.mode == MODE_LIBEVENT)
				{
					event_base_loopexit(mCtx.eventBase, NULL);
				}
#endif
				dbgd("EM_EXIT message .... event loop will be exited....");
			}
			else
			{
				OnEventProc(pmsg);
			}
			if (pmsg->sync)
			{
				dbgv("== send msg recv....id=%d", pmsg->msgid);
				pthread_mutex_lock(pmsg->pmsg_sync_mutex);
				pthread_cond_signal(pmsg->pmsg_sig);
				pthread_mutex_unlock(pmsg->pmsg_sync_mutex);
				dbgv("== send msg recv end....id=%d", pmsg->msgid);
			}

			mMsgMutex.lock();
			mEmptyMsgs.push_back(pmsg);
			mMsgMutex.unlock();
		}
		else
		{
			dbge("### unexpected msg que error...");
			break;
		}

	}
}

void EdTask::msgevent_cb(edevt_t* pevt, int fd, int events)
{
	uint64_t cnt;
	EdTask* ptask = (EdTask*) pevt->user;

	read(fd, &cnt, sizeof(cnt));
	dbgv("eventfd cnt=%d", cnt);
	ptask->dispatchMsgs(cnt);
#if 0
	for (uint32_t i = 0; i < cnt; i++)
	{
		EdMsg *pmsg = task->mMsgQue.get_used();
		if (pmsg)
		{
			int msg_ret;
			if (pmsg->msgid == EM_EXIT)
			{
				pevt->pEdCtx->exit_flag = 1;
				dbgd("EM_EXIT message .... event loop will be exited....");
			}
			else
			{
				msg_ret = task->OnEventProc(pmsg);
			}
			if (pmsg->sync)
			{
				dbgv("== send msg recv....id=%d", pmsg->msgid);
				*pmsg->perror = msg_ret;
				pthread_mutex_lock(pmsg->pmsg_sync_mutex);
				pthread_cond_signal(pmsg->pmsg_sig);
				pthread_mutex_unlock(pmsg->pmsg_sync_mutex);
				dbgv("== send msg recv end....id=%d", pmsg->msgid);
			}
			task->mMsgQue.put_free(pmsg);
		}
		else
		{
			dbge("### unexpected msg que error...");
			break;
		}

	}
#endif

}

void EdTask::postExit(void)
{
	postMsg(EDM_EXIT);
}

void EdTask::callMsgClose()
{
	EdMsg msg;
	msg.msgid = EDM_CLOSE;
	OnEventProc(&msg);
}

#if USE_LIBEVENT
void EdTask::libevent_cb(evutil_socket_t fd, short shortInt, void*user)
{
	dbgv("libevent callback, fd=%d", fd);
	EdTask* ptask = (EdTask*) user;
	u64 cnt;
	read(ptask->mMsgFd, &cnt, sizeof(cnt));
	ptask->dispatchMsgs(cnt);
}
#endif

EdTask::TaskTimer::TaskTimer(u32 id)
{
	timerId = id;
}

void EdTask::TaskTimer::OnTimer()
{
	EdTask* ptask = (EdTask*) getUser();
	EdMsg msg;
	msg.msgid = EDM_TIMER;
	msg.p1 = timerId;
	ptask->OnEventProc(&msg);
}

int EdTask::esOpen(void* user)
{
	memset(&mCtx, 0, sizeof(EdContext));

	mCtx.epfd = epoll_create(10);
	if (!mCtx.epfd)
	{
		dbge("### epoll open error...");
		assert(0);
	}

	mCtx.exit_flag = 0;
	mCtx.evt_count = 0;

	mCtx.user = user;



	mCtx.opened = 1;
	dbgd("=== es opened...");
	return mCtx.epfd;
}

edevt_t* EdTask::regEdEvent(int fd, uint32_t events, EVENTCB cb, void* user)
{

	struct epoll_event event;
	dbgd("esEventReg, fd=%d", fd);
	if (cb == NULL)
		assert(0);

	edevt_t *pevt = allocEvent(&mCtx);

	pevt->fd = fd;
	pevt->evtcb = cb;
	pevt->user = user;

	event.data.ptr = pevt;
	event.events = events;

	mCtx.evt_count++;

	dbgd("    evt=%p, esevent=%p", pevt, pevt->user);

	int ret = epoll_ctl(mCtx.epfd, EPOLL_CTL_ADD, fd, &event);
	if (ret)
	{
		dbge("### epoll_ctl error...ret=%d", ret);
		freeEvent(pevt);
		return NULL;
	}

	pevt->isReg = true;
	mEvtList.push_back(pevt);

	dbgv("== reg event, fd=%d, event_obj=%p, event=%d, count=%d, ret=%d ", fd,	pevt, events, mEvtList.size(), ret);

	return pevt;
}


edevt_t* EdTask::allocEvent(EdContext* psys)
{
	edevt_t *pevt = mEvtList.allocObj();
	if (pevt)
	{
		memset(pevt, 0, sizeof(edevt_t));
		pevt->pEdCtx = psys;
		psys->evt_alloc_cnt++;
	}

	return pevt;
}

void EdTask::freeEvent(edevt_t* pevt)
{
	pevt->pEdCtx->evt_alloc_cnt--;
	mEvtList.freeObj(pevt);
}

int EdTask::esMain(EdContext* psys)
{
	int i;
	int nfds;
	struct epoll_event events[MAX_GET_EVENTS];

	dbgv("es main......");
	for (; psys->exit_flag == 0;)
	{
		nfds = epoll_wait(psys->epfd, events, MAX_GET_EVENTS, -1);
		dbgv("nfds = %d ", nfds);
		struct epoll_event *epv;
		for (i = 0; i < nfds; i++)
		{
			epv = events + i;
			edevt_t* pevt = (edevt_t*) epv->data.ptr;
			dbgv("event data.ptr=%p, pevtuser=%p, event=%0x", pevt, pevt->user, epv->events);

			if(epv->events & EPOLLIN)
				pevt->evtcb(pevt, pevt->fd, EVT_READ);
			if(pevt->isReg==false)
				goto __release_event__;

			if( (epv->events & EPOLLRDHUP) || (epv->events & EPOLLHUP) || (epv->events & EPOLLERR))
				pevt->evtcb(pevt, pevt->fd, EVT_HANGUP);
			if(pevt->isReg==false)
				goto __release_event__;

			if(epv->events & EPOLLOUT)
				pevt->evtcb(pevt, pevt->fd, EVT_WRITE);
			if(pevt->isReg==false)
				goto __release_event__;

__release_event__:
			// check if event is dereg.
			cleanUpEventResource();
		}
	}
	psys->opened = 0;
	usleep(1);

	return 0;
}

void EdTask::threadMain()
{
	dbgd("start thread miain......");
	_tEdTask = this;
	EdContext* pctx = &mCtx;
	_tEdContext = pctx;

	esOpen(this);

	mEdMsgEvt = regEdEvent(mMsgFd, EVT_READ, msgevent_cb, this);

	// main event loop
	esMain(pctx);

	cleanupAllTimer();

	// dereg eventfd for ipc msg.
	deregEdEvent(mEdMsgEvt);
	callMsgClose();
	// cleanup event resources.
	dbgd("After EDM_CLOSE, free remaining event resource, size=%d", mDummyEvtList.size());
	cleanUpEventResource();

	if(mEvtList.size()>0)
	{
		dbge("##### There are still active events..count=%d, Missing closing events???", mEvtList.size());
		for(edevt_t* pevt=mEvtList.pop_front();pevt;)
		{
			dbge("  missed fd: %d", pevt->fd);
			pevt = mEvtList.pop_front();
		}
		assert(0);
	}

	esClose(pctx);

	_tEdContext = NULL;
}


void EdTask::deregEdEvent(edevt_t* pevt)
{
	EdContext* pctx = &mCtx;
	dbgd("esEventDereg fd=%d", pevt->fd);

	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	epoll_ctl(mCtx.epfd, EPOLL_CTL_DEL, pevt->fd, &event);
	pctx->evt_count--;
	dbgv("== dereg event, event_obj=%p, count=%d", pevt, pctx->evt_count);
	pevt->isReg = false;
	mEvtList.remove(pevt);

	mDummyEvtList.push_back(pevt);

}

void EdTask::esClose(EdContext* pctx)
{
	if(pctx->epfd > 0)
		close(pctx->epfd);
	pctx->epfd = -1;

	// check free resource
	if (pctx->evt_alloc_cnt > 0)
	{
		dbge("### Error: event leak....");
		assert(0);
	}

	if (pctx->timer_alloc_cnt > 0)
	{
		dbge("### Error: timer leak....");
		assert(0);
	}

	pctx->opened = 0;
}

int EdTask::changeEdEvent(edevt_t* pevt, uint32_t event)
{
	int ret;
	struct epoll_event ev;
	ev.events = event;
	ev.data.ptr = pevt;
	ret = epoll_ctl(pevt->pEdCtx->epfd, EPOLL_CTL_MOD, pevt->fd, &ev);
	dbgv("== es_change, fd=%d, cb=%p, user=%p, ret=%d", pevt->fd, pevt->evtcb,
			pevt->user, ret);
	return ret;
}

EdMsg* EdTask::allocMsgObj()
{
	EdMsg *pmsg;
#ifdef MSGLIST_ED
	pmsg = mEmptyMsgs.pop_front();

#else
	if (!mEmptyMsgs.empty())
	{
		pmsg = mEmptyMsgs.front();
		mEmptyMsgs.pop_front();
	}
	else
		pmsg = NULL;
#endif
	return pmsg;
}


void EdTask::setSendMsgResult(EdMsg* pmsg, int code)
{
	pmsg->result = code;
}


void EdTask::cleanUpEventResource()
{
	edevt_t* pevt;
	for(pevt = mDummyEvtList.pop_front();pevt;)
	{
		dbgd("free event resource=%p", pevt);
		freeEvent(pevt);
		pevt = mDummyEvtList.pop_front();
	}
}

} // namespace edft
