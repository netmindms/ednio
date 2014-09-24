/*
 * EsHttpTask.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "htask"

#include "../edslog.h"
#include "EsHttpTask.h"
#include "EdNotFoundHttpController.h"
#include <stdexcept>

namespace edft {


__thread EsHttpTask *_tHttpTask=NULL;

EsHttpTask::EsHttpTask()
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
		release();
	}
	else if (pmsg->msgid == UV_HTTPCNN)
	{
		dbgd("new htt cnn...fd=%d", pmsg->p1);
		//u32 hcnn = mCnns.allocHandle();
		EsHttpCnn* pcnn = mCnns.allocObj();
		dbgd("alloc cnn object...");
		//pcnn->setOnListener(this);
		pcnn->initCnn(pmsg->p1, 0, this, pmsg->p2);
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
		mCnns.freeObj(pcnn);
	}
}

void EsHttpTask::setController(char* uri, IUriControllerCb* cb)
{
	mContMap[uri] = cb;
}

EdHttpController* EsHttpTask::OnNewRequest(const char* method, const char* url)
{
	return NULL;
}


EdHttpController* EsHttpTask::getRegController(const char* url)
{
	try {
		//__alloc_controller allocf = mAllocMap.at(url);
		urlmapinfo_t *info = mAllocMap.at(url);
		EdHttpController* ptr = info->alloc();
		ptr->mUserData = info->userObj;
		return ptr;
	} catch(out_of_range &e)
	{
		EdHttpController* ptr = new EdNotFoundHttpController;
		return ptr;
	}
}

void EsHttpTask::freeController(EdHttpController* pctrl)
{
	delete pctrl;
}

void EsHttpTask::release()
{
	for(auto itr=mAllocMap.begin(); itr != mAllocMap.end(); itr++)
	{
		delete (itr->second);
	}
}

} // namespace edft
