/*
 * EdLocalEvent.h
 *
 *  Created on: Jan 18, 2016
 *      Author: netmind
 */

#ifndef EDNIO_EDLOCALEVENT_H_
#define EDNIO_EDLOCALEVENT_H_

#include <list>
#include <functional>
#include "EdType.h"
#include "EdEventFd.h"
namespace edft {

class EdLocalEvent: public EdEventFd {
public:
	typedef struct {
		u16 evt_id;
		u32 p1;
		u32 p2;
	} Event;
	typedef std::function<void (Event&)> Lis;
	EdLocalEvent();
	virtual ~EdLocalEvent();
	int open(size_t maxevt, Lis lis);
	void close();
	int postEvent(u16 evtid, u32 p1, u32 p2);
	void setOnListener(Lis lis);

private:
	virtual void OnEventFd(int cnt) override;
	std::list<Event*> mEvtQue;
	std::list<Event*> mDummyEvtQue;
	Lis mLis;
	size_t mMaxEvent;
};

} /* namespace edft */

#endif /* EDNIO_EDLOCALEVENT_H_ */
