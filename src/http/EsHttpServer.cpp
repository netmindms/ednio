/*
 * EsHttpServer.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define TAG "htsvr"
#define LOG_LEVEL LOG_DEBUG

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

int EsHttpServer::OnEventProc(EdMsg* pmsg)
{

	if(pmsg->msgid == EDM_INIT)
	{
		dbgd("http server init...");
		//mSvcTask = new EsHttpTask();
		//mSvcTask->run();
	}
	else if(pmsg->msgid == EDM_CLOSE)
	{
		dbgd("http server close...");
	}
	else if(pmsg->msgid == UV_START)
	{
		dbgd("start listen port=%d", pmsg->p1);
		mSvrSock.listenSock(pmsg->p1, "0.0.0.0");
		mSvrSock.setOnListener(this);
	}
	return 0;
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
