/*
 * EsHttpServer.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "htsvr"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EsHttpServer.h"

EsHttpServer::EsHttpServer()
{
	mSvcCount = 0;
	mSvcRound = 0;
}

EsHttpServer::~EsHttpServer()
{
	// TODO Auto-generated destructor stub
}


void EsHttpServer::IOnSocketEvent(EdSocket* psock, int event)
{
	if(event == SOCK_EVENT_INCOMING_ACCEPT) {
		int fd = psock->accept();
		dbgd("new incoming cnn, fd=%d", fd);
		if(mSvcCount>0) {
			int idx = (mSvcRound++ % mSvcCount);
			mSvcList[idx]->postMsg(UV_HTTPCNN, fd, 0);
		}
	}
}

void EsHttpServer::addService(EsHttpTask* ptask)
{
	mSvcMutex.lock();
	mSvcList[mSvcCount++] = ptask;
	ptask->run();
	mSvcMutex.unlock();
}
