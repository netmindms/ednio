/*
 * test_mq.cpp
 *
 *  Created on: Jul 5, 2015
 *      Author: netmind
 */
#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "test"
#include <gtest/gtest.h>
#include "../src/EdNio.h"
#include "../src/EdMq.h"

using namespace edft;
using namespace std;

// how to mount mqueue to /dev
// # mkdir /dev/mqueue
// # mount -t mqueue none /dev/mqueue

TEST(ipc, mq) {
	EdTask task;
	EdMq rq;
	EdMq sq; // source q;

	task.setOnListener([&](EdMsg& msg) {
		if(msg.msgid == EDM_INIT) {
			int ret;
			rq.unlink("/testedq");

			ret = rq.create("/testedq", 128, 10);
			dbgd("create fd=%d", ret);
			rq.setOnListener([&](int event) {
				dbgd("sink read event......");
				if(event == EVT_READ) {
					string s = rq.recvString();
					cout << "recv : " << s << endl;
					if(s=="quit") {
						task.postExit();
					}
				}
			});
			ret = sq.open("/testedq");
			sq.changeEvent(0);
			dbgd("open fd=%d", ret);
			assert(ret>0);
			sq.send("start");
			sq.send("---");
			sq.send("quit");
		} else if(msg.msgid == EDM_CLOSE) {
			rq.unlink();
			rq.close();
			sq.close();
		}
		return 0;
	});
	task.run();
	task.wait();
}

