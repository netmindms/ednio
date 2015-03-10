/*
 * EsHttpTask.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "HTTSK"
#define DBG_LEVEL DBG_DEBUG

#include "../ednio_config.h"

#include <stdexcept>
#include "../edslog.h"
#include "EdHttpTask.h"
#include "EdNotFoundHttpController.h"

#define htlogd(FMT, ...) dbgd("<t%02d> " FMT, mTaskId, ## __VA_ARGS__);
namespace edft
{

EdHttpTask::EdHttpTask()
{
	mConfig.recv_buf_size = 4*1024;
	mTaskId = -1;
	mCnnHandleSeed = 0;
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
		//dbgd("<%02d> http task init, ...", mTaskId);
		mTaskId = pmsg->p1;
		htlogd("http task init, ...");
		OnInitHttp();
	}
	else if (pmsg->msgid == EDM_CLOSE)
	{
		htlogd("close http task ...", mTaskId);

		release();
#if USE_SSL
		EdSSLContext::freeDefaultEdSSL();
#endif
	}
	else if (pmsg->msgid == EDMX_HTTPCNN)
	{
		htlogd("New TCP connection, fd=%d", pmsg->p1);
		EdHttpCnn* pcnn = mCnns.allocObj();
		htlogd("alloc cnn object ...");
		pcnn->initCnn(pmsg->p1, ++mCnnHandleSeed, this, pmsg->p2);
		mCnns.push_back(pcnn);
	}

	return 0;
}



EdHttpController* EdHttpTask::allocController(const char* url)
{
	EdHttpController * pcont=NULL;
	try
	{
		urlmapinfo_t *info = mAllocMap.at(url);
		pcont = info->alloc();
		pcont->mUserData = info->userObj;
		htlogd("static url matcing found");
		return pcont;
	} catch (out_of_range &e)
	{
		//htlogd("static url matching: not found");
	}

	if(pcont == NULL)
	{
		auto ulen = strlen(url);
		for(auto itr=mUrlPathMap.begin();itr != mUrlPathMap.end(); itr++)
		{
			urlmapinfo_t *info = *itr;
			if(info->path.size() <= ulen) {
				if( info->path.compare(0, info->path.size(), url, 0, info->path.size()) == 0 ) {
					pcont = info->alloc();
					pcont->mUserData = info->userObj;
					htlogd("path matching found...");
					return pcont;
				}
			}
		}
	}

	htlogd("no matched url...");
	pcont = new EdNotFoundHttpController;
	return pcont;
}

void EdHttpTask::freeController(EdHttpController* pctrl)
{
	delete pctrl;
}

void EdHttpTask::release()
{
	htlogd("stopping service tasks, current connection count=%d", mCnns.size());
	EdHttpCnn* pcnn;
	for (;;)
	{
		pcnn = mCnns.pop_front();
		if (pcnn == NULL)
			break;
		pcnn->close();
		//delete pcnn;
		mCnns.freeObj(pcnn);
	}

	htlogd("free url map...cnt=%d", mAllocMap.size());
	for (auto itr = mAllocMap.begin(); itr != mAllocMap.end(); itr++)
	{
		delete (itr->second);
	}

	htlogd("free url path map, cnt=%d", mUrlPathMap.size());
	for(auto itr=mUrlPathMap.begin(); itr != mUrlPathMap.end(); itr++)
	{
		delete (*itr);
	}
}

void EdHttpTask::removeConnection(EdHttpCnn* pcnn)
{
	htlogd("remove connection, cnn=%x", pcnn);
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
