/*
 * EdEventFd.h
 *
 *  Created on: Aug 28, 2014
 *      Author: netmind
 */

#ifndef EDEVENTFD_H_
#define EDEVENTFD_H_
#include "config.h"

#include "EdEvent.h"

namespace edft
{

class EdEventFd : public EdEvent
{
public:
	class IEventFd {
	public:
		virtual void IOnEventFd(EdEventFd *pefd, int cnt)=0;
	};
public:
	EdEventFd();
	virtual ~EdEventFd();
	virtual void OnEventRead();
	virtual void OnEventFd(int cnt);

	int open();
	void close();

	int raise();
	void setOnListener(IEventFd* cb);

private:
	IEventFd* mOnListener;
};

} /* namespace edft */

#endif /* EDEVENTFD_H_ */
