/*
 * EsHttpServer.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPSERVER_H_
#define ESHTTPSERVER_H_

#include <list>

#include "EdHttpTask.h"


namespace edft {

enum {
	UV_START = EDM_USER+1,
};

class EdHttpServer: public EdSocket::ISocketCb
{
public:
	EdHttpServer();
	virtual ~EdHttpServer();
	//virtual int OnEventProc(EdMsg* pmsg);
	virtual void IOnSocketEvent(EdSocket *psock, int event);

	template<typename T>	void startService(int num=1)
	{
		int mode = EdTask::getCurrentTask()->getRunMode();
		mSvcMutex.lock();
		for(int i=0;i<num;i++)
		{
			EdHttpTask *ptask = new T;
			ptask->run(mode);
			mSvcList[mSvcCount++] = ptask;
		}
		mSvcMutex.unlock();
	}

	int open(int port, bool ssl=false);
	void close();

private:
	EdMutex mSvcMutex;
	EdSocket mSvrSock;
	EdSocket mSSLSvrSock;
	EdHttpTask *mSvcList[100];
	int mSvcCount;
	int mSvcRound;
};

} // namespace edft

#endif /* ESHTTPSERVER_H_ */
