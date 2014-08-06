//============================================================================
// Name        : curlexam.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include "edslog.h"
#include "EdNio.h"
#include "EdTask.h"
#include "EdObjList.h"
#include "edssl/EdSSL.h"
#include "edssl/EdSSLSocket.h"
#include "edcurl/EdCurl.h"

using namespace std;
using namespace edft;

class MainTask: public EdTask
{
	EdCurl *ecurl;
	virtual int OnEventProc(EdMsg* pmsg)
	{
		if (pmsg->msgid == EDM_INIT)
		{

			ecurl = new EdCurl();

			ecurl->open();
			ecurl->request("http://127.0.0.1");

		}
		else if (pmsg->msgid == EDM_CLOSE)
		{
			if(ecurl) {
				ecurl->closeCurl();
			}
		}

		return 0;
	}

};

void testcurl()
{
	MainTask maintask;
	maintask.run();
	getchar();
	maintask.terminate();
}

int main()
{
	EdNioInit();
	testcurl();
	return 0;
}
