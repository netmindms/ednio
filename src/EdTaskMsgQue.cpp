/*
 * EdTaskMsgQue.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: netmind
 */

#include "EdTaskMsgQue.h"

namespace edft {

EdTaskMsgQue::EdTaskMsgQue() {
	mHandle = 0;
	mTask = nullptr;
}

EdTaskMsgQue::~EdTaskMsgQue() {
}

uint32_t EdTaskMsgQue::open(TaskEventListener lis) {
	mTask = EdTask::getCurrentTask();
	if(mTask) {
		mHandle = EdTask::createTaskMsgQue(lis);
		return mHandle;
	}
	else {
		return 0;
	}
}

void EdTaskMsgQue::close() {
	if(mHandle) {
		EdTask::destroyTaskMsgQue(mHandle);
		mHandle = 0;
	}
}


int EdTaskMsgQue::postMsg(u16 msgid, u32 p1, u32 p2) {
	return mTask->postTaskMsgQue(mHandle, msgid, p1, p2);
}

int EdTaskMsgQue::sendMsg(u16 msgid, u32 p1, u32 p2) {
	return mTask->sendTaskMsgQue(mHandle, msgid, p1, p2);
}

int EdTaskMsgQue::postObj(u16 msgid, void* obj) {
	return mTask->postTaskMsgObj(mHandle, msgid, obj);
}

int EdTaskMsgQue::sendObj(u16 msgid, void* obj) {
	return mTask->sendTaskMsgObj(mHandle, msgid, obj);
}

} /* namespace edft */
