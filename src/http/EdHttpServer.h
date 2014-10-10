/*
 * EsHttpServer.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPSERVER_H_
#define ESHTTPSERVER_H_

#include "../config.h"

#include <list>
#include "EdHttpTask.h"

namespace edft {

enum {
	UV_START = EDM_USER+1,
};

typedef struct {
	int port;
	int ssl_port;
	int task_num;
} EdHttpSettings;

class EdHttpServer: public EdSocket::ISocketCb
{
public:
	EdHttpServer();
	virtual ~EdHttpServer();
	virtual void IOnSocketEvent(EdSocket *psock, int event);

	template<typename T>	void startService(EdHttpSettings *pset)
	{
		initCommon(pset);
		int mode = EdTask::getCurrentTask()->getRunMode();
		mSvcMutex.lock();
		for(int i=0;i<pset->task_num;i++)
		{
			EdHttpTask *ptask = new T;
			ptask->run(mode);
			mSvcList[mSvcCount++] = ptask;
		}
		mSvcMutex.unlock();
	}

	void stopService();
	static EdHttpSettings getDefaultSettings();

private:
	EdMutex mSvcMutex;
	EdSocket mSvrSock;
	EdSocket mSSLSvrSock;
	EdHttpTask *mSvcList[100];
	int mSvcCount;
	int mSvcRound;
	EdHttpSettings mSettings;

	void initCommon(EdHttpSettings* pset);
	int open(int port, bool ssl=false);
	void close();
};

} // namespace edft

#endif /* ESHTTPSERVER_H_ */
