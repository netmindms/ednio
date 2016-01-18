/*
 * test_event.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: netmind
 */
#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "TEEVT"

#include <gtest/gtest.h>
#include "../ednio/EdNio.h"
#include "../ednio/EdLocalEvent.h"

using namespace edft;
using namespace std;

TEST(event, local) {
	EdTask task;
	EdLocalEvent local;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			dbgd("task init...");
			local.open(1000, [&](EdLocalEvent::Event &evt) {
				dbgd("local event id=%d, p1=%d, p2=%d", evt.evt_id, evt.p1, evt.p2);
				if(evt.evt_id == 0) {
					task.postExit();
				} else if(evt.evt_id == 2) {
					local.postEvent(3, 3, 3);
					local.postEvent(4, 4, 4);
				} else if(evt.evt_id == 4) {
					local.postEvent(0, 0, 0);
				}
			});
			local.postEvent(1, 1, 1);
			local.postEvent(2, 2, 2);
			task.setTimer(1, 1000);
		} else if(msg.msgid == EDM_CLOSE) {
			task.killTimer(1);
			local.close();
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			assert(false);
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(true, true);
}
