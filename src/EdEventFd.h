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

using namespace std;

namespace edft
{

class EdEventFd : public EdEvent
{
public:
	class IEventFd {
	public:
		virtual void IOnEventFd(EdEventFd *pefd, int cnt)=0;
	};
private:
	IEventFd* mOnListener;
	function<void (EdEventFd* efd, int cnt) > mOnLisCb;

public:
	EdEventFd();
	virtual ~EdEventFd();
	virtual void OnEventRead();
	virtual void OnEventFd(int cnt);

	int open();
	void close();

	int raise();
	void setOnListener(IEventFd* cb);
	void setOnListener(decltype(mOnLisCb) lis);
};

} /* namespace edft */

#endif /* EDEVENTFD_H_ */
