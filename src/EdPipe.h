/*
 * EdPipe.h
 *
 *  Created on: Jun 10, 2014
 *      Author: netmind
 */

#ifndef EDPIPE_H_
#define EDPIPE_H_
#include "config.h"
#include "EdEvent.h"
namespace edft {

class EdPipe : public EdEvent {
public:
	EdPipe();
	virtual ~EdPipe();
	virtual void OnEventRead(void);
	virtual void OnEventWrite(void);
public:
	class IPipeCb {
	public:
		virtual void IOnPipeEvent(EdPipe *espipe, int events)=0;
	};
	void open();
	void close();
	int send(const void *buf, int size);
	int recv(void *buf, int size);
	void setCallback(IPipeCb *cb);

private:
	int mSendFd, mRecvFd;
	IPipeCb *mPipeCb;
};

}
#endif /* EDPIPE_H_ */
