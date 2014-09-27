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

namespace edft {


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
			mSvcList[idx]->postMsg(UV_HTTPCNN, fd, psock==&mSvrSock ? 0:1);
		}
	}
}

int EsHttpServer::open(int port, bool ssl)
{
	EdSocket *psock;
	if(ssl == false) {
		psock = &mSvrSock;
	} else {
		psock = &mSSLSvrSock;
	}
	psock->setOnListener(this);
	int ret = psock->listenSock(port);
	if(ret != 0)
	{
		dbge("### server listen port open fail, ret=%d");
	}
	return ret;
}

void EsHttpServer::close()
{
	dbgd("http server closing...task cnt=%d", mSvcCount);
	mSvrSock.close();
	mSSLSvrSock.close();
	for(int i=0;i<mSvcCount;i++)
	{
		mSvcList[i]->terminate();
		delete mSvcList[i];
		mSvcList[i] = NULL;
	}
	mSvcCount = 0;

}

#if 0
void EsHttpServer::addService(EsHttpTask* ptask)
{
	mSvcMutex.lock();
	mSvcList[mSvcCount++] = ptask;
	ptask->run();
	mSvcMutex.unlock();
}
#endif


}
