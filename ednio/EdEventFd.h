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

class EdEventFd : public EdEvent
{
private:
	typedef std::function<void (int cnt) > Lis;
	Lis mLis;

public:
	EdEventFd();
	virtual ~EdEventFd();
	virtual void OnEventRead();
	virtual void OnEventFd(int cnt);

	int open(Lis lis=nullptr);
	void close();

	int raise();
	void setOnListener(Lis lis);
};

} /* namespace edft */

#endif /* EDEVENTFD_H_ */
