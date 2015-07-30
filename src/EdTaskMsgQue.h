/*
 * EdTaskMsgQue.h
 *
 *  Created on: Jul 30, 2015
 *      Author: netmind
 */

#ifndef SRC_EDTASKMSGQUE_H_
#define SRC_EDTASKMSGQUE_H_
#include "EdTask.h"

namespace edft {

class EdTaskMsgQue {
public:
	EdTaskMsgQue();
	virtual ~EdTaskMsgQue();
	uint32_t open(TaskEventListener lis);
	void close();
	int postMsg(u16 msgid, u32 p1, u32 p2);
	int sendMsg(u16 msgid, u32 p1, u32 p2);
	int postObj(u16 msgid, void* obj);
	int sendObj(u16 msgid, void* obj);
private:
	uint32_t mHandle;
	EdTask *mTask;
};

} /* namespace edft */

#endif /* SRC_EDTASKMSGQUE_H_ */
