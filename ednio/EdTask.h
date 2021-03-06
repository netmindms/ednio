/**
 * @file EdTask.h
 * @author netmind
 *
 */

#ifndef EDTASK_H_
#define EDTASK_H_

#include "ednio_config.h"

#include <functional>
#include <list>
#include <unordered_map>
#include <condition_variable>
#include <thread>
#include <mutex>

#include "EdType.h"
#include "EdContext.h"
#include "EdTimer.h"
#include "EdMutex.h"
#include "EdObjList.h"
#include "EdEventFd.h"
#include "EdMsgObj.h"


#define USE_STL_THREAD 1

namespace edft
{

#if EDNIO_TLS_PTHREAD
extern pthread_key_t gEdTaskTLSKey;
#else
extern __thread class EdTask *_tEdTask;
#endif

typedef struct
{
	u16 msgid;
	union
	{
		struct {
			u32 p1;
			u32 p2;
		};
		void *obj;
		u64 data;
	};
	upEdMsgObj msgobj;
	u32 taskque_handle;
friend class EdTask;
private:
	int sync;
	int *psend_result;
#if USE_STL_THREAD
	//
	std::condition_variable *pcv;
	std::mutex *pmtx;
#else
	pthread_cond_t *pmsg_sig;
	pthread_mutex_t *pmsg_sync_mutex;
#endif

} EdMsg;

#define FETCH_MSGOBJ(T, MSG) unique_ptr<T>((T*)MSG.msgobj.release())


typedef std::function<int (EdMsg& msg)> TaskEventListener;

struct ViewInfo {
	u32 handle;
	EdTask *pTask;
	TaskEventListener lis;
};


/**
 * @author netmind
 * @class EdTask
 * @brief Event Driven Task frame class.
 * @remark EdTask is basically a thread with a event loop.\n
 * It works asynchronously by using non-block io.
 * All codes in EdTask should not be blocked with "best effort".
 * Any code to be blocked will cause delay other events on the same task(thread).
 */
class EdTask
{
friend class EdEvent;
public:
	EdTask();
	EdTask(const EdTask &){}; // copy constructor do nothing
	virtual ~EdTask();

private:

	class TaskTimer : public EdTimer {
		friend class EdTask;
	private:
		TaskTimer(u32 id);
		virtual void OnTimer();
	private:
		u32 timerId;
	};

	int mMaxMsqQueSize;
	std::unordered_map<u32, TaskTimer*> mTimerMap;
	int mRunMode;
	EdMutex mMsgMutex;
	EdObjList<EdMsg> mEmptyMsgs;
	EdObjList<EdMsg> mQuedMsgs;
#if (!USE_STL_THREAD)
	pthread_t mTid;
#endif
	EdContext mCtx;
	int mMsgFd;
	edevt_t *mEdMsgEvt;
	EdObjList<edevt_t> mEvtList;
	EdObjList<edevt_t> mDummyEvtList;
	std::list<EdObject*> mReserveFreeList;
	std::function<int(EdMsg&)> mLis;

#if USE_STL_THREAD
	std::thread mThread;
#endif
	EdMsg* allocMsgObj();


public:
	/**
	 * @brief Run a thread with event loop and wait events.
	 * @remark Then this method is called, EDM_INIT event message is sent and returned after the message is processed by default event procetdure.\n
	 * @param mode If mode is 0, EdTask run with libednio specific event loop. In case of mode 1, EdTask run with libevent compatible mode.
	 * @warning To enable libevent compatible mode, libednio must be built with USE_LIBEVENT definition.
	 * @return 0 success
	 */
	int run(int mode = 0);

	/**
	 * @brief Run a task with event loop in current thread.
	 * @remark This method does not make an additional thread for event loop. Event looping is in calling thread.
	 * @param mode If mode is 0, EdTask run with libednio specific event loop. In case of mode 1, EdTask run with libevent compatible mode.
	 * @warning To enable libevent compatible mode, libednio must be built with USE_LIBEVENT definition.
	 * @return 0 success
	 */
	int runMain(int mode = 0);

	int run(int mode, u32 p1, u32 p2, int msgqnum);
	int runMain(int mode, u32 p1, u32 p2, int msgqnum);


	/**
	 * @brief A task has other task terminated.
	 * @remark This method must be called from task
	 */
	void terminate(void);

	/**
	 * @brief Wait when task is terminated.
	 */
	void wait(void);

	/**
	 * @brief Post exit message to task.
	 * @remark Calling this method has EMD_EXIT sent so that task enters into terminating process.
	 */
	void postExit(void);

	/**
	 * @brief Post a message to this task.
	 * @remark This method add the message to que and is returned immediately.
	 * @param msgid
	 * @param p1
	 * @param p2
	 * @return If fail, return value<0.
	 */
	int postMsg(u16 msgid, u32 p1 = 0, u32 p2 = 0);

	/**
	 * @brief Send a message to this task.
	 * @remark This method add the message to que and wait until the message is processed by task.
	 * @param msgid
	 * @param p1
	 * @param p2
	 * @return If fail, return a value<0 or the value specified by setSendMsgResult().
	 * @warning You must not call this method in the same thread as the task is running. \n
	 * If you do so, calling thread is blocked permanently(that is, dead-locked )
	 */
	int sendMsg(u16 msgid, u32 p1 = 0, u32 p2 = 0);

	/**
	 * @brief Post a message to this task with object pointer.
	 * @remark This method add the message to que and wait until the message is processed by task.
	 * @param msgid
	 * @param obj
	 * @return
	 */
	int postObj(u16 msgid, void *obj);
	int postObj(u16 msgid, upEdMsgObj obj);

	/**
	 * @brief Send a message to this task with object pointer.
	 * @remark This method add the message to que and wait until the message is processed by task.
	 * @param msgid
	 * @param obj
	 * @return If fail, return a value<0 or the value specified by setSendMsgResult().
	 */
	int sendObj(u16 msgid, void *obj);
	int sendObj(u16 msgid, upEdMsgObj obj);

	EdContext* getEdContext(void)
	{
		return &mCtx;
	};

	/**
	 * @brief Set a timer with tmer id specified by user.
	 * @remark Then timer is expired, EDM_TIMER message is activated. timer id is provided by message parameter p1.\n
	 * You should need to specify unique id to distinct each other between some timers.
	 * @param id timer ID
	 * @param msec interval(unit milisec)
	 * @param inittime initial time. If this is zero, first expiration is the same as msec.
	 * @return
	 */
	int setTimer(u32 id, u32 msec, u32 inittime=0);
	/**
	 * @brief Stop a timer by specified timer id
	 * @param id Timer id specified by user.
	 */
	void killTimer(u32 id);
	void cleanupAllTimer();

	int getRunMode();
	void reserveFree(EdObject* obj);
	static EdTask* getCurrentTask();
	int lastSockErrorNo;
	void setOnListener(std::function<int(EdMsg&)> lis);

	// task messag queue
	int postTaskMsgQue(u32 handle, u16 msgid, u32 p1, u32 p2);
	int sendTaskMsgQue(u32 handle, u16 msgid, u32 p1, u32 p2);
	int postTaskMsgObj(u32 handle, u16 msgid, void *obj);
	int sendTaskMsgObj(u32 handle, u16 msgid, void *obj);
	static u32 createTaskMsgQue(TaskEventListener lis);
	static void destroyTaskMsgQue(uint32_t handle);

public:
	virtual int OnEventProc(EdMsg& pmsg);

private:
	edevt_t* regEdEvent(int fd, uint32_t events, EVENTCB cb, void* user);
	void deregEdEvent(edevt_t* pevt);
	int changeEdEvent(edevt_t* pevt, uint32_t event);

private:
	int esOpen(void* user);
	edevt_t* allocEvent(EdContext* psys);
	void freeEvent(edevt_t* pevt);
	int esMain(EdContext* psys);
	void esClose(EdContext* psys);

	void threadMain();
	void taskProc();
	void edEventLoop(EdContext* pctx);
#if USE_LIBEVENT
	void libeventLoop(EdContext* pctx);
#endif

	void initMsg();
	void closeMsg();
	int sendEdMsg(u32 handle, u16 msgid, EdMsg &tmsg, int type);
	int postEdMsg(u32 view_handle, u16 msgid, EdMsg &tmsg, int type);
	void dispatchMsgs(int cnt);
	void callMsgClose();
	void cleanUpEventResource();
	void freeReservedObjs();


protected:
	/**
	 * @brief Set result value related to EdMsg sent by sendMsg.
	 * @param pmsg
	 * @param code
	 */
	void setSendMsgResult(EdMsg& msg, int code);

private:
	static void msgevent_cb(edevt_t* pevt, int fd, int events);
	static void* task_thread(void* arg);

private:
#if USE_LIBEVENT

	class FreeEvent : public EdEventFd {
		virtual void OnEventFd(int cnt);
	};
	FreeEvent *mFreeEvent;

	event *mLibMsgEvent;

	//static void* libevent_thread(void* arg);
	static void libeventMsg_cb(evutil_socket_t, short, void *);
	//void libeventMain(EdContext* pctx);
#endif
};

} // namespace edft

#endif /* EDTASK_H_ */
