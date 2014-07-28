//============================================================================
// Name        : edexam.cpp
// Author      : netmind
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "main "

#include "EdTask.h"
#include "EdSocket.h"

using namespace std;
using namespace edft; // libednio namespace

class MainTask: public EdTask, public EdSocket::ISocketCb
{
	EdSocket *mpSock;
	EdSocket mListenSock;
	EdTimer mTimer;

public:
	MainTask()
	{
		mpSock = NULL;
	}

	// default event message procedure....
	virtual int OnEventProc(EdMsg* pmsg)
	{
		// EDM_INIT message proc is a good position for initializing object or triggering your app starting.
		if (pmsg->msgid == EDM_INIT)
		{
			dbgd("task init......");

			testClientSocket();
			testListenSocket();
			testTimer();

			setSendMsgResult(pmsg, 0);

		}
		// EDM_CLOSE message proc is a googd position for cleaning up resources.
		else if (pmsg->msgid == EDM_CLOSE)
		{
			dbgd("task close......");
			if (mpSock)
			{
				mpSock->close();
				delete mpSock;
				mpSock = NULL;
			}

			mListenSock.close();

			mTimer.kill();
		}
		return 0;
	}

	void testClientSocket()
	{
		// test client socket
		mpSock = new EdSocket();
		mpSock->setCallback(this); // MainTask implements ISocketCb interface
		mpSock->connect("127.0.0.1", 6000);
	}



	void testListenSocket()
	{
		// test server listening socket
		mListenSock.setCallback(this);
		int retval = mListenSock.listenSock(9000); // MainTask implements ISocketCb interface
		if (retval < 0)
			mListenSock.close();
		dbgd("listen ret=%d", retval);
	}

	// This function is a simple example for EdTimer api
	void testTimer()
	{
		class TestTimerCb : public EdTimer::ITimerCb {
			virtual void IOnTimerEvent(EdTimer* ptimer)
			{
				dbgd("timer expired...");
				long cnt = (long)ptimer->getUser();
				if(--cnt<0) {
					dbgd("kill timer.......");
					ptimer->kill();
				}
				ptimer->setUser((void*)cnt);
			};
		};

		static TestTimerCb timercb;


		mTimer.setCallback(&timercb);
		mTimer.setUser((void*)10);
		mTimer.set(1000);
	}

	virtual void IOnSocketEvent(EdSocket *psock, int event)
	{
		if (psock == mpSock)
		{
			if (event == SOCK_EVENT_CONNECTED)
			{
				dbgd("connected...");
			}
			else if (event == SOCK_EVENT_DISCONNECTED)
			{
				dbgd("disconnected...");
				delete psock;
				mpSock = NULL;
			}
			else if (event == SOCK_EVENT_READ)
			{
				char buf[100];
				int rdcnt = psock->recv(buf, 100);
				dbgd("   read cnt = %d", rdcnt);
			}

		}
		else if (psock == &mListenSock)
		{
			static long childid = 0;
			if(childid > 3)
			{
				int fd = mListenSock.accept();
				EdSocket *childsock = new EdSocket();
				childsock->openChildSock(fd);
				childsock->setCallback(this);
				childsock->setUser((void*) ++childid);
			}
			else
			{
				mListenSock.rejectSock();
			}
		}
		else
		{
			long childid = (long) psock->getUser();
			if (childid != 0)
			{
				if (event == SOCK_EVENT_READ)
				{
					char buf[100];
					int rdcnt = psock->recv(buf, 100);
					dbgd("child socket read, id=%d, cnt=%d", childid, rdcnt);
				}
				else if (event == SOCK_EVENT_DISCONNECTED)
				{
					dbgd("child diconnected, id=%d", childid);
					delete psock;
				}
			}
		}
	}


};

int main()
{
	MainTask mtask;
	mtask.run();
	getchar();
	mtask.terminate();
	return 0;
}
