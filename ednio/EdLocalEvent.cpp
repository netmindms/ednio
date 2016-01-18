/*
 * EdLocalEvent.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: netmind
 */
#define DBG_LEVEL DBG_WARN
#define DBGTAG "LOEVT"

#include "edslog.h"
#include "EdLocalEvent.h"

namespace edft {

EdLocalEvent::EdLocalEvent() {
	mMaxEvent = 1000;
}

EdLocalEvent::~EdLocalEvent() {
	assert(mEvtQue.size()==0);
	assert(mDummyEvtQue.size()==0);
}

int EdLocalEvent::open(size_t maxevt, Lis lis) {
	mMaxEvent = maxevt;
	mLis = lis;
	return EdEventFd::open();
}

void EdLocalEvent::setOnListener(Lis lis) {
	mLis = lis;
}

int EdLocalEvent::postEvent(u16 evtid, u32 p1, u32 p2) {
	Event *ptr=nullptr;
	if(mEvtQue.size()>=mMaxEvent) {
		dbge("### Error: too many events pending..., cnt=%d, dummy=%d", mEvtQue.size(), mDummyEvtQue.size());
		return -1;
	}
	if(mDummyEvtQue.size()>0) {
		mEvtQue.splice(mEvtQue.end(), mDummyEvtQue, mDummyEvtQue.begin());
		ptr = mEvtQue.back();
	} else {
		ptr = new Event;
		mEvtQue.push_back(ptr);
	}
	ptr->evt_id = evtid;
	ptr->p1 = p1;
	ptr->p2 = p2;
	return raise();
}

void EdLocalEvent::close() {
	dbgd("que cnt=%d, dummy cnt=%d", mEvtQue.size(), mDummyEvtQue.size());
	for(auto ptr: mDummyEvtQue) {
		delete ptr;
	}
	mDummyEvtQue.clear();
	for(auto ptr: mEvtQue) {
		delete ptr;
	}
	mEvtQue.clear();
	EdEventFd::close();
}

void EdLocalEvent::OnEventFd(int cnt) {
	for(auto i=0;i<cnt;i++) {
		if(!mEvtQue.empty()) {
			mDummyEvtQue.splice(mDummyEvtQue.begin(), mEvtQue, mEvtQue.begin());
			auto *pevt = mDummyEvtQue.front();
			if(mLis) mLis(*pevt);
		}
	}
}

} /* namespace edft */
