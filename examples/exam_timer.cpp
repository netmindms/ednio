
#define DBGTAG "atask"
#define DBG_LEVEL DBG_DEBUG

#include <sys/time.h>
#include "ednio/EdNio.h"

#include "applog.h"

using namespace std;
using namespace edft;

#define TIMER_ID1 1
#define TIMER_ID2 2

class MainTask : public EdTask
{
private:
	int OnEventProc(EdMsg* pmsg)
	{
		if(pmsg->msgid == EDM_INIT)
		{
			logs("task init ...");

			setTimer(TIMER_ID1, 1000);
			setTimer(TIMER_ID2, 5000);
		}
		else if(pmsg->msgid == EDM_CLOSE)
		{
			killTimer(TIMER_ID1);
			killTimer(TIMER_ID2);
			logs("task closed ...");
		}
		else if(pmsg->msgid == EDM_TIMER)
		{
			logs("timer expired, timer id=%d", pmsg->p1);
			if(pmsg->p1 == TIMER_ID2)
			{
				killTimer(TIMER_ID2);
				postExit();
			}
		}

		return 0;
	}
};

int main() {
	EdNioInit();
	MainTask task;
	task.run();
	task.wait();
	return 0;
}
