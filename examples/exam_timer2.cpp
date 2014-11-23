/*
 * exam_timer2.cpp
 *
 *  Created on: Oct 23, 2014
 *      Author: netmind
 */

#define DBGTAG "atask"
#define DBG_LEVEL DBG_DEBUG

#include "applog.h"
#include "ednio/EdNio.h"

using namespace std;
using namespace edft;

class MainTask : public EdTask, public EdTimer::ITimerCb
{
	EdTimer *mTimer;
private:
	virtual int OnEventProc(EdMsg* pmsg)
	{
		if(pmsg->msgid == EDM_INIT)
		{
			dbgd("task init ...");
			mTimer = new EdTimer;
			mTimer->setOnListener(this);
			mTimer->set(1000);

		}
		else if(pmsg->msgid == EDM_CLOSE)
		{
			dbgd("task closed ...");
			mTimer->kill();
			delete mTimer;
		}

		return 0;
	}

	virtual void IOnTimerEvent(EdTimer* ptimer)
	{
		static int expcnt=0;

		if(ptimer == mTimer)
		{
			dbgd("timer expired, cnt=%ld", ptimer->getHitCount());
			expcnt++;
			if(expcnt>=5) {
				mTimer->kill();
				postExit();
			}
		}
	}
};

int main() {
	EdNioInit();
	MainTask task;
	task.run();
	task.wait();
	return 0;
}
