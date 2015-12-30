/*
 * test_msg.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: netmind
 */

#include "../src/EdNio.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <random>
#include <chrono>

using namespace edft;
using namespace std;

#define CLITASK_NUM 10L
#define SEND_CNT (1000000L/CLITASK_NUM)

TEST(msg, basic)
{
	static EdTask svrTask;

	struct msgdata_t {
		long rnum;
		long resp;
	};

	static int total_cnt=0;

	class CliTask: public EdTask
	{
	public:
		int tryCount;


		int OnEventProc(EdMsg& msg) override
		{
			if (msg.msgid == EDM_INIT)
			{
//				printf("cli task init, ptr=%x\n", this);
				tryCount = 0;
				//setTimer(2, 1);
//				postMsg(EDM_USER);
			}
			else if (msg.msgid == EDM_CLOSE)
			{
//				printf("cli task close, ptr=%x\n", this);

			}
			else if(msg.msgid == EDM_USER+1)
			{
				postMsg(EDM_USER);
			}
			else if (msg.msgid == EDM_USER)
			{
//				EXPECT_EQ(msg.p1, 2);
				tryCount++;
				msgdata_t d;
				d.rnum = random();
				d.resp = 0x1323233;
				int sret = svrTask.sendObj(EDM_USER, (void*) &d);
				if (sret)
					assert(0);
				//				ASSERT_EQ(sret, 0);
				//				ASSERT_EQ(d.rnum, d.resp);
				if (d.rnum != d.resp)
					assert(0);
				if (tryCount < SEND_CNT)
				{
					postMsg(EDM_USER);
				}
//				else
//					postExit();
			}
			return 0;
		}
	};

	svrTask.setOnListener([&svrTask](EdMsg& msg)
	{
		if(msg.msgid == EDM_INIT)
		{
//			cout << "svr task init..." << endl;
			svrTask.setTimer(1, 1000);
		}
		else if(msg.msgid == EDM_CLOSE)
		{
//			cout << "svr task close..." << endl;
			svrTask.killTimer(1);
		}
		else if(msg.msgid == EDM_USER)
		{
			total_cnt++;
			msgdata_t *pd = (msgdata_t*)msg.obj;
			assert(pd);
			pd->resp = pd->rnum;
			if(total_cnt == SEND_CNT*CLITASK_NUM) {
				svrTask.postExit();
			}
		}
		else if(msg.msgid == EDM_TIMER) {
			cout << "try count = " << total_cnt << endl;
		}
		return 0;
	});

	svrTask.run();

	vector<CliTask> cliTasks;
	cliTasks.resize(CLITASK_NUM);
	for (auto &t : cliTasks)
	{
		auto ret = t.run();
		ASSERT_EQ(ret, 0);
	}

	cout << "start send msg..." << endl;
	auto t1 = chrono::system_clock::now();
	for (auto &t : cliTasks)
	{
		t.postMsg(EDM_USER+1);
	}
	svrTask.wait();
	auto t2 = chrono::system_clock::now();
	cout << "duration="<< chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << endl;

	for(auto &t: cliTasks) {
		t.terminate();
	}


	ASSERT_EQ(total_cnt, CLITASK_NUM*SEND_CNT);

}

TEST(msg, msgobj) {
	EdTask task;
	string msg_name;

	class MyMsg: public EdMsgObj {
	public:
		string name;
	};

	task.setOnListener([&](EdMsg &msg){
		if(msg.msgid==EDM_INIT) {

		}
		else if(msg.msgid==EDM_USER) {
			auto msgobj = FETCH_MSGOBJ(MyMsg, msg);
			msg_name = move(msgobj->name);
		}
		else if(msg.msgid==EDM_USER+1) {
			string* pstr = (string*)msg.obj;
			*pstr = msg_name;
		}
		return 0;
	});
	task.run();
	unique_ptr<MyMsg> msgobj( new MyMsg );
	msgobj->name = "james";
	task.postObj(EDM_USER, move(msgobj));
	string recv_name;
	task.sendObj(EDM_USER+1, &recv_name);
	ASSERT_EQ(recv_name, "james");
	task.terminate();
}



