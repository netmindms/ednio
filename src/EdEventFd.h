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
	class IEventFdCallback {
	public:
		virtual void IOnEventFd(int cnt)=0;
	};
public:
	EdEventFd();
	virtual ~EdEventFd();
	virtual void OnEventRead();
	virtual void OnEventFd(int cnt);

	int open();
	int close();

	void set();
	void setCallback(IEventFdCallback* cb);

private:
	IEventFdCallback* mCallback;
};

} /* namespace edft */

#endif /* EDEVENTFD_H_ */
