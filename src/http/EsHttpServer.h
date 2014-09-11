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


using namespace edft;

enum {
	UV_START = EDM_USER+1,
};

class EsHttpServer: public EdSocket::ISocketCb
{
public:
	EsHttpServer();
	virtual ~EsHttpServer();
	virtual int OnEventProc(EdMsg* pmsg);
	virtual void IOnSocketEvent(EdSocket *psock, int event);

	EdSocket mSvrSock;

	void addService(EsHttpTask* ptask);
private:
	EdMutex mSvcMutex;
	EsHttpTask *mSvcList[100];
	int mSvcCount;
	int mSvcRound;
};

#endif /* ESHTTPSERVER_H_ */
