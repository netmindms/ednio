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
			mtTimer->setCallback(this);
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
	class TimerTest: public EdTask, public EdTimer::ITimerCb
	{
		EdTimer *mTimer;
		u32 period;
		int mExpCnt;
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				dbgd("  timer test init");
				mExpCnt = 0;
				mTimer = new EdTimer;
				mTimer->setCallback(this);
				mTimer->set(100);
				setTimer(1, 3000);
				period = EdTime::msecTime();

			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				dbgd("  timer test closing");
				assert(mTimer == NULL);

			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				killTimer(1);
				u32 t = EdTime::msecTime();
				dbgd("    task timer expire, time=%d", t - period);
				postExit();
			}
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{

			if (ptimer == mTimer)
			{
				mExpCnt++;
				if (mExpCnt >= 10)
				{
					dbgd("timer exp cnt=%d", mExpCnt);
					ptimer->kill();
					delete ptimer;
					mTimer = NULL;
				}
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



void testcurl(int mode)
{
	class CurlTest : public EdTask, public EdCurl::ICurlCb {
		EdMultiCurl *mMainCurl;
		EdCurl *mLocalCurl;
		EdCurl mAbnormalCurl;

		virtual int OnEventProc(EdMsg* pmsg) {
			if(pmsg->msgid == EDM_INIT) {
				mMainCurl = new EdMultiCurl;
				mMainCurl->open();
				mLocalCurl = new EdCurl;
				mLocalCurl->setCallback(this);
				mLocalCurl->open(mMainCurl);
				mLocalCurl->request("127.0.0.1");

				mAbnormalCurl.setCallback(this);
				mAbnormalCurl.open(mMainCurl);
				mAbnormalCurl.request("211.211.211.211");
				dbgd("    request abnormal curl=%x", &mAbnormalCurl);
				dbgd("      this is not going to be connected...");

			} else if(pmsg->msgid == EDM_CLOSE) {
				assert(mLocalCurl == NULL);
				mMainCurl->close();
				delete mMainCurl;
			}
			return 0;
		}

		virtual void IOnCurlStatus(EdCurl* pcurl, int status) {

			dbgd("curl status = %d, curl=%x", status, pcurl);
			if(pcurl == mLocalCurl) {
				dbgd("   ### Fail !!!, For this curl, status must not be reported...");
				assert(0);
			} else if(pcurl == &mAbnormalCurl) {
				dbgd("   abnormal curl status reported...");
				if(status == 0)
				{
					dbgd("    ### Fail : For this curl, status must be abnormal...");
					assert(status != 0);
				}
				pcurl->close();
			} else {
				assert(0);
			}
		}

		virtual void IOnCurlHeader(EdCurl* pcurl) {

		}

		virtual void IOnCurlBody(EdCurl* pcurl, void* ptr, int size) {
			if(pcurl == mLocalCurl) {
				char* buf = (char*)malloc(size+1);
				assert(buf != NULL);
				dbgd("body size = %d", size);
				memcpy(buf, ptr, size);
				buf[size] = 0;
				dbgd("    body: \n%s", buf);
				pcurl->close();
				delete mLocalCurl;mLocalCurl=NULL;
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
	//testtask();
	for(int i=0;i<2;i++)
	{
		testmsg(i);
		testtimer(i);
		testcurl(i);
	}
	return 0;
}
