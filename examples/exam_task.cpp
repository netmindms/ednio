//============================================================================
// Name        : task_exam.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#define DBGTAG "atask"
#define DBG_LEVEL DBG_DEBUG

#include <sys/time.h>
#include "ednio/EdNio.h"
#include "applog.h"

using namespace std;
using namespace edft;
enum { UM_POST_MSG = EDM_USER+1, UM_SEND_MSG,};

class MainTask : public EdTask
{
private:
	int OnEventProc(EdMsg* pmsg)
	{
		if(pmsg->msgid == EDM_INIT)
		{
			logs("task init ...");
		}
		else if(pmsg->msgid == EDM_CLOSE)
		{
			logs("task closed ...");
		}
		else if(pmsg->msgid == UM_POST_MSG)
		{
			logs("test post message received, p1=%d, p2=%d", pmsg->p1, pmsg->p2);
		}
		else if(pmsg->msgid == UM_SEND_MSG)
		{
			logs("test send message received, p1=%d, p2=%d", pmsg->p1, pmsg->p2);
		}
		return 0;
	}
};

int main() {
	MainTask task;
	task.run();
	task.postMsg(UM_POST_MSG, 100, 200);
	task.sendMsg(UM_SEND_MSG, 0, 0);
	task.postExit();
	task.wait();
	return 0;
}
