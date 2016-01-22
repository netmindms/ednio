/*
 * EdEventFd.h
 *
 *  Created on: Aug 28, 2014
 *      Author: netmind
 */

#ifndef EDEVENTFD_H_
#define EDEVENTFD_H_
#include "ednio_config.h"

#include <functional>
#include "EdEvent.h"

namespace edft
{

class EdEventFd;
typedef std::function<void (EdEventFd &efd, int cnt) > EventFdListener;

class EdEventFd : public EdEvent
{
private:
	EventFdListener mOnLisCb;

public:
	EdEventFd();
	virtual ~EdEventFd();
	virtual void OnEventRead();
	virtual void OnEventFd(int cnt);

	int open();
	void close();

	int raise();
	void setOnListener(EventFdListener lis);
};

} /* namespace edft */

#endif /* EDEVENTFD_H_ */
