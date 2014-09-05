/*
 * EsHttpTask.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG
#define TAG "htask"

#include "../edslog.h"
#include "EsHttpTask.h"

__thread EsHttpTask *_tHttpTask=NULL;

EsHttpTask::EsHttpTask() : mCnns(100)
{

}

EsHttpTask::~EsHttpTask()
{
}

int EsHttpTask::OnEventProc(EdMsg* pmsg)
{
	if(pmsg->msgid == EDM_INIT)
	{
		_tHttpTask = this;
		dbgd("start http task...");
	}
	else if(pmsg->msgid == EDM_CLOSE)
	{
		dbgd("close http task...");
	}
	else if (pmsg->msgid == UV_HTTPCNN)
	{
		dbgd("new htt cnn...fd=%d", pmsg->p1);
		u32 hcnn = mCnns.allocHandle();
		EsHttpCnn* pcnn = mCnns.getObject(hcnn);
		if(!pcnn)
		{
			dbgd("alloc cnn object...");
			pcnn = new EsHttpCnn();
			mCnns.setObject(hcnn, pcnn);
			pcnn->setOnListener(this);
		}
		//mCnns.setObject(hcnn, pcnn);
		pcnn->initCnn(pmsg->p1, hcnn, this);
	}

	return 0;
}

void EsHttpTask::IOnSocketEvent(EdSocket* psock, int event)
{
	EsHttpCnn *pcnn = (EsHttpCnn*)psock;
	if(event == SOCK_EVENT_READ)
	{
		pcnn->procRead();
	}
	else if(event == SOCK_EVENT_DISCONNECTED)
	{
		dbgd("sock disconneted...fd=%d", psock->getFd());
		pcnn->procDisconnected();
		mCnns.freeHandle(pcnn->mHandle);

	}
}

void EsHttpTask::setController(char* uri, IUriControllerCb* cb)
{
	mContMap[uri] = cb;
}

IUriControllerCb* EsHttpTask::getController(string* uri)
{
	try
	{
		IUriControllerCb *cb;
		cb = mContMap.at(*uri);
		return cb;
	}
	catch (out_of_range &exp)
	{
		return NULL;
	}
}
