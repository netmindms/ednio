//============================================================================
// Name        : testednio.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
  
#define DBGTAG "main0"
#define DBG_LEVEL DBG_DEBUG

#include "EdNio.h"
#include "EdTask.h"
#include "EdTimer.h"
#include "EdTime.h"
#include "edcurl/EdCurl.h"
#include "edcurl/EdMultiCurl.h"

using namespace std;
using namespace edft;

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

	MsgTestTask msgtask;
	dbgd(">>>> Test: Message, mode=%d", mode);
	msgtask.run(mode);
	msgtask.wait();
	dbgd("<<<< Message Test OK\n");
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
					if(dt > 1000+10) {
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
					if (t - usec_starttime > 1000+10)
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
	auto task = new TimerTest;
	task->run(mode);
	task->wait();
	delete task;
	dbgd("<<<< Timer test OK\n");
}

/*
 * Test scenario for Curl
 */
void testcurl(int mode)
{
#define CONNECT_TIMEOUT 5
#define TEST_DURATION (CONNECT_TIMEOUT+1)
#define LOAD_COUNT 400

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
				mLocalCurl->request("http://curl.haxx.se");

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
				mReuseCurl->request("http://curl.haxx.se");
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
					mLoadCurl[i]->request("http://curl.haxx.se");
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
					pcurl->request("curl.haxx.se");
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
	auto task = new CurlTest;
	task->run();
	task->wait();
	delete task;
	dbgd("<<<< Curl Test OK");
}

int main()
{

	EdNioInit();
	for (int i = 0; i < 2; i++)
	{
		//testmsg(i);
		testtimer(i);
		//testcurl(i);
	}
	return 0;
}
