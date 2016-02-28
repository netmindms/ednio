/*
 * test_socket.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: netmind
 */




#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include "../ednio/EdNio.h"

using namespace std;
using namespace edft;


TEST(socket, reconnect) {
	EdTask task;
	EdSocket sock;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			sock.setOnListener([&](EdSocket& as, int event) {
				if(event == SOCK_EVENT_CONNECTED) {
					cout<<"connected"<<endl;
					auto r = sock.connect((uint32_t)0, 3000);
					cout << "reconnect, ret=" << r << endl;
				} else if(event == SOCK_EVENT_DISCONNECTED) {
					cout<<"disconnected"<<endl;
				}
			});
			auto r = sock.connect((uint32_t)0, 3000);
			cout << 'conn r=' << r << endl;


		} else if(msg.msgid == EDM_CLOSE) {
			sock.close();
		}
		return 0;
	});
	task.runMain();
}
