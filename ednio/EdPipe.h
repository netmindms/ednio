/*
 * EdPipe.h
 *
 *  Created on: Jun 10, 2014
 *      Author: netmind
 */

#ifndef EDPIPE_H_
#define EDPIPE_H_
#include <functional>
#include "ednio_config.h"
#include "EdEvent.h"
namespace edft {

class EdPipe : public EdEvent {
public:
	typedef std::function<void(int)> Lis;
	EdPipe();
	virtual ~EdPipe();
	virtual void OnEventRead(void);
	virtual void OnEventWrite(void);
public:
	int open(Lis lis=nullptr);
	void close();
	int send(const void *buf, int size);
	int recv(void *buf, int size);
	void setOnListener(Lis lis);

private:
	int mSendFd, mRecvFd;
	Lis mLis;
};

}
#endif /* EDPIPE_H_ */
