#include <iostream>
#include <gtest/gtest.h>
#include <ednio/EdNio.h>

using namespace std;
using namespace edft;

TEST(task, taskmsgque)
{
#define UM_MSG1 (EDM_USER+1)
#define UM_MSG2 (EDM_USER+2)

	EdTask task;
	EdTaskMsgQue mq;
	uint32_t handle1, handle2, handle3;
	task.setOnListener([&](EdMsg &msg){
		if(msg.msgid == EDM_INIT) {
			cout << "task init..." << endl;
			handle1 = EdTask::createTaskMsgQue([&](EdMsg& msg){
				if(msg.msgid == UM_MSG1	) {
					cout << "view msg1 " << msg.taskque_handle << endl;
					task.postTaskMsgQue(handle2, UM_MSG1, 0, 0);
				}
				return 0;
			});
			handle2 = EdTask::createTaskMsgQue([&](EdMsg& msg){
				if(msg.msgid == UM_MSG1	) {
					cout << "view msg2 " << msg.taskque_handle << endl;
				}
				return 0;
			});
			task.postTaskMsgQue(handle1, UM_MSG1, 0, 0);

			handle3 = mq.open([&](EdMsg& msg){
				if(msg.msgid == UM_MSG2) {
					cout << "task message queue: message received..." << endl;
					task.postExit();
				}
				return 0;
			});
//			mq.postMsg(UM_MSG2, 0, 0);
			task.postTaskMsgQue(handle3, UM_MSG2, 0, 0);
		}
		else if(msg.msgid == EDM_CLOSE){
			task.destroyTaskMsgQue(handle1);
			task.destroyTaskMsgQue(handle2);
			mq.close();
			cout << "task ending..."<< endl;
		}
		return 0;
	});
	task.runMain();
}
