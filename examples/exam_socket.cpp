//============================================================================
// Name        : task_exam.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "ednio/EdNio.h"
#include "applog.h"

using namespace std;
using namespace edft;

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

			logs("server task init ...");
			mSock = new EdSocket;
			mSock->setOnListener(this);
			mSock->listenSock(9090);
		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			logs("server task close ...");
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
				logs("  incoming connection ...");
				if (mChildSock == NULL)
				{
					mChildSock = new EdSocket;
					mChildSock->setOnListener(this);
					psock->acceptSock(mChildSock, this);
				}
				else
				{
					logs("### connection already exists...");
					logs("### this server only one connection...");
					psock->rejectSock();
				}
			}
		}
		else if (psock == mChildSock)
		{
			if (event == SOCK_EVENT_DISCONNECTED)
			{
				logs("client disconnected ...");
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
					logs("read buf, read cnt = %d", rcnt);
					buf[rcnt] = 0;
					logs("    receibed str = %s", buf);
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
			logs("task init ...");
			mSock = new EdSocket;
			mSock->setOnListener(this);
			mSock->connect("127.0.0.1", 9090);
		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			logs("task closed ...");
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
		logs("on socket event, event=%d", event);
		if (event == SOCK_EVENT_CONNECTED)
		{
			logs("  connected...");
			const char *str = "This is a client hello message ...\n";
			psock->send(str, strlen(str));
			setTimer(1, 3000);
			logs("disconnect after 3 sec ");
		}
		else if (event == SOCK_EVENT_DISCONNECTED)
		{
			logs("  disconnected...");
			psock->close();
			postExit();
		}
	}
};

int main()
{
	EdNioInit();
	ClientTask client;
	ServerTask server;
	server.run();
	client.run();
	server.wait();
	client.wait();
	return 0;
}
