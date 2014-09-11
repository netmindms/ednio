/*
 * EdHttpServer.cpp
 *
 *  Created on: Sep 6, 2014
 *      Author: netmind
 */

#include <stdio.h>
#include "EdHttpServer.h"
namespace edft
{

EdHttpServer::EdHttpServer()
{
	// TODO Auto-generated constructor stub

}

EdHttpServer::~EdHttpServer()
{
	// TODO Auto-generated destructor stub
}


int EdHttpServer::open(int port)
{
	mListenSocket = new EdSocket;
	mListenSocket->setOnListener(this);
	mListenSocket->listenSock(port);
	return 0;
}

void EdHttpServer::IOnSocketEvent(EdSocket* psock, int event)
{
	// TODO accept code
}


void EdHttpServer::close()
{
	if(mListenSocket != NULL)
	{
		mListenSocket->close();
		delete mListenSocket;
		mListenSocket = NULL;
	}
}


void EdHttpServer::start()
{
	// TODO server start
}

} /* namespace edft */
