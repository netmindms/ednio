//============================================================================
// Name        : testednio.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Test Driven Development for ednio
//============================================================================

#define DBGTAG "main0"
#define DBG_LEVEL DBG_DEBUG

#include <sys/types.h>
#include <dirent.h>
#include <vector>

#include "EdType.h"
#include "EdNio.h"
#include "EdTask.h"
#include "EdTimer.h"
#include "EdTime.h"
#include "edcurl/EdCurl.h"
#include "edcurl/EdMultiCurl.h"
#include "http/EdHttpWriter.h"
#include "http/EdHttpTask.h"
#include "http/EdHttpStringReader.h"
#include "http/EdHttpStringWriter.h"
#include "http/EdHttpServer.h"
#include "http/EdHttpTask.h"
#include "http/EdHttpFileReader.h"
#include "http/EdHttpFileWriter.h"
#include "http/EdHdrDate.h"

#include "edssl/EdSSLContext.h"
#include "edssl/EdSSLSocket.h"
#include "edssl/EdSmartSocket.h"

void levlog(int lev, const char *tagid, int line, const char *fmtstr, ...)
{
	struct timeval tm;
	va_list ap;

	gettimeofday(&tm, NULL);
	struct tm* ptr_time = localtime(&tm.tv_sec);

	char buf[4096];

	int splen = 2 * lev;
	char spbuf[splen + 1];
	memset(spbuf, ' ', splen);
	spbuf[splen] = 0;

	va_start(ap, fmtstr);
	vsnprintf(buf, 4096 - 1, fmtstr, ap);
	va_end(ap);

	printf("%02d:%02d:%02d.%02d [%s]:%-5d %s%s\n", ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec, (int) (tm.tv_usec / 10000), tagid, line, spbuf, buf);
}

#define logm(...) {levlog(0, "MTEST", __LINE__, __VA_ARGS__); }
#define logs(...) {levlog(1, "SUB  ", __LINE__, __VA_ARGS__); }
#define logss(...) { levlog(2, "SUB2 ", __LINE__, __VA_ARGS__);  }

using namespace std;
using namespace edft;

long _gStartFds;

int get_num_fds()
{
	int fd_count;
	char buf[300];
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
		logm("### Fail: fd count check error, start=%ld, end=%ld", _gStartFds, fdn);
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

// test task
void testtask(int mode)
{
	enum
	{
		TS_NORMAL = EDM_USER + 1,
	};
	class MainTask: public TestTask
	{
		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_NORMAL);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal test...");
				logs("== Normal test ok...\n");
				nextTest();
			}
			return 0;
		}
	};

	logm(">>>> Test: Task, mode=%d", mode);
	fdcheck_start();
	auto task = new MainTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Task test OK\n");
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
				logs("child init");
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("child close task");
			}
			else
			{
				logs("child received msg. id=%d, p1=%x, p2=%x", pmsg->msgid, pmsg->p1, pmsg->p2);
			}
			return 0;
		}
	};

	class MsgTestTask: public TestTask
	{
		enum
		{
			TS_BASIC_MSG = EDM_USER + 1, TS_OBJECT_MSG, TS_BETWEEN_TASK,

			BASIC_MSG = 10000, OBJECT_MSG, CHILD_TASK_END_MSG,
		};
		MsgChildTestTask* pchild;

		EdTimer *mTimer;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_BASIC_MSG);
				addTest(TS_OBJECT_MSG);
				addTest(TS_BETWEEN_TASK);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("msg task closing...");
			}
			else if (pmsg->msgid == TS_BASIC_MSG)
			{
				logs("== Start message parameter check...");
				postMsg(BASIC_MSG, 0xfacdab58, 0x90bc23aa);
			}
			else if (pmsg->msgid == BASIC_MSG)
			{
				if (pmsg->p1 != 0xfacdab58 || pmsg->p2 != 0x90bc23aa)
				{
					logs("### Fail: mesasge parameter check fail");
					assert(0);
				}
				logs("== End message parameter check...OK");
				nextTest();
			}
			else if (pmsg->msgid == TS_OBJECT_MSG)
			{
				logs("== Start object message check...");
				mTimer = new EdTimer;
				postObj(OBJECT_MSG, (void*) mTimer);
			}
			else if (pmsg->msgid == OBJECT_MSG)
			{
				if (pmsg->obj != (void*) mTimer)
				{
					logs("### Fail: object message check error");
					assert(0);
				}
				delete mTimer;
				logs("== End object message check...");
				nextTest();
			}
			else if (pmsg->msgid == TS_BETWEEN_TASK)
			{
				logs("== Start mesage test between tasks...");
				pchild = new MsgChildTestTask;
				logs("start child task...");
				pchild->run();
				postMsg(CHILD_TASK_END_MSG);
				pchild->postMsg(EDM_USER + 1);

			}
			else if (pmsg->msgid == CHILD_TASK_END_MSG)
			{
				logs("post terminating child task");
				pchild->postExit();
				usleep(10000);
				pchild->sendMsg(EDM_USER + 2);
				logs("waiting child task");
				pchild->wait();
				delete pchild;
				nextTest();
			}
			else
			{
				logs("### invalid test scenario");
				assert(0);
			}
			return 0;
		}
	};

	fdcheck_start();
	logm(">>>> Test: Message, mode=%d", mode);
	MsgTestTask msgtask;
	msgtask.run(mode);
	msgtask.wait();
	logm("<<<< Message Test OK\n");
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
				logs("  timer test init");
				addTest(TS_NORMAL_TIMER);
				addTest(TS_USEC_TIMER);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("  timer test closing");
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
				logs("== start usec timer test, period=1 sec, interval=%d usec", USEC_TIMER_INTERVAL);
				mTimerUsec->setUsec(USEC_TIMER_INTERVAL);
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				if (pmsg->p1 == 1)
				{

				}

			}
			return 0;
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{

			if (ptimer == mTimer)
			{
				mExpCnt++;
				if (mExpCnt == 1000 / NORMAL_TIMER_INTERVAL)
				{
					int dt = EdTime::msecTime() - period;
					logs("    task timer expire, duration=%d", dt);
					if (dt > 1000 + 10)
					{
						logs("### Fail: timer delayed, duration=%d", dt);
						assert(0);
					}

					mTimer->kill();
					logs(" normal timer test OK, duration=%d", dt);
					delete mTimer;
					mTimer = NULL;
					nextTest();
				}
				else if (mExpCnt > 1000 / NORMAL_TIMER_INTERVAL)
				{
					logs("### Fail : timer expire count error, expect=%d, real=%d", 1000/NORMAL_TIMER_INTERVAL, mExpCnt);
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
						logs("### Fail: usec timer time over!!!, period=%d", t - usec_starttime);
						assert(0);
					}
					logs("usec timer test OK, durationt=%d msec, hit=%d, usec_count=%d", t - usec_starttime, mHitCount, mUsecCnt);
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

	logm(">>>> Test: Timer, mode=%d", mode);
	fdcheck_start();
	auto task = new TimerTest;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Timer test OK\n");
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
				logs("%d: ### Fail: status error, status=%d", curlid, status);
				assert(0);
			}
			logs("%d: status check ok", curlid);

			long t = getContentLength();
			if (t != recvLen)
			{
				logs("%d: recv data len check error...., recvlen=%ld, content-len=%ld", recvLen, t);
				assert(0);
			}
			logs("%d: body len check Ok", curlid);
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
				logs("curl test closed...");
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				killTimer(pmsg->p1);
				postExit();
			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal curl test.........");
				mRecvDataSize = 0;
				mLocalCurl = new EdCurl;
				mLocalCurl->setOnCurlListener(this, this);
				mLocalCurl->open(mMainCurl);
				mLocalCurl->request("http://localhost");

			}
			else if (pmsg->msgid == TS_NOTFOUND)
			{
				logs("== Start notfound curl test.........");
				mCurlNotFound = new EdCurl;
				mCurlNotFound->setOnCurlListener(this);
				mCurlNotFound->setUser((void*) "[curl-notfound]");
				mCurlNotFound->open(mMainCurl);
				mCurlNotFound->request("http://localhost/asdfasdfas");

			}
			else if (pmsg->msgid == TS_TIMEOUT)
			{
				logs("== Start timeout curl test.........");
				mAbnormalCurl.setOnCurlListener(this);
				mAbnormalCurl.setUser((void*) "[curl-notconnected]");
				mAbnormalCurl.open(mMainCurl);
				mAbnormalCurl.request("211.211.211.211", CONNECT_TIMEOUT);
				logs("request abnormal curl=%lx", &mAbnormalCurl);
				logs("this is not going to be connected...");

			}
			else if (pmsg->msgid == TS_REUSE)
			{
				logs("== Start reuse curl test....");
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
				logs("== Start load test, try count=%d", cnn);
				int i;
				for (i = 0; i < cnn; i++)
				{
					mLoadCurl[i] = new LoadCurl(this);
					mLoadCurl[i]->setOnCurlListener(this, this);
					mLoadCurl[i]->open(mMainCurl);
					mLoadCurl[i]->curlid = i;
					mLoadCurl[i]->request("http://localhost");
				}
				logs("try %d request...", cnn);
			}
			else if (pmsg->msgid == LOAD_RESULT)
			{
				mLoadEndCnt++;
				if (mLoadEndCnt == LOAD_COUNT)
				{
					logs("all curl end...");
					logs("== End load test");
					nextTest();
				}
			}
			return 0;
		}

		virtual void IOnCurlResult(EdCurl* pcurl, int status)
		{

			logs("curl status = %d, curl=%x", status, pcurl);
			if (pcurl == mLocalCurl)
			{
				if (status != 0)
				{
					logs("### Fail: This curl is expected with normal status code but error status");
					assert(status == 0);
				}
				long len = pcurl->getContentLength();
				if (len != mRecvDataSize)
				{
					logs("### Fail: content length not match");
					assert(0);
				}
				logs("Content length check OK..., len=%ld", mRecvDataSize);

				logs("[curl-normal] : Expected Result. OK.......");
				pcurl->close();
				delete mLocalCurl;
				mLocalCurl = NULL;
				logs("== End normal curl test...\n\n");

				nextTest();
			}
			else if (pcurl == mCurlNotFound)
			{
				logs("%s result: %d", (char* )pcurl->getUser(), pcurl->getResponseCode());
				int code = pcurl->getResponseCode();
				if (code != 404)
				{
					logs("### Fail: 404 response expected, buf code = %d", code);
					assert(0);
				}
				logs("    Not Found Test OK.");
				pcurl->close();
				delete pcurl;
				mCurlNotFound = NULL;
				logs("== End notfound curl test...\n\n");

				nextTest();
			}
			else if (pcurl == &mAbnormalCurl)
			{
				logs("abnormal curl status reported...code=%d", status);
				if (status == 0)
				{
					logs("    ### Fail : For this curl, status must be abnormal...");
					assert(status != 0);
				}
				logs("    %s : %s", (char* )pcurl->getUser(), "Expected Result. OK");
				pcurl->close();
				logs("== End time out curl test...\n\n");

				nextTest();
			}
			else if (pcurl == mReuseCurl)
			{
				if (status != 0)
				{
					logs("### Reuse Test Fail: curl response is not normal. status=%d", status);
					assert(0);
				}

				mReuseCnt++;
				if (mReuseCnt < 5)
				{
					pcurl->reset();
					logs("start new requesting by reusing current curl..., cnt=%d", mReuseCnt);
					pcurl->request("http://localhost");
				}
				else
				{
					logs("== End reuse curl test....\n\n");
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
				 logs("body size = %d", size);
				 memcpy(buf, ptr, size);
				 buf[size] = 0;
				 logs("    body: \n%s", buf);
				 free(buf);
				 */
			}
		}
	};

	logm(">>>> Test: Curl, mode=%d", mode);
	fdcheck_start();
	auto task = new CurlTest;
	task->run();
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Curl Test OK\n");
}

void testreservefree(int mode)
{
	enum
	{
		TS_SIMPLE_FREE = EDM_USER + 1, TS_SELF_FREE,
	};
	class ReserveFreeTest: public TestTask, public EdEventFd::IEventFd, public EdTimer::ITimerCb
	{
		// simple free test
		EdEventFd* mEv;
		EdTimer* mTimer;
		u64 mEventCnt;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_SIMPLE_FREE);
				addTest(TS_SELF_FREE);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("Reserve free test closed...");
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
			else if (pmsg->msgid == TS_SELF_FREE)
			{
				class ti: public EdTimer::ITimerCb
				{
					virtual void IOnTimerEvent(EdTimer* ptimer)
					{
						logs("self free test timer on");
						ptimer->kill();
						getCurrentTask()->reserveFree(ptimer);
						//delete ptimer;
						logs("free reserved for self free timer");
						((ReserveFreeTest*) getCurrentTask())->nextTest();
						delete this;
					}
				};
				logs("== Start self free object test...");
				EdTimer* mt = new EdTimer;
				mt->setOnListener(new ti);
				mt->set(100);
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
				logs("timer on, raise cnt=%ld", mEventCnt);
				ptimer->kill();
				mEv->close();
				reserveFree(ptimer);
				reserveFree(mEv);
				logs("reserved free objs...");
				nextTest();
			}
		}

	};

	logm(">>>> Test: ReserveFree, mode=%d", mode);
	fdcheck_start();
	auto task = new ReserveFreeTest;
	task->run();
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Reserve Free Test OK\n\n");
}

void testMainThreadTask(int mode)
{
	class MainThreadTask: public EdTask
	{
		virtual int OnEventProc(EdMsg *pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("task init event");
				postMsg(EDM_USER);
				postExit();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("task close event");
			}
			else if (pmsg->msgid == EDM_USER)
			{
				logs("user event msg");
			}
			return 0;
		}
	};

	logm("== Start main thread task test...");
	fdcheck_start();
	MainThreadTask task;
	task.runMain(mode);
	fdcheck_end();
	logm("== End main thread task test...\n\n");
}

void testMultiTaskInstance(int mode)
{
#define TASK_INSTANCE_NUM 10
	enum
	{
		UM_TASK_START = EDM_USER + 1,
	};
	class MultiTestTask: public TestTask
	{

	public:
		long hitCount;

		u64 getHitCount()
		{
			return hitCount;
		}
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				hitCount = 0;
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == UM_TASK_START)
			{
				class efd: public EdEventFd, public EdTimer::ITimerCb
				{
					u64 mOnCnt;
					EdTimer timer;
					int mId;
					u32 mStartTime;
					MultiTestTask* mTask;
				public:
					efd(MultiTestTask* ptask)
					{
						mOnCnt = 0;
						mId = -1;
						mTask = ptask;
					}
					void start(int id)
					{
						logs("== Start task, id=%d", id);
						mId = id;
						timer.setOnListener(this);
						timer.set(1000);
						open();
						raise();
						mStartTime = EdTime::msecTime();
					}
					void OnEventFd(int cnt)
					{
						mOnCnt++;
						mTask->hitCount += cnt;
						raise();
					}

					void IOnTimerEvent(EdTimer* ptimer)
					{
						u32 t = EdTime::msecTime();
						logs("[%2d] : end-time=%d, on-count=%ld, hit-count=%ld", mId, t - mStartTime, mOnCnt, mTask->hitCount);
						ptimer->kill();
						close();
						logs("end.....");
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

	logm(">>>> Test: Multi task instance, mode=%d", mode);
	fdcheck_start();
	MultiTestTask task[TASK_INSTANCE_NUM];
	for (int i = 0; i < TASK_INSTANCE_NUM; i++)
	{
		task[i].run();
		task[i].postMsg(UM_TASK_START, i);
	}

	int totalhit = 0;
	for (int i = 0; i < TASK_INSTANCE_NUM; i++)
	{
		task[i].wait();
		totalhit += task[i].getHitCount();
	}
	logm("Total hit count=%ld", totalhit);
	fdcheck_end();
	logm("<<<< Multi task instance OK\n\n");

}

void testHttpBase(int mode)
{
	enum
	{
		TS_STRING_READER = EDM_USER + 1,
	};
	class BaseHttp: public TestTask
	{
		EdHttpStringReader mReader;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{

				addTest(TS_STRING_READER);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_STRING_READER)
			{
				long rdcnt;
				char data[] = "0123456789";
				char buf[100];
				long count = 0;
				mReader.setString(data);
				rdcnt = 0;
				count = 0;

				rdcnt = mReader.Read(buf + count, 3);
				assert(rdcnt == 3);
				count += rdcnt;

				rdcnt = mReader.Read(buf + count, 3);
				assert(rdcnt == 3);
				count += rdcnt;

				rdcnt = mReader.Read(buf + count, 3);
				assert(rdcnt == 3);
				count += rdcnt;

				rdcnt = mReader.Read(buf + count, 3);
				assert(rdcnt == 1);
				count += rdcnt;

				rdcnt = mReader.Read(buf + count, 3);
				assert(rdcnt == -1);

				if (memcmp(data, buf, strlen(data)))
				{
					logs("### Fail: string reader no data match");
					assert(0);
				}

				nextTest();
			}

			return 0;
		}
	};

	logm(">>>> Test: Base Http , mode=%d", mode);
	fdcheck_start();
	BaseHttp task;
	task.runMain(mode);
	fdcheck_end();
	logm("<<<< Base Http OK\n\n");
}

void testHttpSever(int mode)
{
	static bool serverEnd = false;
	static EdHttpServer* server = NULL;

	enum
	{
		TS_NORMAL = EDM_USER + 1, TS_SERVER_END, TS_MULTIPART,
	};
	class MyHttpTask;
	class MyController;
	class FileCtrl;
	class UpFileCtrl;
	class MultipartCtrl;

	class MyHttpTask: public EdHttpTask
	{
		EdHttpStringWriter *mWriter;
		EdHttpStringReader *mReader;
		void *crtmem;
		void *keymem;

	public:

		virtual int OnEventProc(EdMsg* pmsg)
		{
			int ret = EdHttpTask::OnEventProc(pmsg);
			if (pmsg->msgid == EDM_INIT)
			{
				setDefaultCertPassword("ks2662");
				/*
				 EdFile file;
				 file.openFile("/home/netmind/testkey/netsvr.crt");
				 int csize = file.getSize("/home/netmind/testkey/netsvr.crt");
				 crtmem = malloc(csize);
				 file.readFile(crtmem, csize);
				 file.closeFile();

				 file.openFile("/home/netmind/testkey/netsvr.key");
				 int ksize = file.getSize("/home/netmind/testkey/netsvr.key");
				 keymem = malloc(ksize);
				 file.readFile(keymem, ksize);
				 file.closeFile();
				 setDefaultCertMem(crtmem, csize, keymem, ksize);
				 */

				//setDefaultCertFile("/home/netmind/testkey/server.crt", "/home/netmind/testkey/server.key");
				setDefaultCertFile("/home/netmind/testkey/netsvr.crt", "/home/netmind/testkey/netsvr.key");

				regController<MyController>("/userinfo", NULL);
				regController<FileCtrl>("/getfile", NULL);
				regController<UpFileCtrl>("/upfile", NULL);
				regController<MultipartCtrl>("/multi", NULL);
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				CHECK_FREE_MEM(crtmem);
				CHECK_FREE_MEM(keymem);
			}
			return ret;
		}

	};

	class MyController: public EdHttpController, public EdTimer::ITimerCb
	{
		EdHttpStringReader *mStrReader;
		EdTimer mTimer;
		MyHttpTask *mMyTask;
	public:
		MyController()
		{
			mStrReader = NULL;
			mMyTask = (MyHttpTask*) EdTask::getCurrentTask();
			logs("mycont const.....");
		}
		virtual void OnRequest()
		{
			logs("after 100msec, send response...");
			mTimer.setOnListener(this);
			mTimer.set(100);
		}

		virtual void OnContentRecvComplete()
		{
			;
		}
		;

		virtual void OnComplete(int result)
		{
			logs("http complete...result=%d", result);
			delete mStrReader;
			mStrReader = NULL;
			serverEnd = true;
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{
			logs("send response,...");
			ptimer->kill();
			mStrReader = new EdHttpStringReader;
			mStrReader->setString("Hello, ednio http service....\n");
			setRespBodyReader(mStrReader, "text/plain");
			setHttpResult("200");
		}

	};

	class FileCtrl: public EdHttpController
	{
		EdHttpFileReader reader;
		void OnInit()
		{
			logs("file ctrl on init...");
		}
		;
		virtual void OnRequest()
		{
			reader.open("/home/netmind/bb");
			setRespBodyReader(&reader, "application/zip");
			setHttpResult("200");
		}
		;

		virtual void OnComplete(int result)
		{
			logs("file ctrl complete, result=%d", result);
			reader.close();
		}

	};

	class UpFileCtrl: public EdHttpController
	{
		EdHttpFileWriter *writer;
		void OnInit()
		{
			writer = NULL;
		}

		void OnRequest()
		{
			logs("upfile request,...");
			writer = new EdHttpFileWriter;
			writer->open("/tmp/upfile.dat");
			setReqBodyWriter(writer);
		}

		void OnComplete(int result)
		{
			logs("upfile complete, result=%d", result);
			CHECK_DELETE_OBJ(writer);
		}
	};

	class MultipartCtrl: public EdHttpController
	{
		void OnRequest()
		{
			logs("multipart url requested...");
		}
		void OnComplete(int result)
		{
			logs("multipart curl complete, result=%d", result);
		}
	};

	class HttpTestTask: public TestTask
	{
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				server = new EdHttpServer;

				addTest(TS_NORMAL);
				addTest(TS_SERVER_END);
				nextTest();

			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				server->close();
				delete server;
				server = NULL;
			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				int port = 9090;
				int task_inst = 1;
				logs("server open, port=%d, task-instance=%d", port, task_inst);
				server->open(port);
				server->open(7070, true);
				server->startService<MyHttpTask>(task_inst);

				nextTest();
			}
			else if (pmsg->msgid == TS_SERVER_END)
			{
				setTimer(1, 500);
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				if (pmsg->p1 == 1)
				{
					if (serverEnd == true)
					{
						postExit();
						killTimer(pmsg->p1);
					}
				}
			}
			return 0;
		}
	};
	logm(">>>> Test: Http Server, mode=%d", mode);
	fdcheck_start();
	HttpTestTask *task = new HttpTestTask;
	mode = 1;
	task->runMain(mode);
	fdcheck_end();
	logm("<<<< HttpServer OK\n\n");

}

void testssl(int mode)
{
	enum
	{
		TS_SSL = EDM_USER + 1, TS_SMART_SOCK,
	};
	class SSLTestTask: public TestTask, public EdSSLSocket::ISSLSocketCb, public EdSmartSocket::INet
	{
		EdSSLSocket *ssl;
		int sslReadCnt;

		// smart sock test
		EdSmartSocket* smartSock;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				ssl = NULL;
				smartSock = NULL;

				addTest(TS_SSL);
				//addTest(TS_SMART_SOCK);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

				EdSSLContext::freeDefaultEdSSL();
			}
			else if (pmsg->msgid == TS_SSL)
			{
				logs("== Start basic ssl client test...");
				ssl = new EdSSLSocket;
				sslReadCnt = 0;
				ssl->openSSLClientSock();
				ssl->setOnSSLListener(this);
				ssl->connect("127.0.0.1", 443);
			}
			else if (pmsg->msgid == TS_SMART_SOCK)
			{
				logs("== Start smart sock test...");
				smartSock = new EdSmartSocket;
				smartSock->socketOpen(true);
				smartSock->setOnNetListener(this);
				smartSock->connect("127.0.0.1", 443);
			}
			return 0;
		}

		virtual void IOnSSLSocket(EdSSLSocket *psock, int event)
		{
			if (event == SSL_EVENT_CONNECTED)
			{
				logs("ssl connected...");
				char req[] = "GET / HTTP/1.1\r\n"
						"User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
						"Host: 127.0.0.1\r\n"
						"Accept: */*\r\n\r\n";
				ssl->send(req, strlen(req));

			}
			else if (event == SSL_EVENT_DISCONNECTED)
			{
				logs("ssl disconnected..., cur read cnt=%d", sslReadCnt);
				if (sslReadCnt < 177)
				{
					logs("### Fail: ssl read count mismatch, count=%d", sslReadCnt);
					assert(0);
				}
				ssl->close();

				logs("== basic ssl client test OK...\r\n");
				nextTest();
			}
			else if (event == SSL_EVENT_READ)
			{
				char buf[8 * 1024 + 1];
				int rdcnt = ssl->recv(buf, 8 * 1024);
				if (rdcnt > 0)
				{
					sslReadCnt += rdcnt;
					buf[rdcnt] = 0;
					logs(buf);
					if (sslReadCnt == 462)
					{
						ssl->close();
						reserveFree(ssl);
						ssl = NULL;
						nextTest();
					}
				}
			}

		}

		virtual void IOnNet(EdSmartSocket *psock, int event)
		{
			if (event == NETEV_CONNECTED)
			{
				logs("sm on connected..");
				char req[] = "GET / HTTP/1.1\r\n"
						"User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
						"Host: 127.0.0.1\r\n"
						"Accept: */*\r\n\r\n";
				psock->sendPacket(req, strlen(req));
			}
			else if (event == NETEV_DISCONNECTED)
			{
				logs("sm on disconnected..");
				psock->socketClose();
				delete psock;
				nextTest();
			}
			else if (event == NETEV_READ)
			{
				logs("sm on read");
				char buf[1000];
				int rcnt = psock->recvPacket(buf, 500);
				if (rcnt > 0)
				{
					buf[rcnt] = 0;
					logs(buf);
				}
			}
		}
	};

	logm(">>>> Test: SSL, mode=%d", mode);
	fdcheck_start();
	auto task = new SSLTestTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< SSL test OK\n");
}

void testsmartsock(int mode)
{
	enum
	{
		TS_ECHO = EDM_USER + 1, TS_PENDING, TS_SSL,
	};

	class ClientTask: public TestTask, public EdSmartSocket::INet
	{

		EdSmartSocket* mEchoSock;
		char echostr[100];

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("client start...");
				addTest(TS_ECHO);
				addTest(TS_PENDING);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_ECHO)
			{
				logs("== Start normal test");
				strcpy(echostr, "'echo message'");
				mEchoSock = new EdSmartSocket;
				mEchoSock->setOnNetListener(this);
				mEchoSock->socketOpen();
				mEchoSock->connect("127.0.0.1", 7000);

			}
			else if (pmsg->msgid == TS_PENDING)
			{
				subtestpending();
			}
			else if (pmsg->msgid == TS_SSL)
			{

			}
			return 0;
		}

		void subtestpending()
		{
			static EdSmartSocket* pdSock;

			class echoimpl: public EdSmartSocket::INet
			{
				long writeCnt;
				void IOnNet(EdSmartSocket* psock, int event)
				{
					if (event == NETEV_CONNECTED)
					{
						logs("pending connected...");
						writeCnt = 0;
						char buf[1024];
						int ret;
						for (;;)
						{
							memset(buf, 0, 1024);
							ret = psock->sendPacket(buf, 1024);
							if (ret == SEND_PENDING)
							{
								writeCnt += 1024;
								logs("send pending... cur wrietcnt=%d", writeCnt);
								break;
							}
							else if (ret == SEND_FAIL)
							{
								logs("### Fail: send fail......writeCnt=%d", writeCnt);
								assert(0);
							}
							else if (ret == SEND_OK)
							{
								writeCnt += 1024;
							}
						}

					}
					else if (event == NETEV_DISCONNECTED)
					{
						logs("pending disconnected...");
						delete psock;
						delete this;
					}
					else if (event == NETEV_SENDCOMPLETE)
					{
						logs("pending send complete...write cnt=%d", writeCnt);
						usleep(1000 * 1000);
						psock->close();
						getCurrentTask()->reserveFree(psock);
						delete this;
						((TestTask*) getCurrentTask())->nextTest();
					}
				}
			};
			echoimpl *pif = new echoimpl;
			;
			pdSock = new EdSmartSocket;
			pdSock->setOnNetListener(pif);
			pdSock->socketOpen();
			pdSock->connect("127.0.0.1", 7001);

		}

		virtual void IOnNet(EdSmartSocket* psock, int event)
		{
			if (psock == mEchoSock)
			{
				if (event == NETEV_CONNECTED)
				{
					logs("echo connected...");
					int ret = mEchoSock->sendPacket(echostr, strlen(echostr));
					if (ret != SEND_OK)
					{
						logs("### Fail: send fail...");
						assert(0);
					}
				}
				else if (event == NETEV_DISCONNECTED)
				{
					logs("echo disconnected...");
				}
				else if (event == NETEV_READ)
				{
					int rcnt;
					char buf[100];
					rcnt = psock->recvPacket(buf, 100);
					if (rcnt > 0)
					{
						buf[rcnt] = 0;
						if (!strcmp(echostr, buf))
						{
							logs("echo test ok...\n");
							nextTest();
							psock->socketClose();
							reserveFree(psock);
							mEchoSock = NULL;
						}
						else
						{
							logs("### Fail: echo string mismatch...");
							assert(0);
						}
					}
				}
			}
		}
	};

	class ServerTask: public TestTask, public EdSocket::ISocketCb, public EdSmartSocket::INet
	{
		EdSmartSocket *mChildSock;
		EdSocket* mEchoSvrSock;

		// pending test
		EdSocket* mPendingSvr;
		EdSmartSocket* mPendingSock;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("server start...");
				mChildSock = NULL;

				mEchoSvrSock = new EdSocket;
				mEchoSvrSock->setOnListener(this);
				mEchoSvrSock->listenSock(7000);

				mPendingSvr = new EdSocket;
				mPendingSvr->setOnListener(this);
				mPendingSvr->listenSock(7001);
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				mEchoSvrSock->close();
				delete mEchoSvrSock;

				mPendingSvr->close();
				delete mPendingSvr;

			}
			return 0;
		}

		virtual void IOnSocketEvent(EdSocket *psock, int event)
		{
			if (psock == mEchoSvrSock)
			{
				if (event == SOCK_EVENT_INCOMING_ACCEPT)
				{
					int fd = mEchoSvrSock->accept();
					assert(fd > 0);
					assert(mChildSock == NULL);
					mChildSock = new EdSmartSocket;
					mChildSock->setOnNetListener(this);
					mChildSock->socketOpenChild(fd);
				}
			}
			else if (psock == mPendingSvr)
			{
				int fd = mPendingSvr->accept();
				assert(fd > 0);
				class sif: public EdSmartSocket::INet
				{
					void IOnNet(EdSmartSocket* psock, int event)
					{
						static int bwait = 1;
						static int pendReadCnt = 0;
						if (event == NETEV_DISCONNECTED)
						{
							logs("pending sock disconnected..., pend read cnt=%d", pendReadCnt);

							psock->close();
							delete psock;
							delete this;
						}
						else if (event == NETEV_READ)
						{
							//logs("pending sock on read");
							if (bwait == 1)
							{
								usleep(1000 * 1000);
								bwait = 0;
							}
							char buf[1000];
							int rcnt = psock->recvPacket(buf, 500);
							if (rcnt > 0)
							{
								pendReadCnt += rcnt;
							}

						}
					}
				};
				mPendingSock = new EdSmartSocket;
				mPendingSock->setOnNetListener(new sif);
				mPendingSock->socketOpenChild(fd);
			}
		}

		void IOnNet(EdSmartSocket *psock, int event)
		{

			if (event == NETEV_DISCONNECTED)
			{
				logs("child disconnected...");
				psock->close();
				delete psock;
				mChildSock = NULL;
			}
			else if (event == NETEV_CONNECTED)
			{
				assert(0);
			}
			else if (event == NETEV_READ)
			{
				char buf[100 + 1];
				int rcnt = psock->recvPacket(buf, 100);
				if (rcnt > 0)
				{
					buf[rcnt] = 0;
					logs("server recv: %s", buf);
					int ret = psock->sendPacket(buf, rcnt);
					if (ret != SEND_OK)
					{
						logs("### Fail: echo server send fail...");
						assert(0);
					}
				}
				else
				{
					logs("server read fail...ret=%d", rcnt);
				}
			}
		}

	};

	logm(">>>> Test: smart socket, mode=%d", mode);
	fdcheck_start();
	auto stask = new ServerTask;
	stask->run(mode);

	auto ctask = new ClientTask;
	ctask->run(mode);

	ctask->wait();
	stask->terminate();
	delete ctask;
	delete stask;
	fdcheck_end();
	logm("<<<< smart socket test OK\n");
}

void testreadclose(int mode)
{
	enum
	{
		TS_NORMAL = EDM_USER + 1,
	};
	class MainTask: public TestTask, public EdSocket::ISocketCb
	{
		EdSocket *sock;
		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_NORMAL);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal test...");
				sock = new EdSocket;
				sock->setOnListener(this);
				sock->connect("127.0.0.1", 4040);
			}
			return 0;
		}

		void IOnSocketEvent(EdSocket *psock, int event)
		{
			if (event == SOCK_EVENT_READ)
			{
				logs("sevt read...");
				char buf[200];
				int rcnt = psock->recv(buf, 100);
				logs("    rcnt = %d", rcnt);
			}
			else if (event == SOCK_EVENT_DISCONNECTED)
			{
				logs("sevt disc...");
				psock->close();
				delete psock;
				nextTest();
			}
			else if (event == SOCK_EVENT_CONNECTED)
			{
				logs("sevt conn...");
			}
			else if (event == SOCK_EVENT_WRITE)
			{
				logs("sevt write...");
			}
		}
	};

	logm(">>>> Test: Task, mode=%d", mode);
	fdcheck_start();
	auto task = new MainTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Task test OK\n");
}

#include <string.h>
#include "http/http_parser.h"
//#include "http/multipart_parser.h"
#include "MultipartParser.h"

void testmultipartapi()
{
	class Mp
	{
	public:
		static void onPartBegin(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onPartBegin\n");
		}

		static void onHeaderField(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onHeaderField: (%s)\n", string(buffer + start, end - start).c_str());
		}

		static void onHeaderValue(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onHeaderValue: (%s)\n", string(buffer + start, end - start).c_str());
		}

		static void onPartData(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onPartData: (%s)\n", string(buffer + start, end - start).c_str());
		}

		static void onPartEnd(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onPartEnd\n");
		}

		static void onEnd(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onEnd\n");
		}
	};
	MultipartParser parser;

	parser.onPartBegin = Mp::onPartBegin;
	parser.onHeaderField = Mp::onHeaderField;
	parser.onHeaderValue = Mp::onHeaderValue;
	parser.onPartData = Mp::onPartData;
	parser.onPartEnd = Mp::onPartEnd;
	parser.onEnd = Mp::onEnd;

	static char testmsg[]="--abcd\r\n"
							"content-type: text/plain\r\n"
							"content-disposition: form-data; name=\"field1\"; filename=\"field1\"\r\n"
							"foo-bar: abc\r\n"
							"x: y\r\n\r\n"
							"hello world\r\n\r\n"
							"x\r\n\r\n"
							"--abcd--\r\n";
	int cnt=0;
	int bufsize = strlen(testmsg);
	int feedlen;
	parser.setBoundary("abcd");
	int rdcnt=0;
	for(;;) {
		feedlen = min(bufsize-cnt, 5);
	   cnt = parser.feed(testmsg+rdcnt, feedlen);
	   if(cnt == 0)
		   break;
	   else
		   rdcnt += cnt;
	}
}

int main()
{

#if 0
	char t[100] = "name=kim&addr=2323";
	char* p = t;
	char *tk;
	tk = strsep(&p, "=&");
	tk = strsep(&p, "=&");

	char *urlis[] =
	{	"UF_SCHEMA", "UF_HOST", "UF_PORT", "UF_PATH", "UF_QUERY", "UF_FRAGMENT", "UF_USERINFO"};

	http_parser_url url;
	//char raw[] = "http://www.yahoo.co.kr:8080/index.html?name=kim&sec=100";
	char raw[] = "/index.html?name=kim&sec=100";
	char temp[200];
	strcpy(temp, raw);
	char *host, *path, *para;
	int ur = http_parser_parse_url(temp, strlen(temp), 0, &url);
	for (int i = 0; i < UF_MAX; i++)
	{
		if (url.field_set & (1 << i))
		{
			string f;
			f.assign(raw + url.field_data[i].off, url.field_data[i].len);
			dbgd("url parsing: %s = %s", urlis[i], f.c_str());
			temp[url.field_data[i].off + url.field_data[i].len] = 0;
			if (i == UF_HOST)
			{
				host = temp + url.field_data[i].off;
			}
			else if (i == UF_PATH)
			{
				path = temp + url.field_data[i].off;
			}
			else if (i == UF_QUERY)
			{
				para = temp + url.field_data[i].off;
			}
		}
	}
#endif

	EdNioInit();
	for (int i = 0; i < 1; i++)
	{
		//testreadclose(i);
		testHttpSever(i);
		//testsmartsock(i);
		//testHttpBase(i);
		//testssl(i);
//		testMultiTaskInstance(1);
//		testreservefree(i);
//		testtimer(i);
//		testMainThreadTask(i);
//		testmsg(i);
//		testcurl(i);
	}
	return 0;
}
