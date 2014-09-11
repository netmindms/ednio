/*
 * EdHttpController.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPCONTROLLER_H_
#define EDHTTPCONTROLLER_H_

#include "../EdEventFd.h"

namespace edft
{

class EdHttpController : public EdEventFd
{
public:
	EdHttpController();
	virtual ~EdHttpController();
	virtual void OnRequest();
	virtual void OnContentRecvComplete();
	virtual void OnContentSendComplete();
	virtual void OnComplete();
	void close();

};

} /* namespace edft */

#endif /* EDHTTPCONTROLLER_H_ */
