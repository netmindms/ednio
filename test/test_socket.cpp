/*
 * test_socket.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: netmind
 */




#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include <unordered_map>

#include "../ednio/EdNio.h"

using namespace std;
using namespace edft;


TEST(socket, connect) {
	EdTask _task;
	EdSocket _svrSock;
	EdSocket _childSock;
	EdSocket _clientSock;
	string _clientPeerAddr;
	int _clientPeerPort;

	_task.setOnListener([&](EdMsg& msg) {
		if(msg.msgid == EDM_INIT) {
			_svrSock.listenSock(27070, "0.0.0.0", [&](int event) {
				if(event == SOCK_EVENT_INCOMING_ACCEPT) {
					int fd = _svrSock.accept();
					_childSock.openChildSock(fd, [&](int event) {
						if(event == SOCK_EVENT_READ) {
							char buf[128];
							_childSock.recv(buf, sizeof(128));
						} else if(event == SOCK_EVENT_DISCONNECTED) {
							puts("disconnected");
							_childSock.close();
						}
					});
				}
			});

			struct in_addr inaddr;
			_clientSock.connect("127.0.0.1", 27070, [&](int event) {
				if(event == SOCK_EVENT_CONNECTED) {
					puts("connected....");
					char buf[128]={0};
					uint16_t port;
					_clientSock.getPeerAddr(buf, &port);
					_clientPeerPort = port;
					_clientPeerAddr.assign(buf);
					_clientSock.close();
					_task.setTimer(1, 1000, 1000);

				} else if(event == SOCK_EVENT_DISCONNECTED) {

				}
			});

		} else if(msg.msgid == EDM_CLOSE) {
			_clientSock.close();
			_svrSock.close();
		} else if(msg.msgid == EDM_TIMER) {
			_task.postExit();
			_task.killTimer(1);
		}
		return 0;
	});
	_task.runMain();

	ASSERT_STREQ("127.0.0.1", _clientPeerAddr.c_str());
}

TEST(socket, reconnect) {
	EdTask task;
	EdSocket sock;
	EdSocket svrSock;
	unordered_map<int, EdSocket> sockMap;
	int connect_cnt=0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			auto r = svrSock.listenSock(13000, "0.0.0.0", [&](int event) {
				if(event == SOCK_EVENT_INCOMING_ACCEPT) {
					int fd = svrSock.accept();
					if(fd>0) {
						sockMap[fd].openChildSock(fd, [&sockMap, &fd](int event) {
							if(event == SOCK_EVENT_DISCONNECTED) {
								sockMap.erase(fd);
							}
						});
					}
				}
			});
			assert(r==0);
			r = sock.connect((uint32_t)0, 13000, [&sock, &task, &connect_cnt](int event) {
				if(event == SOCK_EVENT_CONNECTED) {
					connect_cnt++;
					sock.close();
					auto r = sock.connect("127.0.0.1", 13000, [&task, &sock, &connect_cnt](int event) {
						if(event == SOCK_EVENT_CONNECTED) {
							connect_cnt++;
							sock.close();
							task.postExit();
						} else {
							assert(0);
						}
					});
				} else if(event == SOCK_EVENT_DISCONNECTED) {
					assert(0);
				}
			});

		} else if(msg.msgid == EDM_CLOSE) {
			svrSock.close();
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(connect_cnt, 2);
}


TEST(socket, udp) {

	class MyTask: public EdTask
	{
	public:
		string send_msg="1234";
			string recv_msg;
		EdSocket mLeftSock, mRightSock;
		int OnEventProc(EdMsg& msg) override {
			if (msg.msgid == EDM_INIT) {
				mLeftSock.openSock(SOCK_TYPE_UDP, [this](int event) {
					if(event== SOCK_EVENT_READ) {
						char tmp[100];
						uint32_t raddr;
						uint16_t rport;
						auto rcnt = mLeftSock.recvFrom(tmp, 100, &raddr, &rport);
						if(rcnt>0) {
							mRightSock.sendto(raddr, rport, tmp, rcnt);
						}
					}
				});
				auto ret = mLeftSock.bindSock(5050, "0.0.0.0");
				assert(!ret);

				mRightSock.openSock(SOCK_TYPE_UDP, [this](int event) {
					if(event == SOCK_EVENT_READ){
						char tmp[100];
						uint32_t raddr;
						uint16_t rport;
						auto rcnt = mRightSock.recvFrom(tmp, 100, &raddr, &rport);
						assert(rcnt==4);
						recv_msg.assign(tmp, rcnt);
						postExit();
					}
				});
				ret = mRightSock.bindSock(5051, "0.0.0.0");
				assert(!ret);
				uint32_t rip = inet_addr("127.0.0.1");
				mLeftSock.sendto(rip, 5051, send_msg.data(), send_msg.size());
			}
			else if (msg.msgid == EDM_CLOSE)
			{
				mLeftSock.close();
				mRightSock.close();
				dbgd("task will be closed...");

			}
			return 0;
		}
	};
	MyTask task;
	task.run();
	task.wait();
	ASSERT_STREQ(task.send_msg.data(), task.recv_msg.data());
}
