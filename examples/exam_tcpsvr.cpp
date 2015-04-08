#include <iostream>
#include <stdio.h>
#include <string>
#include <mutex>
#include <ednio/EdNio.h>
#include <ednio/EdSignalFd.h>
using namespace std;
using namespace edft;


#define UDM_NEW_CNN (EDM_USER+100)

u32 _gHandleSeed=0;

u32 GetNewHandle()
{
	u32 ret;
	mutex mtx;
	mtx.lock();
	ret = ++_gHandleSeed;
	mtx.unlock();
	return ret;
}

class TcpTask : public EdTask {
	int OnEventProc(EdMsg& msg) override {
		if(msg.msgid == EDM_INIT) {

		} else if(msg.msgid == EDM_CLOSE) {

		} else if(msg.msgid == UDM_NEW_CNN) {

		}
		return 0;
	}

};

class TcpMultiServer : public EdTask
{
	EdSocket mLisSock;
	EdSignalFd mSig;
	unordered_map<uint32_t, EdSocket> mCnns;
	int OnEventProc(EdMsg &msg) override {
		if(msg.msgid == EDM_INIT) {
			mSig.setOnListener([this](int sig){
				cout << "signal received.." << endl;
				postExit();
			});
			mSig.setSignal({SIGTERM, SIGINT});

			mLisSock.setOnListener([this](EdSocket &sock, int event){
				if(event == SOCK_EVENT_INCOMING_ACCEPT) {
					int fd = sock.accept();
					auto handle = GetNewHandle();
//					cout << "incoming ... fd="<< fd << ", handle=" << handle << endl;
					auto &cnn = mCnns[handle];
					cnn.setUserInt(handle);
					cnn.setOnListener([this](EdSocket &sock, int event){
						if(event == SOCK_EVENT_DISCONNECTED) {
							auto handle = sock.getUserInt();
							cout << "disconnected, fd=" << sock.getFd() << ", handle=" << handle << endl;
							sock.close();
							mCnns.erase(handle);
						} else if(event == SOCK_EVENT_READ) {
							cout << "read sock event, fd=" << sock.getFd() << endl;
							char buf[1024];
							auto rcnt = sock.recv(buf, sizeof(buf));
							if(rcnt>0) {
								string s(buf, rcnt);
								cout << s<< endl;
								auto wcnt = sock.send(buf, rcnt);
								cout << "echo resp" << endl;
							}
						}
					});
					cnn.openChildSock(fd);
				}

			});
			mLisSock.listenSock(9090);
		} else if(msg.msgid == EDM_CLOSE) {
			mCnns.clear();
			mLisSock.close();
			mSig.close();
		}

		return 0;
	}
};

int main(int argc, char* argv[])
{
	cout << "main start..." << endl;
	EdNioInit();
	TcpMultiServer svr;
	svr.runMain();
	return 0;
}
