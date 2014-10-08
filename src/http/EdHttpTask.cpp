/*
 * EsHttpTask.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "htask"

#include "../config.h"

#include <stdexcept>
#include "../edslog.h"
#include "EdHttpTask.h"
#include "EdNotFoundHttpController.h"

namespace edft
{

EdHttpTask::EdHttpTask()
{

}

EdHttpTask::~EdHttpTask()
{
}

int EdHttpTask::OnEventProc(EdMsg* pmsg)
{
	if (pmsg->msgid == EDM_INIT)
	{
		dbgd("http task init, ...");
	}
	else if (pmsg->msgid == EDM_CLOSE)
	{
		dbgd("close http task ...");
		release();
		EdSSLContext::freeDefaultEdSSL();
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

int EdHttpTask::setDefaultCertFile(const char* crtfile, const char* keyfile)
{
	return EdSSLContext::getDefaultEdSSL()->setSSLCertFile(crtfile, keyfile);
}

int EdHttpTask::setDefaultCertMem(const void *crt, int crtlen, const void* key, int keylen)
{
	return EdSSLContext::getDefaultEdSSL()->setSSLCertMem((void*)crt, crtlen, (void*)key, keylen);
}

void EdHttpTask::setDefaultCertPassword(const char* pw)
{
	EdSSLContext::getDefaultEdSSL()->setCertPassword(pw);
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


int EdHttpTask::openDefaultCertFile(const char* crtfile, const char* keyfile, const char* pw)
{
	setDefaultCertPassword(pw);
	return setDefaultCertFile(crtfile, keyfile);
}

} // namespace edft
