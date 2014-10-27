//============================================================================
// Name        : task_exam.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <sys/types.h>

#include "ednio/EdNio.h"
using namespace std;
using namespace edft;

void levlog(int lev, const char *tagid, int line, const char *fmtstr, ...)
{
	struct timeval tm;
	va_list ap;

	gettimeofday(&tm, NULL);
	struct tm* ptr_time = localtime(&tm.tv_sec);

	char buf[2048];

	int splen = 2 * lev;
	char spbuf[splen + 1];
	memset(spbuf, ' ', splen);
	spbuf[splen] = 0;

	va_start(ap, fmtstr);
	vsnprintf(buf, 4096 - 1, fmtstr, ap);
	va_end(ap);

	printf("%02d:%02d:%02d.%02d [%s]:%-5d %s%s\n", ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec, (int) (tm.tv_usec / 10000), tagid, line, spbuf, buf);
}

#define logs(...) {levlog(1, "SUB  ", __LINE__, __VA_ARGS__); }



class ServerTask: public EdTask, public EdSocket::ISocketCb
{
	EdSocket *mSock;
	EdSocket *mChildSock;
private:
	int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{
			mChildSock = NULL;
			mSock = NULL;

			dbgd("server task init ...");
			mSock = new EdSocket;
			mSock->setOnListener(this);
			mSock->listenSock(9090);
		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			dbgd("server task close ...");
			if (mChildSock != NULL)
			{
				mChildSock->close();
				delete mChildSock;
			}

			if (mSock != NULL)
			{
				mSock->close();
				delete mSock;
			}
		}
		return 0;
	}

	void IOnSocketEvent(EdSocket *psock, int event)
	{
		if (psock == mSock)
		{
			if (event == SOCK_EVENT_INCOMING_ACCEPT)
			{
				dbgd("  incoming connection ...");
				if (mChildSock == NULL)
				{
					mChildSock = new EdSocket;
					mChildSock->setOnListener(this);
					psock->acceptSock(mChildSock, this);
				}
				else
				{
					dbgd("### connection already exists...");
					dbgd("### this server only one connection...");
					psock->rejectSock();
				}
			}
		}
		else if (psock == mChildSock)
		{
			if (event == SOCK_EVENT_DISCONNECTED)
			{
				dbgd("client disconnected ...");
				mChildSock->close();
				delete mChildSock;
				mChildSock = NULL;
				postExit();
			}
			else if (event == SOCK_EVENT_READ)
			{
				char buf[100];
				int rcnt = psock->recv(buf, sizeof(buf));
				if (rcnt > 0)
				{
					dbgd("read buf, read cnt = %d", rcnt);
					buf[rcnt] = 0;
					dbgd("    receibed str = %s", buf);
				}
			}
		}
	}
};

class ClientTask: public EdTask, public EdSocket::ISocketCb
{
	EdSocket *mSock;
private:
	int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{
			dbgd("task init ...");
			mSock = new EdSocket;
			mSock->setOnListener(this);
			mSock->connect("127.0.0.1", 9090);
		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			dbgd("task closed ...");
			mSock->close();
			delete mSock;
		}
		else if (pmsg->msgid == EDM_TIMER)
		{
			killTimer(1);
			mSock->close();
			postExit();
		}

		return 0;
	}

	void IOnSocketEvent(EdSocket *psock, int event)
	{
		dbgd("on socket event, event=%d", event);
		if (event == SOCK_EVENT_CONNECTED)
		{
			dbgd("  connected...");
			const char *str = "This is a client hello message ...\n";
			psock->send(str, strlen(str));
			setTimer(1, 3000);
			dbgd("disconnect after 3 sec ");
		}
		else if (event == SOCK_EVENT_DISCONNECTED)
		{
			dbgd("  disconnected...");
			psock->close();
			postExit();
		}
	}
};

int main()
{
	ClientTask client;
	ServerTask server;
	server.run();
	client.run();
	server.wait();
	client.wait();
	return 0;
}
