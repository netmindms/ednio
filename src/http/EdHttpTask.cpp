/*
 * EsHttpTask.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "HTTSK"
#define DBG_LEVEL DBG_WARN

#include "../ednio_config.h"

#include <stdexcept>
#include "../edslog.h"
#include "EdHttpTask.h"
#include "EdNotFoundHttpController.h"

namespace edft
{

EdHttpTask::EdHttpTask()
{
	mConfig.recv_buf_size = 4*1024;
}

EdHttpTask::~EdHttpTask()
{
}


void EdHttpTask::OnInitHttp()
{

}


int EdHttpTask::OnEventProc(EdMsg* pmsg)
{
	if (pmsg->msgid == EDM_INIT)
	{
		dbgd("http task init, ...");
		OnInitHttp();
	}
	else if (pmsg->msgid == EDM_CLOSE)
	{
		dbgd("close http task ...");
		release();
#if USE_SSL
		EdSSLContext::freeDefaultEdSSL();
#endif
	}
	else if (pmsg->msgid == EDMX_HTTPCNN)
	{
		dbgd("New TCP connection, fd=%d", pmsg->p1);
		EdHttpCnn* pcnn = mCnns.allocObj();
		dbgd("alloc cnn object ...");
		pcnn->initCnn(pmsg->p1, 0, this, pmsg->p2);
	}

	return 0;
}



EdHttpController* EdHttpTask::allocController(const char* url)
{
	try
	{
		urlmapinfo_t *info = mAllocMap.at(url);
		EdHttpController* ptr = info->alloc();
		ptr->mUserData = info->userObj;
		return ptr;
	} catch (out_of_range &e)
	{
		EdHttpController* ptr = new EdNotFoundHttpController;
		return ptr;
	}
}

void EdHttpTask::freeController(EdHttpController* pctrl)
{
	delete pctrl;
}

void EdHttpTask::release()
{
	dbgd("stopping service task, cur connection count=%d", mCnns.size());
	EdHttpCnn* pcnn;
	for (;;)
	{
		pcnn = mCnns.pop_front();
		if (pcnn == NULL)
			break;
		pcnn->close();
		delete pcnn;
	}

	dbgd("free url map...cnt=%d", mAllocMap.size());
	for (auto itr = mAllocMap.begin(); itr != mAllocMap.end(); itr++)
	{
		delete (itr->second);
	}
}

void EdHttpTask::removeConnection(EdHttpCnn* pcnn)
{
	dbgd("remove connection, cnn=%x", pcnn);
	mCnns.remove(pcnn);
	mCnns.freeObj(pcnn);
}




http_server_cfg_t* EdHttpTask::getConfig()
{
	return &mConfig;
}

#if USE_SSL
int EdHttpTask::setDefaultCertFile(const char* crtfile, const char* keyfile)
{
	return EdSSLContext::getDefaultEdSSL()->setSSLCertFile(crtfile, keyfile);
}


void EdHttpTask::setDefaultCertPassword(const char* pw)
{
	EdSSLContext::getDefaultEdSSL()->setCertPassword(pw);
}

int EdHttpTask::openDefaultCertFile(const char* crtfile, const char* keyfile, const char* pw)
{
	setDefaultCertPassword(pw);
	return setDefaultCertFile(crtfile, keyfile);
}
#endif
} // namespace edft
