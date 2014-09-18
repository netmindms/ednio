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
#include "http/EsHttpServer.h"
#include "http/EsHttpTask.h"
#include "edssl/EdSSL.h"
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
	class MyHttpTask;
	class MyController;
	/*
	 class MyUserDataUrl: public EdHttpController
	 {
	 public:
	 MyUserDataUrl() {
	 logs("MyUserData const.....");
	 }
	 virtual void OnRequest()
	 {

	 };

	 virtual void OnContentRecvComplete() {
	 ;
	 };
	 virtual void OnContentSendComplete() {

	 };
	 virtual void OnComplete() {
	 // TODO controller complete
	 delete this;
	 }
	 };
	 */

	class MyHttpTask: public EsHttpTask
	{
		EdHttpStringWriter *mWriter;
		EdHttpStringReader *mReader;
	public:

		virtual int OnEventProc(EdMsg* pmsg)
		{
			int ret = EsHttpTask::OnEventProc(pmsg);
			if (pmsg->msgid == EDM_INIT)
			{
				regController<MyController>("/userinfo");
				//regController<MyUserDataUrl>("/userdata");
			}
			return ret;
		}
#if 0
		EdHttpController* OnNewRequest(const char* method, const char* url)
		{
			if (!strcmp(method, "GET") && !strcmp(url, "userinfo"))
			{
				return new MyController;
			}
			else
			{
				return NULL;
			}
		}
#endif
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
			mMyTask = (MyHttpTask*) getCurrentTask();
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
		virtual void OnContentSendComplete()
		{
			delete mStrReader;
			mStrReader = NULL;
		}
		;
		virtual void OnComplete()
		{
			// TODO controller complete
			logs("http complete...");
			delete this;
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{
			logs("send response,...");
			ptimer->kill();
			mStrReader = new EdHttpStringReader;
			mStrReader->setString("Hello, ednio http service....");
			setRespBodyReader(mStrReader);
			setHttpResult("200");
		}

	};

	class HttpTestTask: public TestTask
	{
		EsHttpServer mServer;
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				int port = 9090;
				int task_inst = 1;
				logs("server open, port=%d, task-instance=%d", port, task_inst);
				mServer.open(port);
				mServer.addService<MyHttpTask>(task_inst);

			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				mServer.close();
			}
			return 0;
		}
	};
	logm(">>>> Test: Http Server, mode=%d", mode);
	fdcheck_start();
	HttpTestTask task;
	task.runMain(mode);
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

				//addTest(TS_SSL);
				addTest(TS_SMART_SOCK);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

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
	enum { TS_NORMAL=EDM_USER+1, };

	class ClientTask: public TestTask, public EdSmartSocket::INet
	{

		EdSmartSocket* mSock;
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("client start...");
				addTest(TS_NORMAL);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if(pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal test");
				mSock = new EdSmartSocket;
				mSock->setOnNetListener(this);
				mSock->socketOpen();
				mSock->connect("127.0.0.1", 7000);
			}
			return 0;
		}

		virtual void IOnNet(EdSmartSocket* psock, int event) {
			if(event == NETEV_CONNECTED) {
				logs("normal connected...");
			} else if(event == NETEV_DISCONNECTED) {
				logs("normal disconnected...");
			}
		}
	};

	class ServerTask: public TestTask, public EdSocket::ISocketCb, public EdSmartSocket::INet
	{
		EdSmartSocket *mChildSock;
		EdSocket* mSvrSock;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("server start...");
				mSvrSock = new EdSocket;
				mSvrSock->setOnListener(this);
				mSvrSock->listenSock(7000);
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			return 0;
		}

		virtual void IOnSocketEvent(EdSocket *psock, int event) {
			if(event == SOCK_EVENT_INCOMING_ACCEPT) {
				int fd = mSvrSock->accept();
				assert(fd >0);
				assert(mChildSock == NULL);
				mChildSock = new EdSmartSocket;
				mChildSock->setOnNetListener(this);
				mChildSock->socketOpenChild(fd);
			}
		}

		void IOnNet(EdSmartSocket *psock, int event) {
			if(event == NETEV_DISCONNECTED) {
				logs("child disconnected...");
			} else if(event == NETEV_CONNECTED) {
				assert(0);
			} else if(event == NETEV_READ) {
				char buf[100+1];
				int rcnt = psock->recvPacket(buf, 100);
				if(rcnt>0) {
					buf[rcnt] = 0;
					logs("server recv: %s", buf);
					char tag[] = "server_tag: ";
					psock->sendPacket(tag, strlen(tag));
					psock->sendPacket(buf, rcnt);
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
	stask->wait();
	delete ctask;
	delete stask;
	fdcheck_end();
	logm("<<<< smart socket test OK\n");
}

#include "http/http_parser.h"
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
		testsmartsock(i);
		//testHttpBase(i);
//		testssl(i);
//		testHttpSever(i);
//		testMultiTaskInstance(1);
//		testreservefree(i);
//		testtimer(i);
//		testMainThreadTask(i);
//		testmsg(i);
//		testcurl(i);
	}
	return 0;
}
