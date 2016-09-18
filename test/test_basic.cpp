/*
 * test_basic.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: netmind
 */

#define DBGTAG "TIMRT"
#define DBG_LEVEL DBG_DEBUG

#include <iostream>
#include <gtest/gtest.h>
#include "../ednio/EdNio.h"
#include "../ednio/EdTimerPool.h"
#include <iostream>
#include <chrono>

#include "tglobal.h"

using namespace std;
using namespace testing;
using namespace edft;
using namespace std;

TEST(basic, task)
{
	class MyTask : public EdTask
	{
		int OnEventProc(EdMsg& msg) override
		{
			if(msg.msgid == EDM_INIT)
			{
				postExit();
			}
			else if(msg.msgid == EDM_CLOSE)
			{

			}
			return 0;
		}

	};

	FDCHK_S()
	MyTask task;
	task.run();
	task.wait();
	FDCHK_E()
}


TEST(basic, timer)
{
	using namespace std::chrono;
	static int wtime = 500;
	static system_clock::time_point stp;
	static system_clock::time_point etp;

	class MyTask : public EdTask
	{
		EdTimer mTimer;
		int OnEventProc(EdMsg& msg) override
		{
			if(msg.msgid == EDM_INIT)
			{
				stp = system_clock::now();
				mTimer.set(wtime, 0, [this](){
					etp = system_clock::now();
					mTimer.kill();
					postExit();
				});
			}
			else if(msg.msgid == EDM_CLOSE)
			{
				auto dur = duration_cast<milliseconds>(etp-stp);
				dbgd("timer expired, dur=%d", dur.count());
				if((dur.count() < wtime-50) || (dur.count() > wtime+50)) {
					assert(0);
				}
			}
			return 0;
		}

	};

	FDCHK_S()
	MyTask task;
	task.run();
	usleep(wtime*1000+20);
	task.terminate();
	FDCHK_E()
}

TEST(basic, timerpool)
{
	using namespace std::chrono;
	static int wtime = 500;
	static system_clock::time_point stp;
	static system_clock::time_point etp;

	class MyTask : public EdTask
	{
		EdTimerPool _tmpool;
		uint32_t _tid1=0, _tid2=0;
		int t1cnt=0, t2cnt=0;
		int OnEventProc(EdMsg& msg) override {
			if(msg.msgid == EDM_INIT) {
				_tmpool.open();
				_tid1 = _tmpool.setTimer(0, 1000, 1000, [this](int cnt) {
					dbgd("timer fired, _tid1=%u", _tid1);
				});
				_tid2 = _tmpool.setTimer(0, 300, 300, [this](int cnt) {
					dbgd("timer fired, _tid2=%u", _tid2);
					t2cnt++;
					if(t2cnt >= 4) {
						_tmpool.killTimer(_tid2);
					}
				});
				dbgd("new timer , tid1=%u", _tid1);
			} else if(msg.msgid == EDM_CLOSE) {
				_tmpool.close();
			}
			return 0;
		}

	};

	FDCHK_S()
	MyTask task;
	task.run();
	task.wait();
	FDCHK_E()
}

TEST(timer, perf)
{
	EdTask task;
	EdTimer timer;
	int hitcount=0;
	task.setOnListener([&](EdMsg& msg){
		if(msg.msgid == EDM_INIT) {
			timer.set(33, 0, [&](){
				usleep(10*1000);
				hitcount++;
			});
			task.setTimer(1, 10000);
			cout << "start timer for 10 sec" << endl;
		}
		else if(msg.msgid == EDM_CLOSE) {

		}
		else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			timer.kill();
			cout << "timer end" << endl;
			cout << "hitcount: " << hitcount << endl;
			if(hitcount>313 && hitcount<293)
				assert(0);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
}

