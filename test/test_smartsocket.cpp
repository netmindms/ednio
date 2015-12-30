/*
 * test_smartsocket.cpp
 *
 *  Created on: Mar 5, 2015
 *      Author: netmind
 */

#define DBGTAG "SMSTE"
#define DBG_LEVEL DBG_ERR

#include <memory>
#include <gtest/gtest.h>
#include "../src/EdNio.h"
#include "../src/edslog.h"

using namespace ::testing;
using namespace edft;

TEST(socket, smart)
{
	class MyTask: public EdTask
	{
		EdSmartSocket mSock;
		EdSocket mSvrSock;
		unique_ptr<EdSmartSocket> upChildSock;
		string sendstr = "Test Data String";
		string recvstr;
		EdTimer mTimer;

		int OnEventProc(EdMsg& msg) override
		{
			if (msg.msgid == EDM_INIT)
			{
				mSvrSock.setOnListener([this](EdSocket &sock, int event)
				{
					if(event== SOCK_EVENT_INCOMING_ACCEPT)
					{
						int fd = sock.accept();
						upChildSock.reset( new EdSmartSocket);
						upChildSock->setOnListener([this](EdSmartSocket &sock, int event)
								{
									if(event == NETEV_READABLE)
									{
										char buf[100];
										auto rcnt = sock.recvPacket(buf, 100);
										if(rcnt>0)
										{
											buf[rcnt] = 0;
											sock.sendPacket(buf, rcnt);
										}
									}
								});
						upChildSock->openChild(fd);
					}
				});
				mSvrSock.listenSock(9090);

				mSock.setOnListener([this](EdSmartSocket &sock, int event)
				{
					if(event == NETEV_CONNECTED)
					{
						if(&sock == &mSock)
						{
							dbgd("connected...");
							sock.sendPacket(sendstr.c_str(), sendstr.size());
						}
						else
						{
							ASSERT_EQ(0,0);
						}
					}
					else if(event == NETEV_DISCONNECTED)
					{
						dbgd("disconnected...");
						ASSERT_EQ(0,1);

					}
					else if(event == NETEV_READABLE)
					{
						char buf[100];
						auto rcnt = sock.recvPacket(buf, 100);
						dbgd("client recv cnt=%d", rcnt);
						if(rcnt>0)
						{
							recvstr.append(buf, rcnt);
						}
					}
				});
				mSock.connect("127.0.0.1", 9090);
				mTimer.setOnListener([this](EdTimer &timer)
				{
					dbgd("check recv data...");
					timer.kill();
					dbgd("client recv str=%s", recvstr.c_str());
					if(sendstr != recvstr)
					{
						ASSERT_EQ(0,1);
						assert(0);
					}
					postExit();
				});
				mTimer.set(500);
			}
			else if (msg.msgid == EDM_CLOSE)
			{
				dbgd("task will be closed...");
				mSock.close();
				upChildSock->close();
				mSvrSock.close();
			}
			return 0;
		}
	};
	MyTask task;
	task.run();
	task.wait();
}

