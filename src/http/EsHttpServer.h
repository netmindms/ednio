/*
 * EsHttpServer.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPSERVER_H_
#define ESHTTPSERVER_H_

#include <list>

#include "EsHttpTask.h"


namespace edft {

enum {
	UV_START = EDM_USER+1,
};

class EsHttpServer: public EdSocket::ISocketCb
{
public:
	EsHttpServer();
	virtual ~EsHttpServer();
	//virtual int OnEventProc(EdMsg* pmsg);
	virtual void IOnSocketEvent(EdSocket *psock, int event);

	template<typename T>	void addService(int num=1)
	{
		mSvcMutex.lock();
		for(int i=0;i<num;i++)
		{
			EsHttpTask *ptask = new T;
			ptask->run();
			mSvcList[mSvcCount++] = ptask;
		}
		mSvcMutex.unlock();
	}

	int open(int port);
	void close();

private:
	EdMutex mSvcMutex;
	EdSocket mSvrSock;
	EsHttpTask *mSvcList[100];
	int mSvcCount;
	int mSvcRound;
};

} // namespace edft

#endif /* ESHTTPSERVER_H_ */
