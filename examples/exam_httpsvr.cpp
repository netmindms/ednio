/*
 * exam_httpsvr.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: netmind
 */

#include <ednio/EdNio.h>
#include "applog.h"

using namespace edft;
using namespace std;

class MyController: public EdHttpController
{
	string mMsg = "This is a http body message";
	EdHttpStringReader *mBodyReader;
	virtual void OnHttpCtrlInit()
	{
		logs("http control init...");

	}
	virtual void OnHttpRequestHeader()
	{
		int method = getReqMethod();
		string str = getReqUrl();
		logs("method=%d, url=%s", method, str.c_str());

		logs("set body data : %s", mMsg.c_str());
		mBodyReader = new EdHttpStringReader;
		mBodyReader->setString(&mMsg);
		setRespBodyReader(mBodyReader, "text/html");
		sendHttpResp("200");
	}
	virtual void OnHttpComplete(int result)
	{
		delete mBodyReader;
	}
	virtual void OnHttpDataNew(EdHttpContent *pct)
	{

	}
	virtual void OnHttpDataContinue(EdHttpContent *pct, const void *buf, int len)
	{

	}
	virtual void OnHttpDataRecvComplete(EdHttpContent *pct)
	{

	}
	virtual void OnHttpRequestMsg()
	{

	}

};

class MyHttpTask: public EdHttpTask
{
	void OnInitHttp()
	{
		logs("http task init...");
		regController<MyController>("/test", NULL);
	}
};

class MainTask: public EdTask
{
	EdHttpServer *mServer;

	int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{
			logs("task init ...");
			mServer = new EdHttpServer();
			EdHttpSettings settings = EdHttpServer::getDefaultSettings();
			settings.task_num = 1;
			settings.port = 9090;
			settings.ssl_port = 0;
			mServer->startService<MyHttpTask>(&settings);

		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			logs("task close ...");
			mServer->stopService();
		}
		return 0;
	}
};

int main()
{
	EdNioInit();
	MainTask* task = new MainTask();
	task->run();
	for (;;)
	{
		int c = getchar();
		if (c == 'q')
		{
			task->terminate();
			break;
		}
	}
	delete task;
	return 0;
}
