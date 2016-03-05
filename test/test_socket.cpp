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
