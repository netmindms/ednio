/*
 * EdHttpServer.h
 *
 *  Created on: Sep 6, 2014
 *      Author: netmind
 */

#ifndef EDHTTPSERVER_H_
#define EDHTTPSERVER_H_

#include "EdHttpMsg.h"
#include "EdHttpController.h"
#include "../EdSocket.h"

namespace edft
{

class EdHttpServer: public EdSocket::ISocketCb
{
public:
	EdHttpServer();
	virtual ~EdHttpServer();
	virtual void IOnSocketEvent(EdSocket *psock, int event);
	int open(int port);
	void start();
	void close();
	template<typename T> void addTask(int instance_num)
	{
		// TODO add http task
		T* ptask = new T;

	}
	;


private:
	EdSocket mSvrSock;
	EdMutex mSvcMutex;
	EsHttpTask *mSvcList[100];
	int mSvcCount;
	int mSvcRound;

};

} /* namespace edft */

#endif /* EDHTTPSERVER_H_ */
