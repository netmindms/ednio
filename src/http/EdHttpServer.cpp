/*
 * EsHttpServer.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "HTSVR"
#define DBG_LEVEL DBG_WARN

#include "../config.h"

#include "../edslog.h"
#include "EdHttpServer.h"
#include "EdHttpCnn.h"

namespace edft
{

EdHttpServer::EdHttpServer()
{
	mSvcCount = 0;
	mSvcRound = 0;
}

EdHttpServer::~EdHttpServer()
{
}

void EdHttpServer::IOnSocketEvent(EdSocket* psock, int event)
{
	if (event == SOCK_EVENT_INCOMING_ACCEPT)
	{
		int fd = psock->accept();
		dbgd("new incoming cnn, fd=%d", fd);
		if (mSvcCount > 0)
		{
			int idx = (mSvcRound++ % mSvcCount);
			mSvcList[idx]->postMsg(EDMX_HTTPCNN, fd, psock == &mSvrSock ? 0 : 1);
		}
	}
}

int EdHttpServer::open(int port, bool ssl)
{
	EdSocket *psock;
	if (ssl == false)
	{
		psock = &mSvrSock;
	}
	else
	{
#if USE_SSL
		psock = &mSSLSvrSock;
#else
		assert(0);
#endif
	}
	psock->setOnListener(this);
	int ret = psock->listenSock(port);
	if (ret != 0)
	{
		dbge("### server listen port open fail, ret=%d");
	}
	return ret;
}

void EdHttpServer::close()
{
	dbgd("http server closing...task cnt=%d", mSvcCount);
	mSvrSock.close();
	mSSLSvrSock.close();
	for (int i = 0; i < mSvcCount; i++)
	{
		mSvcList[i]->terminate();
		delete mSvcList[i];
		mSvcList[i] = NULL;
	}
	mSvcCount = 0;

}

void EdHttpServer::initCommon()
{
	dbgd("init server....");
	EdHttpCnn::initHttpParser();
}

}
