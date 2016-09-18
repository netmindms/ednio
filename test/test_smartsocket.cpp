/*
 * test_smartsocket.cpp
 *
 *  Created on: Mar 5, 2015
 *      Author: netmind
 */

#define DBGTAG "SMSTE"
#define DBG_LEVEL DBG_VERBOSE

#include <iostream>
#include <memory>
#include <string>
#include <gtest/gtest.h>
#include "../ednio/EdNio.h"
#include "../ednio/edslog.h"

using namespace std;
using namespace ::testing;
using namespace edft;
using namespace std;

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
				mSvrSock.setOnListener([this](int event)
				{
					if(event== SOCK_EVENT_INCOMING_ACCEPT)
					{
						int fd = mSvrSock.accept();
						upChildSock.reset( new EdSmartSocket);
						upChildSock->setOnListener([this](int event)
								{
									if(event == NETEV_READABLE)
									{
										char buf[100];
										auto rcnt = upChildSock->recvPacket(buf, 100);
										if(rcnt>0)
										{
											buf[rcnt] = 0;
											upChildSock->sendPacket(buf, rcnt);
										}
									}
								});
						upChildSock->openChild(fd);
					}
				});
				mSvrSock.listenSock(9090);

				mSock.setOnListener([this](int event) {
					if(event == NETEV_CONNECTED)
					{
						if(&mSock == &mSock)
						{
							dbgd("connected...");
							mSock.sendPacket(sendstr.c_str(), sendstr.size());
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
						auto rcnt = mSock.recvPacket(buf, 100);
						dbgd("client recv cnt=%d", rcnt);
						if(rcnt>0)
						{
							recvstr.append(buf, rcnt);
						}
					}
				});
				mSock.connect("127.0.0.1", 9090);
				mTimer.set(500, 0, [this]() {
					dbgd("check recv data...");
					mTimer.kill();
					dbgd("client recv str=%s", recvstr.c_str());
					if(sendstr != recvstr)
					{
						ASSERT_EQ(0,1);
						assert(0);
					}
					postExit();
				});
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

