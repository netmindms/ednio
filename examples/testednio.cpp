//============================================================================
// Name        : testednio.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#define DBGTAG "main0"
#define DBG_LEVEL DBG_DEBUG

#include <sys/types.h>
#include <dirent.h>

#include "EdNio.h"
#include "EdTask.h"
#include "EdTimer.h"
#include "EdTime.h"
#include "edcurl/EdCurl.h"
#include "edcurl/EdMultiCurl.h"

using namespace std;
using namespace edft;

long _gStartFds;

int get_num_fds()
{
	int fd_count;
	char buf[64];
	struct dirent *dp;

	snprintf(buf, 256, "/proc/%i/fd/", getpid());

	fd_count = 0;
	DIR *dir = opendir(buf);
	while ((dp = readdir(dir)) != NULL)
	{
		fd_count++;
	}
	closedir(dir);
	return fd_count;
}

void fdcheck_start()
{
	_gStartFds = get_num_fds();

}

void fdcheck_end()
{
	long fdn = get_num_fds();
	if (_gStartFds != fdn)
	{
		dbgd("### Fail: fd count check error, start=%ld, end=%ld", _gStartFds, fdn);
		assert(0);
	}
}

class TestTask: public EdTask
{
	std::list<int> mTestList;
public:
	void nextTest()
	{
		if (mTestList.size() > 0)
		{
			int s = mTestList.front();
			mTestList.pop_front();
			postMsg(s);
		}
		else
		{
			postExit();
		}
	}

	void addTest(int t)
	{
		mTestList.push_back(t);
	}
};

void testtask()
{
	class MyEvent: public EdEvent, public EdTimer::ITimerCb
	{
		EdTimer *mtTimer;
	public:
		MyEvent()
		{
			dbgd("const MyEvent");
			mtTimer = new EdTimer;

		}
		virtual ~MyEvent()
		{
			dbgd("dest MyEvent");
			mtTimer->kill();
			EdTask* ct = _tEdTask;
			ct->reserveFree(mtTimer);
		}

		void go()
		{
			mtTimer->setOnListener(this);
			mtTimer->set(1000);
		}

		virtual void IOnTimerEvent(EdTimer *ptimer)
		{
			dbgd("on timer");
		}
	};

	class Task: public EdTask
	{
		MyEvent *ev;
	public:
		int OnEventProc(EdMsg* msg)
		{
			if (msg->msgid == EDM_INIT)
			{
				dbgd("this task=%x", this);
				ev = new MyEvent;
				ev->go();
				setTimer(1, 1000);

			}
			else if (msg->msgid == EDM_CLOSE)
			{
				reserveFree(ev);
			}
			else if (msg->msgid == EDM_TIMER)
			{
				dbgd("on task timer...id=%d", msg->p1);
				killTimer(1);
			}
			return 0;
		}
	};

	auto task = new Task;
	task->run();
	getchar();
	task->terminate();
	delete task;
	dbgd("========== end");
}

// message exchanging test
void testmsg(int mode)
{

	class MsgChildTestTask: public EdTask
	{
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				dbgd("  child init");
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				dbgd("  child close task");
			}
			else
			{
				dbgd("  child received msg. id=%d, p1=%x, p2=%x", pmsg->msgid, pmsg->p1, pmsg->p2);
			}
			return 0;
		}
	};

	class MsgTestTask: public EdTask
	{
		MsgChildTestTask* pchild;
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				pchild = new MsgChildTestTask;
				dbgd("  start child task...");
				pchild->run();
				postExit();
				pchild->postMsg(EDM_USER + 1);
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				dbgd("  msg task closing...");
				pchild->postExit();
				usleep(10000);
				pchild->sendMsg(EDM_USER + 2);
				dbgd("  waiting child task");
				pchild->wait();
				delete pchild;
			}
			return 0;
		}
	};

	fdcheck_start();
	dbgd(">>>> Test: Message, mode=%d", mode);
	MsgTestTask msgtask;
	msgtask.run(mode);
	msgtask.wait();
	dbgd("<<<< Message Test OK\n");
	fdcheck_end();
}

void testtimer(int mode)
{
#define NORMAL_TIMER_INTERVAL 50 // ms
#define USEC_TIMER_INTERVAL 1 // 1 us
	enum
	{
		TS_NORMAL_TIMER = EDM_USER + 1, TS_USEC_TIMER,
	};
	class TimerTest: public TestTask, public EdTimer::ITimerCb
	{
		std::list<int> mTestList;

		EdTimer *mTimer, *mTimerUsec;
		u32 period;
		int mExpCnt;
		int mMsecTargetCnt;
		u64 mUsecCnt, mHitCount;
		u32 usec_starttime;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				dbgd("  timer test init");
				addTest(TS_NORMAL_TIMER);
				addTest(TS_USEC_TIMER);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				dbgd("  timer test closing");
				assert(mTimer == NULL);

			}
			else if (pmsg->msgid == TS_NORMAL_TIMER)
			{
				mExpCnt = 0;

				mTimer = new EdTimer;
				mTimer->setOnListener(this);
				mTimer->set(NORMAL_TIMER_INTERVAL);
				period = EdTime::msecTime();
			}
			else if (pmsg->msgid == TS_USEC_TIMER)
			{
				mUsecCnt = 0;
				mHitCount = 0;
				mTimerUsec = new EdTimer;
				mTimerUsec->setOnListener(this);
				usec_starttime = EdTime::msecTime();
				dbgd("== start usec timer test, period=1 sec, interval=%d usec", USEC_TIMER_INTERVAL);
				mTimerUsec->setUsec(USEC_TIMER_INTERVAL);
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				if (pmsg->p1 == 1)
				{

				}

			}
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{

			if (ptimer == mTimer)
			{
				mExpCnt++;
				if (mExpCnt == 1000 / NORMAL_TIMER_INTERVAL)
				{
					int dt = EdTime::msecTime() - period;
					dbgd("    task timer expire, duration=%d", dt);
					if (dt > 1000 + 10)
					{
						dbgd("### Fail: timer delayed, duration=%d", dt);
						assert(0);
					}

					mTimer->kill();
					dbgd(" normal timer test OK, duration=%d", dt);
					delete mTimer;
					mTimer = NULL;
					nextTest();
				}
				else if (mExpCnt > 1000 / NORMAL_TIMER_INTERVAL)
				{
					dbgd("### Fail : timer expire count error, expect=%d, real=%d", 1000/NORMAL_TIMER_INTERVAL, mExpCnt);
					assert(0);
				}
			}
			else if (ptimer == mTimerUsec)
			{
				mUsecCnt++;
				mHitCount += ptimer->getHitCount();
				u32 targetcnt = 1000000 / USEC_TIMER_INTERVAL;
				if (mHitCount >= targetcnt)
				{
					u32 t = EdTime::msecTime();
					if (t - usec_starttime > 1000 + 10)
					{
						dbgd("### Fail: usec timer time over!!!, period=%d", t - usec_starttime);
						assert(0);
					}
					dbgd("usec timer test OK, durationt=%d msec, hit=%d, usec_count=%d", t - usec_starttime, mHitCount, mUsecCnt);
					mTimerUsec->kill();
					delete mTimerUsec;
					mTimerUsec = NULL;
					nextTest();
				}

			}
			else
			{
				assert(0);
			}
		}
	};

	dbgd(">>>> Test: Timer, mode=%d", mode);
	fdcheck_start();
	auto task = new TimerTest;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	dbgd("<<<< Timer test OK\n");
}

/*
 * Test scenario for Curl
 */
void testcurl(int mode)
{
#define CONNECT_TIMEOUT 5
#define TEST_DURATION (CONNECT_TIMEOUT+1)
#define LOAD_COUNT 100

	enum
	{
		TS_NORMAL = EDM_USER + 1, TS_TIMEOUT, TS_NOTFOUND, TS_REUSE, TS_LOAD, LOAD_RESULT,
	};
	class CurlTest;
	class LoadCurl: public EdCurl
	{
	public:
		int curlid;
		long recvLen;

		EdTask *mTask;
		LoadCurl(EdTask *task)
		{
			curlid = -1;
			mTask = task;
			recvLen = 0;
		}
		virtual void OnCurlEnd(int status)
		{

			mTask->postMsg(LOAD_RESULT, curlid, 0);
			if (status != 0)
			{
				dbgd("%d: ### Fail: status error, status=%d", curlid, status);
				assert(0);
			}
			dbgd("%d: status check ok", curlid);

			long t = getContentLength();
			if (t != recvLen)
			{
				dbgd("%d: recv data len check error...., recvlen=%ld, content-len=%ld", recvLen, t);
				assert(0);
			}
			dbgd("%d: body len check Ok", curlid);
			close();
			delete this;
		}

		virtual void OnCurlBody(void *buf, int len)
		{
			recvLen += len;
		}

	};

	class CurlTest: public EdTask, public EdCurl::ICurlResult, public EdCurl::ICurlBody
	{

		EdMultiCurl *mMainCurl;
		EdCurl *mLocalCurl;
		EdCurl mAbnormalCurl;
		EdCurl *mCurlNotFound;
		EdCurl *mReuseCurl;
		LoadCurl *mLoadCurl[1000];
		int mLoadEndCnt;

		int mReuseCnt;
		long mRecvDataSize;

		std::list<int> mTestList;
		void nextTest()
		{
			if (mTestList.size() > 0)
			{
				int s = mTestList.front();
				mTestList.pop_front();
				postMsg(s);
			}
			else
			{
				postExit();
			}
		}

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				mCurlNotFound = NULL;
				mLocalCurl = NULL;

				mMainCurl = new EdMultiCurl;
				mMainCurl->open();

				mTestList.push_back(TS_NORMAL);
				mTestList.push_back(TS_NOTFOUND);
				mTestList.push_back(TS_REUSE);
				mTestList.push_back(TS_TIMEOUT);
				mTestList.push_back(TS_LOAD);

				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				assert(mLocalCurl == NULL);
				mMainCurl->close();
				delete mMainCurl;
				dbgd("curl test closed...");
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				killTimer(pmsg->p1);
				postExit();
			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				dbgd("== Start normal curl test.........");
				mRecvDataSize = 0;
				mLocalCurl = new EdCurl;
				mLocalCurl->setOnCurlListener(this, this);
				mLocalCurl->open(mMainCurl);
				mLocalCurl->request("http://localhost");

			}
			else if (pmsg->msgid == TS_NOTFOUND)
			{
				dbgd("== Start notfound curl test.........");
				mCurlNotFound = new EdCurl;
				mCurlNotFound->setOnCurlListener(this);
				mCurlNotFound->setUser((void*) "[curl-notfound]");
				mCurlNotFound->open(mMainCurl);
				mCurlNotFound->request("http://localhost/asdfasdfas");

			}
			else if (pmsg->msgid == TS_TIMEOUT)
			{
				dbgd("== Start timeout curl test.........");
				mAbnormalCurl.setOnCurlListener(this);
				mAbnormalCurl.setUser((void*) "[curl-notconnected]");
				mAbnormalCurl.open(mMainCurl);
				mAbnormalCurl.request("211.211.211.211", CONNECT_TIMEOUT);
				dbgd("request abnormal curl=%lx", &mAbnormalCurl);
				dbgd("this is not going to be connected...");

			}
			else if (pmsg->msgid == TS_REUSE)
			{
				dbgd("== Start reuse curl test....");
				mReuseCnt = 0;
				mReuseCurl = new EdCurl;
				mReuseCurl->setOnCurlListener(this);
				mReuseCurl->open(mMainCurl);
				mReuseCurl->request("http://localhost");
			}
			else if (pmsg->msgid == TS_LOAD)
			{
				int cnn = LOAD_COUNT;
				mLoadEndCnt = 0;
				dbgd("== Start load test, try count=%d", cnn);
				int i;
				for (i = 0; i < cnn; i++)
				{
					mLoadCurl[i] = new LoadCurl(this);
					mLoadCurl[i]->setOnCurlListener(this, this);
					mLoadCurl[i]->open(mMainCurl);
					mLoadCurl[i]->curlid = i;
					mLoadCurl[i]->request("http://localhost");
				}
				dbgd("    try %d request...", cnn);
			}
			else if (pmsg->msgid == LOAD_RESULT)
			{
				mLoadEndCnt++;
				if (mLoadEndCnt == LOAD_COUNT)
				{
					dbgd("all curl end...");
					dbgd("== End load test");
					nextTest();
				}
			}
			return 0;
		}

		virtual void IOnCurlResult(EdCurl* pcurl, int status)
		{

			dbgd("curl status = %d, curl=%x", status, pcurl);
			if (pcurl == mLocalCurl)
			{
				if (status != 0)
				{
					dbgd("### Fail: This curl is expected with normal status code but error status");
					assert(status == 0);
				}
				long len = pcurl->getContentLength();
				if (len != mRecvDataSize)
				{
					dbgd("### Fail: content length not match");
					assert(0);
				}
				dbgd("Content length check OK..., len=%ld", mRecvDataSize);

				dbgd("[curl-normal] : Expected Result. OK.......");
				pcurl->close();
				delete mLocalCurl;
				mLocalCurl = NULL;
				dbgd("== End normal curl test...\n\n");

				nextTest();
			}
			else if (pcurl == mCurlNotFound)
			{
				dbgd("%s result: %d", (char* )pcurl->getUser(), pcurl->getResponseCode());
				int code = pcurl->getResponseCode();
				if (code != 404)
				{
					dbgd("### Fail: 404 response expected, buf code = %d", code);
					assert(0);
				}
				dbgd("    Not Found Test OK.");
				pcurl->close();
				delete pcurl;
				mCurlNotFound = NULL;
				dbgd("== End notfound curl test...\n\n");

				nextTest();
			}
			else if (pcurl == &mAbnormalCurl)
			{
				dbgd("abnormal curl status reported...code=%d", status);
				if (status == 0)
				{
					dbgd("    ### Fail : For this curl, status must be abnormal...");
					assert(status != 0);
				}
				dbgd("    %s : %s", (char* )pcurl->getUser(), "Expected Result. OK");
				pcurl->close();
				dbgd("== End time out curl test...\n\n");

				nextTest();
			}
			else if (pcurl == mReuseCurl)
			{
				if (status != 0)
				{
					dbgd("### Reuse Test Fail: curl response is not normal. status=%d", status);
					assert(0);
				}

				mReuseCnt++;
				if (mReuseCnt < 5)
				{
					pcurl->reset();
					dbgd("start new requesting by reusing current curl..., cnt=%d", mReuseCnt);
					pcurl->request("http://localhost");
				}
				else
				{
					dbgd("== End reuse curl test....\n\n");
					mReuseCurl->close();
					delete mReuseCurl;
					mReuseCurl = NULL;
					nextTest();
				}
			}

			else
			{
				assert(0);
			}
		}

		virtual void IOnCurlHeader(EdCurl* pcurl)
		{

		}

		virtual void IOnCurlBody(EdCurl* pcurl, void* ptr, int size)
		{
			if (pcurl == mLocalCurl)
			{
				mRecvDataSize += size;
				/*
				 char* buf = (char*) malloc(size + 1);
				 assert(buf != NULL);
				 dbgd("body size = %d", size);
				 memcpy(buf, ptr, size);
				 buf[size] = 0;
				 dbgd("    body: \n%s", buf);
				 free(buf);
				 */
			}
		}
	};

	dbgd(">>>> Test: Curl, mode=%d", mode);
	fdcheck_start();
	auto task = new CurlTest;
	task->run();
	task->wait();
	delete task;
	fdcheck_end();
	dbgd("<<<< Curl Test OK\n");
}

void testreservefree(int mode)
{
	enum
	{
		TS_SIMPLE_FREE = EDM_USER + 1,
	};
	class ReserveFreeTest: public TestTask, public EdEventFd::IEventFd, public EdTimer::ITimerCb
	{
		// simple free test
		EdEventFd* mEv;
		EdTimer* mTimer;
		std::list<int> mTestList;
		u64 mEventCnt;

		void nextTest()
		{
			if (mTestList.size() > 0)
			{
				int s = mTestList.front();
				mTestList.pop_front();
				postMsg(s);
			}
			else
			{
				postExit();
			}
		}

		virtual int OnEventProc(EdMsg* pmsg)
		{

			if (pmsg->msgid == EDM_INIT)
			{
				mTestList.push_back(TS_SIMPLE_FREE);

				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_SIMPLE_FREE)
			{
				mEventCnt = 0;

				mEv = new EdEventFd;
				mEv->setOnListener(this);
				mEv->open();
				mEv->raise();

				mTimer = new EdTimer;
				mTimer->setOnListener(this);
				mTimer->set(1000);

			}
			return 0;
		}

		virtual void IOnEventFd(EdEventFd *pefd, int cnt)
		{
			if (pefd == mEv)
			{
				mEventCnt++;
				pefd->raise();
			}
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{
			if (ptimer == mTimer)
			{
				dbgd("timer on, raise cnt=%ld", mEventCnt);
				ptimer->kill();
				mEv->close();

				reserveFree(ptimer);
				reserveFree(mEv);
				dbgd("reserved free objs...");
				nextTest();
			}
		}

	};

	dbgd(">>>> Test: ReserveFree, mode=%d", mode);
	fdcheck_start();
	auto task = new ReserveFreeTest;
	task->run();
	task->wait();
	delete task;
	fdcheck_end();
	dbgd("<<<< Reserve Free Test OK");
}


void testMultiTaskInstance(int mode)
{
#define TASK_INSTANCE_NUM 16
	enum {UM_TASK_START=EDM_USER+1, };
	class MultiTestTask: public TestTask
	{

	public:
		int mHitCnt;

		u64 getHitCount() {
			return mHitCnt;
		}
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if(pmsg->msgid == EDM_INIT)
			{

			}
			else if(pmsg->msgid == EDM_CLOSE)
			{

			}
			else if(pmsg->msgid == UM_TASK_START)
			{
				class efd : public EdEventFd, public EdTimer::ITimerCb {
					u64 mOnCnt;
					EdTimer timer;
					int mId;
					u32 mStartTime;
					MultiTestTask* mTask;
				public:
					efd(MultiTestTask* ptask) {
						mOnCnt = 0;
						mHitCnt = 0;
						mId = -1;
						mTask = ptask;
					}
					void start(int id) {
						dbgd("== Start task, id=%d", id);
						mId = id;
						timer.setOnListener(this);
						timer.set(1000);
						open();
						raise();
						mStartTime = EdTime::msecTime();
					}
					void OnEventFd(int cnt) {
						mOnCnt++;
						mTask->mHitCnt += cnt;
						raise();
					}

					void IOnTimerEvent(EdTimer* ptimer) {
						u32 t = EdTime::msecTime();
						dbgd("[%2d] : end-time=%d, on-count=%ld, hit-count=%ld", mId, t-mStartTime, mOnCnt, mTask->mHitCnt);
						ptimer->kill();
						close();
						dbgd("    end.....");
						getCurrentTask()->postExit();
						delete this;

					}
				};
				efd *pfd = new efd(this);
				pfd->start(pmsg->p1);
			}
			return 0;
		}
	};

	dbgd(">>>> Test: Multi task instance, mode=%d", mode);
	fdcheck_start();
	MultiTestTask task[TASK_INSTANCE_NUM];
	for (int i = 0; i < TASK_INSTANCE_NUM; i++)
	{
		task[i].run();
		task[i].postMsg(UM_TASK_START, i);
	}

	int totalhit=0;
	for (int i = 0; i < TASK_INSTANCE_NUM; i++)
	{
		task[i].wait();
		totalhit += task[i].getHitCount();
	}

	fdcheck_end();
	dbgd("<<<< Multi task instance OK");

}

int main()
{

	EdNioInit();
	for (int i = 0; i < 2; i++)
	{
		testMultiTaskInstance(i);

//		testmsg(i);
//		testtimer(i);
//		testcurl(i);
//		testreservefree(i);
	}
	return 0;
}
