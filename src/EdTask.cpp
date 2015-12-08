/*
 * EdTask.cpp
 *
 *  Created on: 2014. 02. 10.
 *      Author: khkim
 */

#define DBG_LEVEL DBG_VERBOSE
#define DBGTAG "etask"

#if USE_LIBEVENT
#include <event2/util.h>
#include <event2/event.h>
#endif

#include <cstdint>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <cassert>
#include <cstdlib>
#include <sys/eventfd.h>
#include <stdexcept>
#include <string.h>
#include <fcntl.h>
#include <execinfo.h>

#include "EdNio.h"
#include "EdTask.h"
#include "edslog.h"
#include "EdEvent.h"
#include "EdEventFd.h"

using std::unique_lock;

#define DEFAULT_MSQ_SIZE 1000

namespace edft {

__thread class EdTask *_tEdTask=nullptr;

static mutex _gviewMutex;
static unordered_map<u32, ViewInfo> gViewMap;

EdTask::EdTask() {
#if (!USE_STL_THREAD)
	mTid = 0;
#endif
	mMaxMsqQueSize = 0;
	memset(&mCtx, 0, sizeof(mCtx));
#if USE_LIBEVENT
	mLibMsgEvent = NULL;
	mFreeEvent = NULL;
#endif
	mEdMsgEvt = NULL;
	mMsgFd = -1;

	mRunMode = MODE_EDEV;
	lastSockErrorNo = 0;
}

EdTask::~EdTask() {
	//closeMsg();
}

void* EdTask::task_thread(void* arg) {
	EdTask* ptask = (EdTask*) arg;
	ptask->taskProc();
	return NULL;
}

int EdTask::run(int mode) {
	return run(mode, 0, 0, DEFAULT_MSQ_SIZE);
}

int EdTask::runMain(int mode) {
	return runMain(mode, 0, 0, DEFAULT_MSQ_SIZE);
}

int EdTask::run(int mode, u32 p1, u32 p2, int msgqnum) {
	mMaxMsqQueSize = msgqnum;
	initMsg();
	if (mMsgFd <= 0) {
		dbge("### event msg fd error, fd=%d", mMsgFd);
		return -1;
	}
	mRunMode = mode;
#if (USE_STL_THREAD)
	mThread = thread(task_thread, (void*) this);
#else
	auto ret = pthread_create(&mTid, NULL, task_thread, this);
	if (ret)
	{
		return ret;
	}
#endif
	return sendMsg(EDM_INIT, p1, p2);
}

int EdTask::runMain(int mode, u32 p1, u32 p2, int msgqnum) {
	mMaxMsqQueSize = msgqnum;
	initMsg();
	if (mMsgFd <= 0) {
		dbge("### event msg fd error, fd=%d", mMsgFd);
		return -1;
	}
	mRunMode = mode;
#if (!USE_STL_THREAD)
	mTid = pthread_self();
#endif
	postMsg(EDM_INIT, p1, p2);
	task_thread((void*) this);
	return 0;
}

void EdTask::wait(void) {
#if USE_STL_THREAD
	if(mThread.joinable())
		mThread.join();
#else
	pthread_join(mTid, NULL);
#endif
}

void EdTask::terminate(void) {
	if (mCtx.mode == MODE_EDEV) {
#if USE_STL_THREAD
		if (mThread.joinable()) {
			postMsg(EDM_EXIT, 0, 0);
			mThread.join();
		}
#else
		if (mTid != 0)
		{
			postMsg(EDM_EXIT, 0, 0);
			pthread_join(mTid, NULL);
		}
#endif
	}
	else {
#if USE_LIBEVENT
		postExit();
		wait();
#endif
	}
}

int EdTask::postEdMsg(u32 view_handle, u16 msgid, EdMsg &tmsg, int type) {

	mMsgMutex.lock();
	if (mMsgFd < 0) {
		dbgw("### send message fail : msg fd invalid...");
		mMsgMutex.unlock();
		return -1;
	}

	int ret = 0;

	EdMsg* pmsg;
	pmsg = allocMsgObj();

	if (pmsg) {
		uint64_t t = 1;
		pmsg->msgid = msgid;
		if (type == 0) {
			pmsg->data = tmsg.data;
		}
		else {
			pmsg->msgobj = move(tmsg.msgobj);
		}
		pmsg->sync = 0;
		pmsg->taskque_handle = view_handle;
		mQuedMsgs.push_back(pmsg);
		ret = write(mMsgFd, &t, sizeof(t));
		ret = (ret == 8) ? 0 : -2;
	}
	else {
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

int EdTask::postMsg(u16 msgid, u32 p1, u32 p2) {
	EdMsg tmsg;
	tmsg.p1 = p1;
	tmsg.p2 = p2;
	return postEdMsg(0, msgid, tmsg, 0);
}

int EdTask::postObj(u16 msgid, void *obj) {
	EdMsg tmsg;
	tmsg.obj = obj;
	return postEdMsg(0, msgid, tmsg, 0);
}

int EdTask::postObj(u16 msgid, upEdMsgObj obj) {
	EdMsg tmsg;
	tmsg.msgobj = move(obj);
	return postEdMsg(0, msgid, tmsg, 1);
}

#if USE_STL_THREAD
int EdTask::sendEdMsg(u32 handle, u16 msgid, EdMsg &tmsg, int type) {
	mMsgMutex.lock();
	if (mMsgFd < 0) {
		dbgw("### send message fail : msg fd invalid...");
		mMsgMutex.unlock();
		return -1;
	}

	int ret;
	EdMsg *pmsg;
	pmsg = allocMsgObj();
	if (pmsg) {
		pmsg->taskque_handle = handle;
#if USE_STL_THREAD
		condition_variable cv;
		mutex mtx;
		pmsg->pcv = &cv;
		pmsg->pmtx = &mtx;
#else
		pthread_cond_t cond;
		pthread_mutex_t mutex;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
		pmsg->pmsg_sync_mutex = &mutex;
		pmsg->pmsg_sig = &cond;
#endif

		uint64_t t = 1;
		int result = 0;
		pmsg->msgid = msgid;
		if (type == 0)
			pmsg->data = tmsg.data;
		else
			pmsg->msgobj = move(tmsg.msgobj);
		pmsg->sync = 1;
		pmsg->psend_result = &result;

#if USE_STL_THREAD
		unique_lock<mutex> lck(mtx);
		mQuedMsgs.push_back(pmsg);
#else
		pthread_mutex_lock(&mutex);
		mQuedMsgs.push_back(pmsg);
#endif

		mMsgMutex.unlock();

		ret = write(mMsgFd, &t, sizeof(t));
		if (ret == 8) {
			dbgd("send cond waiting....");
#if USE_STL_THREAD
			cv.wait(lck);
			lck.unlock();
#else
			pthread_cond_wait(&cond, &mutex);
			pthread_mutex_unlock(&mutex);
#endif
			ret = result;
		}
		else {
#if USE_STL_THREAD
			lck.unlock();
#else
			pthread_mutex_unlock(&mutex);
#endif
			dbge("### send msg error, ret=%d", ret);
			assert(0);
			ret = -1;
		}
#if 0
		pthread_cond_destroy (&cond);
		pthread_mutex_destroy(&mutex);
#endif

	}
	else {
		dbge("### Error: message queue full !!!....., eventfd=%d", mMsgFd);
		assert(mMsgFd == 0);

		mMsgMutex.unlock();

		ret = -1;
	}

	return ret;
}
#else
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
		int result = 0;
		pmsg->msgid = msgid;

		if(type==0) pmsg->data = tmsg.data;
		else pmsg->msgobj = move(tmsg.msgobj);
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
#endif

int EdTask::sendMsg(u16 msgid, u32 p1, u32 p2) {
	EdMsg tmsg;
	tmsg.p1 = p1;
	tmsg.p2 = p2;
	return sendEdMsg(0, msgid, tmsg, 0);
}

void EdTask::initMsg() {
#if 0
	for (int i = 0; i < mMaxMsqQueSize; i++)
	{
		EdMsg* pmsg = mEmptyMsgs.allocObj();
		mEmptyMsgs.push_back(pmsg);
	}
	dbgd("msg que size=%d", mEmptyMsgs.size());
#endif
	dbgd("    msg que num=%d", mMaxMsqQueSize);
	mMsgFd = eventfd(0, EFD_NONBLOCK);
}

void EdTask::closeMsg() {
	mMsgMutex.lock();
	dbgv("close task msg, task=%p, empty=%d, qued=%d", this, mEmptyMsgs.size(), mQuedMsgs.size());
	if (mMsgFd >= 0) {
		close(mMsgFd);
		mMsgFd = -1;
	}

	EdMsg* pmsg;
	if (mQuedMsgs.size() > 0) {
		dbgw("#### qued msg exist......cnt=%d", mQuedMsgs.size());
		for (;;) {
			pmsg = mQuedMsgs.pop_front();
			if (pmsg) {
				if (pmsg->sync) {
#if USE_STL_THREAD
					unique_lock<mutex> lck(*pmsg->pmtx);
					*pmsg->psend_result = -1000;
					pmsg->pcv->notify_one();
					lck.unlock();
#else
					pthread_mutex_lock(pmsg->pmsg_sync_mutex);
					*pmsg->psend_result = -1000;
					pthread_cond_signal(pmsg->pmsg_sig);
					pthread_mutex_unlock(pmsg->pmsg_sync_mutex);
#endif
				}
				mEmptyMsgs.push_back(pmsg);
			}
			else
				break;
		}

	}

	for (;;) {

		pmsg = mEmptyMsgs.pop_front();
		if (pmsg) {
			EdObjList<EdMsg>::freeObj(pmsg);
		}
		else
			break;
	}
	mMsgMutex.unlock();
}

int EdTask::setTimer(u32 id, u32 msec, u32 inittime) {
	TaskTimer *pt;
	try {
		TaskTimer* &p = mTimerMap.at(id);
		pt = p;

	} catch (std::out_of_range &exp) {
		pt = new TaskTimer(id);
		mTimerMap[id] = pt;
		pt->setUser((void*) this);
	}
	pt->set(msec, inittime);
	return 0;

}

void EdTask::killTimer(u32 id) {
	unordered_map<u32, TaskTimer*>::iterator it = mTimerMap.find(id);
	if (it != mTimerMap.end()) {
		TaskTimer *p = it->second;
		p->kill();
		mTimerMap.erase(it);
		delete p;
		dbgv("kill task timer, id=%u, timercount=%d", id, mTimerMap.size());
	}
}

void EdTask::cleanupAllTimer() {
	TaskTimer *pt;
	unordered_map<u32, TaskTimer*>::iterator it;

	for (it = mTimerMap.begin(); it != mTimerMap.end(); it++) {
		pt = it->second;
		dbgw("### cleanup pending timer, id=%u", pt->timerId);
		pt->kill();
		delete pt;
	}

	mTimerMap.clear();
}

int EdTask::sendObj(u16 msgid, void* obj) {
	EdMsg tmsg;
	tmsg.obj = obj;
	return sendEdMsg(0, msgid, tmsg, 0);
}

int EdTask::sendObj(u16 msgid, upEdMsgObj obj) {
	EdMsg tmsg;
	tmsg.msgobj = move(obj);
	return sendEdMsg(0, msgid, tmsg, 1);
}

int EdTask::OnEventProc(EdMsg& msg) {
	if (mLis)
		return mLis(msg);
	else
		return 0;
}

void EdTask::dispatchMsgs(int cnt) {
	dbgv("dispatch msgs.....cnt=%d", cnt);
	for (int i = 0; i < cnt; i++) {
		EdMsg *pmsg;
		mMsgMutex.lock();
		pmsg = mQuedMsgs.pop_front();
		mMsgMutex.unlock();
		if (pmsg != NULL) {
			if (pmsg->msgid == EDM_EXIT) {
				mCtx.exit_flag = 1;
#if USE_LIBEVENT
				if (mCtx.mode == MODE_LIBEVENT)
				{
					event_base_loopexit(mCtx.eventBase, NULL);
				}
#endif
				dbgd("EDM_EXIT message .... event loop will be exited...., ptask=%p", this);
			}
//			else if (pmsg->msgid == EDM_VIEW) {
//				ViewMsgCont *msgcont = (ViewMsgCont*) pmsg->obj;
//				auto itr = mViewMap.find(msgcont->view_handle);
//				if (itr != mViewMap.end()) {
////					itr->second()
//					//TODO
//				}
//			}
			else {
				if (pmsg->taskque_handle == 0) {
					OnEventProc(*pmsg);
				}
				else {
					ViewInfo *pinfo;
					_gviewMutex.lock();
					auto itr = gViewMap.find(pmsg->taskque_handle);
					if (itr != gViewMap.end()) {
						pinfo = &itr->second;
					} else {
						pinfo = nullptr;
					}
					_gviewMutex.unlock();
					if(pinfo) pinfo->lis(*pmsg);
				}
			}
			if (pmsg->sync) {
				dbgv("== send msg recv....id=%d", pmsg->msgid);
#if USE_STL_THREAD
				unique_lock<mutex> lck(*pmsg->pmtx);
				pmsg->pcv->notify_one();
				lck.unlock();
#else
				pthread_mutex_lock(pmsg->pmsg_sync_mutex);
				pthread_cond_signal(pmsg->pmsg_sig);
				pthread_mutex_unlock(pmsg->pmsg_sync_mutex);
#endif
				dbgv("== send msg recv end....id=%d", pmsg->msgid);
			}

			mMsgMutex.lock();
			mEmptyMsgs.push_back(pmsg);
			mMsgMutex.unlock();

		}
		else {
			dbge("### unexpected msg que error...");
			break;
		}

	}
}

void EdTask::msgevent_cb(edevt_t* pevt, int fd, int events) {
	uint64_t cnt;
	EdTask* ptask = (EdTask*) pevt->user;
	int rcnt = read(fd, &cnt, sizeof(cnt));
	dbgv("eventfd cnt=%d", cnt);
	if (rcnt == 8) {
		ptask->dispatchMsgs(cnt);
	}
}

void EdTask::postExit(void) {
	postMsg(EDM_EXIT);
}

void EdTask::callMsgClose() {
	EdMsg msg;
	msg.msgid = EDM_CLOSE;
	OnEventProc(msg);
	freeReservedObjs();
}

#if USE_LIBEVENT
void EdTask::libeventMsg_cb(evutil_socket_t fd, short shortInt, void*user)
{
	dbgv("libevent callback, fd=%d", fd);
	EdTask* ptask = (EdTask*) user;
	u64 cnt;
	int rcnt = read(ptask->mMsgFd, &cnt, sizeof(cnt));
	if(rcnt == 8)
	{
		ptask->dispatchMsgs(cnt);
	}
}
#endif

EdTask::TaskTimer::TaskTimer(u32 id) {
	timerId = id;
}

void EdTask::TaskTimer::OnTimer() {
	EdTask* ptask = (EdTask*) getUser();
	EdMsg msg;
	msg.msgid = EDM_TIMER;
	msg.p1 = timerId;
	ptask->OnEventProc(msg);
}

int EdTask::esOpen(void* user) {
	memset(&mCtx, 0, sizeof(EdContext));
	mCtx.mode = mRunMode;

	if (mCtx.mode == MODE_EDEV) {
		mCtx.epfd = epoll_create(10);
		if (!mCtx.epfd) {
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

edevt_t* EdTask::regEdEvent(int fd, uint32_t events, EVENTCB cb, void* user) {

	struct epoll_event event;
	dbgd("esEventReg, fd=%d", fd);
	if (cb == NULL)
		assert(0);

	edevt_t *pevt = allocEvent(&mCtx);

	pevt->fd = fd;
	pevt->evtcb = cb;
	pevt->user = user;

	event.data.u64 = 0; // for clearing valgrind notice('uninitialized value access waring') for 32bit enviroment 
	event.data.ptr = pevt;
	event.events = events;

	mCtx.evt_count++;

	dbgd("    evt=%p, esevent=%p, events=%0x", pevt, pevt->user, events);

	int ret = epoll_ctl(mCtx.epfd, EPOLL_CTL_ADD, fd, &event);
	if (ret) {
		dbge("### epoll_ctl error...ret=%d", ret);
		freeEvent(pevt);
		return NULL;
	}

	pevt->isReg = true;
	mEvtList.push_back(pevt);

	dbgv("== reg event, fd=%d, event_obj=%p, event=%d, count=%d, ret=%d ", fd, pevt, events, mEvtList.size(), ret);

	return pevt;
}

edevt_t* EdTask::allocEvent(EdContext* psys) {
	edevt_t *pevt = mEvtList.allocObj();
	if (pevt) {
		memset(pevt, 0, sizeof(edevt_t));
		pevt->pEdCtx = psys;
		psys->evt_alloc_cnt++;
	}

	return pevt;
}

void EdTask::freeEvent(edevt_t* pevt) {
	pevt->pEdCtx->evt_alloc_cnt--;
	mEvtList.freeObj(pevt);
}

int EdTask::esMain(EdContext* psys) {
	int i;
	int nfds;
	struct epoll_event events[MAX_GET_EVENTS];

	dbgv("es main......");
	for (; psys->exit_flag == 0;) {
		nfds = epoll_wait(psys->epfd, events, MAX_GET_EVENTS, -1);
		dbgv("nfds = %d ", nfds);
		struct epoll_event *epv;
		for (i = 0; i < nfds; i++) {
			epv = events + i;
			edevt_t* pevt = (edevt_t*) epv->data.ptr;
			assert(pevt);
			dbgd("event data.ptr=%p, pevtuser=%p, event=%0x, obj_fd=%d", pevt, pevt->user, epv->events, pevt->fd);
			// test
			if(pevt->fd==0) {
#if 0
				void *buffer[100];
				int j, nptrs;
				nptrs = backtrace(buffer, 100);
				char **strings = backtrace_symbols(buffer, nptrs);
				strings = backtrace_symbols(buffer, nptrs);
				if (strings == NULL) {
					perror("backtrace_symbols");
					exit(EXIT_FAILURE);
				}

				for (j = 0; j < nptrs; j++)
					printf("%s\n", strings[j]);

				free(strings);
#endif
				dbge("#### error............");
				assert(0);
			}
			if (pevt->isReg == false)
				goto __release_event__;
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
				;
			// check if event is dereg.
//			cleanUpEventResource(); // TODO:
		}
		cleanUpEventResource(); // TODO: move to here
		// TODO: confirm remote this call
		//freeReservedObjs();
	}
	psys->opened = 0;
	usleep(1);

	return 0;
}

void EdTask::taskProc() {
	dbgd("start thread main, run mode=%d", mRunMode);
	_tEdTask = this;
	EdContext* pctx = &mCtx;
	_tEdContext = pctx;
	esOpen(this);
	if (mRunMode == MODE_EDEV) {
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

void EdTask::edEventLoop(EdContext* pctx) {
	mEdMsgEvt = regEdEvent(mMsgFd, EVT_READ, msgevent_cb, this);
	// main event loop
	esMain(pctx);
	callMsgClose();

	deregEdEvent(mEdMsgEvt);
	cleanUpEventResource();
	if (mEvtList.size() > 0) {
		dbge("##### There are still active events..count=%d, Missing closing events???", mEvtList.size());
		for (edevt_t* pevt = mEvtList.pop_front(); pevt;) {
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
	mLibMsgEvent = event_new(pctx->eventBase, mMsgFd, EV_READ | EV_PERSIST, libeventMsg_cb, (void*) this);
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

void EdTask::threadMain() {
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

	if (mEvtList.size() > 0) {
		dbge("##### There are still active events..count=%d, Missing closing events???", mEvtList.size());
		for (edevt_t* pevt = mEvtList.pop_front(); pevt;) {
			dbge("  missed fd: %d", pevt->fd);
			pevt = mEvtList.pop_front();
		}
		assert(0);
	}

	esClose(pctx);

	_tEdContext = NULL;
}

void EdTask::deregEdEvent(edevt_t* pevt) {
	EdContext* pctx = &mCtx;
	dbgd("esEventDereg fd=%d", pevt->fd);

	auto ret = epoll_ctl(mCtx.epfd, EPOLL_CTL_DEL, pevt->fd, nullptr);
	assert(ret==0);
	pctx->evt_count--;
	dbgd("== dereg event, event_obj=%p, count=%d, ptask=%p, ret=%d, errno=%d", pevt, pctx->evt_count, this, ret, errno);
	pevt->isReg = false;
	mEvtList.remove(pevt);

	mDummyEvtList.push_back(pevt);

}

void EdTask::esClose(EdContext* pctx) {
	if (pctx->mode == MODE_EDEV) {
		if (pctx->epfd > 0)
			close(pctx->epfd);
		pctx->epfd = -1;
	}

	// check free resource
	if (pctx->evt_alloc_cnt > 0) {
		dbge("### Error: event leak....");
		//assert(0);
	}

	if (pctx->timer_alloc_cnt > 0) {
		dbge("### Error: timer leak....");
		assert(0);
	}

	pctx->opened = 0;
}

int EdTask::changeEdEvent(edevt_t* pevt, uint32_t event) {
	int ret;
	struct epoll_event ev;
	ev.events = event;
	ev.data.ptr = pevt;
	ret = epoll_ctl(pevt->pEdCtx->epfd, EPOLL_CTL_MOD, pevt->fd, &ev);
	dbgv("== change event, fd=%d, cb=%p, user=%p, event=%0x, ret=%d", pevt->fd, pevt->evtcb, pevt->user, event, ret);
	return ret;
}

EdMsg* EdTask::allocMsgObj() {
	EdMsg *pmsg;
	pmsg = mEmptyMsgs.pop_front();
	if (pmsg == nullptr) {
		if (mEmptyMsgs.size() + mQuedMsgs.size() < mMaxMsqQueSize) {
			pmsg = mEmptyMsgs.allocObj();
		}
	}
	return pmsg;
}

void EdTask::setSendMsgResult(EdMsg& msg, int code) {
	if (msg.sync)
		*msg.psend_result = code;
}

void EdTask::cleanUpEventResource() {
	edevt_t* pevt;
	for (pevt = mDummyEvtList.pop_front(); pevt;) {
		dbgd("free event resource=%p", pevt);
		// test
		pevt->fd = 0;
		freeEvent(pevt);
		pevt = mDummyEvtList.pop_front();
	}
}

void EdTask::reserveFree(EdObject* obj) {
	dbgd("reserve free obj=%x", obj);
	if (obj->mIsFree == false) {
		obj->mIsFree = true;
		mReserveFreeList.push_back(obj);
#if USE_LIBEVENT
		if (mRunMode == MODE_LIBEVENT)
		{
			mFreeEvent->raise();
		}
#endif
	}
}

void EdTask::freeReservedObjs() {
	EdObject* obj;
	do {
		dbgd("  remain free=%d", mReserveFreeList.size());
		if (mReserveFreeList.empty() == false) {
			obj = mReserveFreeList.front();
			mReserveFreeList.pop_front();
			delete obj;
			dbgd("## delete p=%x", obj);
		}
		else {
			break;
		}
	} while (true);
}

#if USE_LIBEVENT
void EdTask::FreeEvent::OnEventFd(int cnt)
{
	dbgd("OnEventFd: cleanup reserved free objs, task=%x", getCurrentTask());
	getCurrentTask()->freeReservedObjs();
}
#endif

EdTask* EdTask::getCurrentTask() {
	return _tEdTask;
}

int EdTask::getRunMode() {
	return mRunMode;
}

void EdTask::setOnListener(function<int(EdMsg&)> lis) {
	mLis = lis;
}

u32 EdTask::createTaskMsgQue(TaskEventListener lis) {
	static uint32_t seed_handle = 0;
	lock_guard<mutex> lck(_gviewMutex);
	if (++seed_handle == 0)
		++seed_handle;
	auto &view = gViewMap[seed_handle];
	view.pTask = EdTask::getCurrentTask();
	view.lis = lis;
	view.handle = seed_handle;
	return seed_handle;
}

void EdTask::destroyTaskMsgQue(uint32_t handle) {
	lock_guard<mutex> lck(_gviewMutex);
	gViewMap.erase(handle);
}

int EdTask::postTaskMsgQue(uint32_t handle, u16 msgid, u32 p1, u32 p2) {
	EdMsg tmsg;
	tmsg.p1 = p1, tmsg.p2 = p2;
	return postEdMsg(handle, msgid, tmsg, 0);
}

int EdTask::sendTaskMsgQue(u32 handle, u16 msgid, u32 p1, u32 p2) {
	EdMsg tmsg;
	tmsg.p1 = p1, tmsg.p2 = p2;
	return sendEdMsg(handle, msgid, tmsg, 0);
}

int EdTask::postTaskMsgObj(u32 handle, u16 msgid, void* obj) {
	EdMsg tmsg;
	tmsg.obj = obj;
	return postEdMsg(handle, msgid, tmsg, 0);
}

int EdTask::sendTaskMsgObj(u32 handle, u16 msgid, void* obj) {
	EdMsg tmsg;
	tmsg.obj = obj;
	return sendEdMsg(handle, msgid, tmsg, 0);
}

} // namespace edft

