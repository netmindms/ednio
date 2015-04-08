//============================================================================
// Name        : exam_tcpcli.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <ednio/EdNio.h>
#include <atomic>
#include <mutex>
using namespace std;
using namespace edft;

struct cnn_req
{
	string ip;
	uint16_t port;
	int num;
};

u32 GetNewHandle()
{
	static u32 _gHandleSeed = 0;
	u32 ret;
	mutex mtx;
	mtx.lock();
	ret = ++_gHandleSeed;
	mtx.unlock();
	return ret;
}
class TcpClietTask: public EdTask
{
	int cntConnected;
	int cntDis;
	int cntSockFdFail;
	cnn_req reqInfo;
	unordered_map<u32, EdSocket> mSockList;
	function<void(EdSocket &sock, int event)> mCb;

	int OnEventProc(EdMsg& msg) override
	{
		if (msg.msgid == EDM_INIT)
		{
			cout << " cli task init..." << endl;
			cntConnected = 0;
			cntDis = 0;
			cntSockFdFail=0;
			mCb = [this](EdSocket &sock, int event)
			{
				if(event == SOCK_EVENT_CONNECTED)
				{
//					cout << "connected: " << ", handle=" << sock.getUserInt() << endl;
					cntConnected++;
				}
				else if(event == SOCK_EVENT_DISCONNECTED)
				{
					cntDis++;
//					cout << "@@ disconnected: " << ", handle=" << sock.getUserInt() << endl;
					sock.close();
					mSockList.erase(sock.getUserInt());
				}
				else if(event == SOCK_EVENT_READ)
				{
					char buf[1024];
					auto rcnt = sock.recv(buf, sizeof(buf));
					if(rcnt>0)
					{

					}
				}
			};
		}
		else if (msg.msgid == EDM_CLOSE)
		{
			cout << "cli task close..." << endl;
			mSockList.clear();
		}
		else if (msg.msgid == EDM_USER)
		{
			reqInfo = *((cnn_req*) msg.obj);
			delete ((cnn_req*) msg.obj);

			setTimer(1, 500);
			for (int i=0;i<reqInfo.num;i++)
			{
				auto handle = GetNewHandle();
				auto &sock = mSockList[handle];
				sock.setUserInt(handle);
				sock.setOnListener(mCb);
				auto ret = sock.openSock(SOCK_TYPE_TCP);
				if(ret>=0) {
					auto cret = sock.connect(reqInfo.ip.data(), reqInfo.port);

				}
				else
				{
					cntSockFdFail++;
//					cout << "### open sock fail" << endl;
				}
			}

		}
		else if(msg.msgid == EDM_TIMER)
		{
			cout << "======" <<endl;
			cout << "connected cnt: " << cntConnected << endl;
			cout << "disconnected cnt: " << cntDis << endl;
			cout << "socket open fail cnt: " << cntSockFdFail << endl;
			cout << "--------" <<endl;

		}
		return 0;
	}
	;
};

int main(int argc, char* argv[])
{

	TcpClietTask task;
	task.run();
	cnn_req *pinfo = new cnn_req;
	pinfo->ip = "127.0.0.1";
	pinfo->port = 9090;
	pinfo->num = 1000;
	task.postObj(EDM_USER, pinfo);
	task.wait();
	return 0;
}
