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

#include "EdNio.h"
#include "EdTask.h"
#include "edslog.h"
#include "EdEvent.h"
#include "edcurl/EdCurl.h"
#include "EdEventFd.h"

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

	mRunMode = MODE_EDEV;
}

EdTask::~EdTask()
{
	//closeMsg();
}

void* EdTask::task_thread(void* arg)
{
	EdTask* ptask = (EdTask*) arg;
	ptask->taskProc();
	return NULL;
}

int EdTask::run(int mode)
{

	initMsg();
	mRunMode = mode;
#if 1
	pthread_create(&mTid, NULL, task_thread, this);
	return sendMsg(EDM_INIT);
#else
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
#endif
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
	ptask->libeventMain(pctx);
	return NULL;
}

void EdTask::libeventMain(EdContext* pctx)
{
	_tEdTask = this;
	_tEdContext = &mCtx;

	esOpen(this);

	pctx->eventBase = event_base_new();
	mLibMsgEvent = event_new(pctx->eventBase, mMsgFd, EV_READ | EV_PERSIST, libevent_cb, (void*) this);
	event_add(mLibMsgEvent, NULL);
	//event_base_dispatch(_gEdContext.eventBase);

	mFreeEvent = new FreeEvent;
	mFreeEvent->open();

	event_base_loop(pctx->eventBase, 0);
	cleanupAllTimer();
	event_del(mLibMsgEvent);
	event_free(mLibMsgEvent);
	mLibMsgEvent = NULL;
	callMsgClose();
	dbgd("free obj list, cnt=%d", mReserveFreeList.size());

	mFreeEvent->close();
	delete mFreeEvent;

	event_base_free(pctx->eventBase);
	pctx->eventBase = NULL;

	esClose(pctx);
}
#endif

int EdTask::postEdMsg(u16 msgid, u64 data)
{

	mMsgMutex.lock();
	if (mMsgFd < 0)
	{
		dbgw("### send message fail : msg fd invalid...");
		mMsgMutex.unlock();
		return -1;
	}

	int ret = 0;

	EdMsg* pmsg;
	pmsg = allocMsgObj();

	if (pmsg)
	{
		uint64_t t = 1;
		pmsg->msgid = msgid;
		pmsg->data = data;
		pmsg->sync = 0;
		mQuedMsgs.push_back(pmsg);
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
	if (mMsgFd < 0)
	{
		dbgw("### send message fail : msg fd invalid...");
		mMsgMutex.unlock();
		return -1;
	}

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
		int result;
		pmsg->msgid = msgid;

		pmsg->data = data;
		pmsg->sync = 1;
		pmsg->psend_result = &result;

		pthread_mutex_lock(&mutex);
		mQuedMsgs.push_back(pmsg);
		mMsgMutex.unlock();

		ret = write(mMsgFd, &t, sizeof(t));
		if (ret == 8)
		{
			dbgd("send cond waiting....");
			pthread_cond_wait(&cond, &mutex);
			pthread_mutex_unlock(&mutex);
			ret = result;
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
		EdMsg* pmsg = mEmptyMsgs.allocObj();
		mEmptyMsgs.push_back(pmsg);
	}
	dbgd("msg que size=%d", mEmptyMsgs.size());
	dbgd("    msg que num=%d", mMaxMsqQueSize);
	mMsgFd = eventfd(0, EFD_NONBLOCK);
}

void EdTask::closeMsg()
{
	mMsgMutex.lock();
	dbgv("close task msg, task=%p, empty=%d, qued=%d", this, mEmptyMsgs.size(), mQuedMsgs.size());
	if (mMsgFd >= 0)
	{
		close(mMsgFd);
		mMsgFd = -1;
	}

	EdMsg* pmsg;
	if (mQuedMsgs.size() > 0)
	{
		dbgw("#### qued msg exist......cnt=%d", mQuedMsgs.size());
		for (;;)
		{
			pmsg = mQuedMsgs.pop_front();
			if (pmsg)
			{
				if (pmsg->sync)
				{
					pthread_mutex_lock(pmsg->pmsg_sync_mutex);
					*pmsg->psend_result = -1000;
					pthread_cond_signal(pmsg->pmsg_sig);
					pthread_mutex_unlock(pmsg->pmsg_sync_mutex);
				}
				mEmptyMsgs.push_back(pmsg);
			}
			else
				break;
		}

	}

	for (;;)
	{

		pmsg = mEmptyMsgs.pop_front();
		if (pmsg)
		{
			EdObjList<EdMsg>::freeObj(pmsg);
		}
		else
			break;
	}
	mMsgMutex.unlock();
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
		pmsg = mQuedMsgs.pop_front();
		mMsgMutex.unlock();
		if (pmsg != NULL)
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
				dbgd("EDM_EXIT message .... event loop will be exited....");
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
	freeReservedObjs();
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
	mCtx.mode = mRunMode;

#if 0
#if USE_SSL
	//const SSL_METHOD *method = TLSv1_client_method();
	const SSL_METHOD *method = TLSv1_method();
	mCtx.sslCtx = SSL_CTX_new(method);
	dbgd("new ssl context = %p", mCtx.sslCtx);
#endif
#endif

	if (mCtx.mode == MODE_EDEV)
	{
		mCtx.epfd = epoll_create(10);
		if (!mCtx.epfd)
		{
			dbge("### epoll open error...");
			assert(0);
		}
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

	dbgd("    evt=%p, esevent=%p, events=%0x", pevt, pevt->user, events);

	int ret = epoll_ctl(mCtx.epfd, EPOLL_CTL_ADD, fd, &event);
	if (ret)
	{
		dbge("### epoll_ctl error...ret=%d", ret);
		freeEvent(pevt);
		return NULL;
	}

	pevt->isReg = true;
	mEvtList.push_back(pevt);

	dbgv("== reg event, fd=%d, event_obj=%p, event=%d, count=%d, ret=%d ", fd, pevt, events, mEvtList.size(), ret);

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

			if (epv->events & EPOLLIN)
				pevt->evtcb(pevt, pevt->fd, EVT_READ);
			if (pevt->isReg == false)
				goto __release_event__;

			if ((epv->events & EPOLLRDHUP) || (epv->events & EPOLLHUP) || (epv->events & EPOLLERR))
				pevt->evtcb(pevt, pevt->fd, EVT_HANGUP);
			if (pevt->isReg == false)
				goto __release_event__;

			if (epv->events & EPOLLOUT)
				pevt->evtcb(pevt, pevt->fd, EVT_WRITE);
			if (pevt->isReg == false)
				goto __release_event__;

			__release_event__:
			// check if event is dereg.
			cleanUpEventResource();
		}
		freeReservedObjs();
	}
	psys->opened = 0;
	usleep(1);

	return 0;
}

void EdTask::taskProc()
{
	dbgd("start thread miain......");
	_tEdTask = this;
	EdContext* pctx = &mCtx;
	_tEdContext = pctx;
	esOpen(this);
	if (mRunMode == MODE_EDEV)
	{
		edEventLoop(pctx);
	}
#if USE_LIBEVENT
	else
	{
		libeventLoop(pctx);
	}
#endif
	closeMsg();
	cleanupAllTimer();
	esClose(pctx);
	_tEdContext = NULL;
}

void EdTask::edEventLoop(EdContext* pctx)
{
	mEdMsgEvt = regEdEvent(mMsgFd, EVT_READ, msgevent_cb, this);
	// main event loop
	esMain(pctx);
	callMsgClose();

	deregEdEvent(mEdMsgEvt);
	cleanUpEventResource();
	if (mEvtList.size() > 0)
	{
		dbge("##### There are still active events..count=%d, Missing closing events???", mEvtList.size());
		for (edevt_t* pevt = mEvtList.pop_front(); pevt;)
		{
			dbge("  missed fd: %d", pevt->fd);
			pevt = mEvtList.pop_front();
		}
		assert(0);
	}

}

#if USE_LIBEVENT
void EdTask::libeventLoop(EdContext* pctx)
{
	pctx->eventBase = event_base_new();
	mLibMsgEvent = event_new(pctx->eventBase, mMsgFd, EV_READ | EV_PERSIST, libevent_cb, (void*) this);
	event_add(mLibMsgEvent, NULL);
	//event_base_dispatch(_gEdContext.eventBase);

	mFreeEvent = new FreeEvent;
	mFreeEvent->open();

	event_base_loop(pctx->eventBase, 0);
	callMsgClose();
	event_del(mLibMsgEvent);
	event_free(mLibMsgEvent);
	mLibMsgEvent = NULL;

	dbgd("free obj list, cnt=%d", mReserveFreeList.size());
	mFreeEvent->close();
	delete mFreeEvent;

	event_base_free(pctx->eventBase);
	pctx->eventBase = NULL;
}
#endif

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

	if (mEvtList.size() > 0)
	{
		dbge("##### There are still active events..count=%d, Missing closing events???", mEvtList.size());
		for (edevt_t* pevt = mEvtList.pop_front(); pevt;)
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
	if (pctx->mode == MODE_EDEV)
	{
		if (pctx->epfd > 0)
			close(pctx->epfd);
		pctx->epfd = -1;
	}

#if 0
#if USE_SSL
	if(pctx->sslCtx)
	{
		SSL_CTX_free(pctx->sslCtx);
		pctx->sslCtx = NULL;
	}
#endif
#endif

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
	dbgv("== change event, fd=%d, cb=%p, user=%p, event=%0x, ret=%d", pevt->fd, pevt->evtcb, pevt->user, event, ret);
	return ret;
}

EdMsg* EdTask::allocMsgObj()
{
	EdMsg *pmsg;
	pmsg = mEmptyMsgs.pop_front();
	return pmsg;
}

void EdTask::setSendMsgResult(EdMsg* pmsg, int code)
{
	*pmsg->psend_result = code;
}

void EdTask::cleanUpEventResource()
{
	edevt_t* pevt;
	for (pevt = mDummyEvtList.pop_front(); pevt;)
	{
		dbgd("free event resource=%p", pevt);
		freeEvent(pevt);
		pevt = mDummyEvtList.pop_front();
	}
}

void EdTask::reserveFree(EdObject* obj)
{
	dbgd("reserve free obj=%x", obj);
	if (obj->mIsFree == false)
	{
		obj->mIsFree = true;
		mReserveFreeList.push_back(obj);
		if (mRunMode == MODE_LIBEVENT)
		{
			mFreeEvent->set();
		}
	}
}

void EdTask::freeReservedObjs()
{
	EdObject* obj;
	do
	{
		dbgd("  remain free=%d", mReserveFreeList.size());
		if (mReserveFreeList.empty() == false)
		{
			obj = mReserveFreeList.front();
			mReserveFreeList.pop_front();
			delete obj;
			dbgd("## delete p=%x", obj);
		}
		else
		{
			break;
		}
	} while (true);
}

void EdTask::FreeEvent::OnEventFd(int cnt)
{
	dbgd("OnEventFd: cleanup reserved free objs, task=%x", getCurrentTask());
	getCurrentTask()->freeReservedObjs();
}

} // namespace edft
