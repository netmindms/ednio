/*
 * EdMariaCnn.cpp
 *
 *  Created on: Oct 11, 2014
 *      Author: netmind
 */

#include "../config.h"

#define DBGTAG "MYCNN"
#define DBG_LEVEL DBG_DEBUG

#include <stdlib.h>
#include <stdio.h>

#include "../EdType.h"
#include "../edslog.h"
#include "EdMariaCnn.h"
#include "EdMariaQuery.h"

namespace edft
{

EdMariaCnn::EdMariaCnn()
{
	mDbCnn = NULL;
	mCnnStatus = 0;
	mQuery = NULL;
}

EdMariaCnn::~EdMariaCnn()
{
	CHECK_FREE_MEM(mDbCnn);
}

void EdMariaCnn::OnDbConnected()
{
}

void EdMariaCnn::IOnTimerEvent(EdTimer* ptimer)
{
	dbgd("timeout...");
	ptimer->kill();
}

int EdMariaCnn::connect(const char* hostaddr, int port, const char* id, const char* pw, const char* dbname)
{
	if (mDbCnn == NULL)
	{
		mDbCnn = (MYSQL*) malloc(sizeof(MYSQL));
		mysql_init(mDbCnn);
		mysql_options(mDbCnn, MYSQL_OPT_NONBLOCK, 0);
		MYSQL* ret;
		int status = mysql_real_connect_start(&ret, mDbCnn, hostaddr, id, pw, dbname, 0, NULL, 0);
		int fd = mysql_get_socket(mDbCnn);
		dbgd("db cnn fd=%d, status=%0x", fd, status);
		if (status != 0)
		{
			setFd(fd);
			u32 evt = EVT_HANGUP;
			if (status & MYSQL_WAIT_READ)
				evt |= EVT_READ;
			if (status & MYSQL_WAIT_WRITE)
				evt |= EVT_WRITE;
			if (status & MYSQL_WAIT_TIMEOUT)
			{
				mTimer.setOnListener(this);
				mTimer.set(1000);
			}
			registerEvent(evt);
			mContProc = _procCnnCont;
			mOpStatus = DB_OP_CONNECTING;
			//mCurWaitProc = procCnnEvent;
		}

	}

	return 0;
}

void EdMariaCnn::OnEventRead()
{
	dbgd("cnn event read, status=%d", mOpStatus);
	mQuery->queryContinue(this, MYSQL_WAIT_READ);

#if 0
	mContProc(this, MYSQL_WAIT_READ);
	if (mOpStatus == DB_OP_CONNECTING)
	{
		procCnnCont(MYSQL_WAIT_READ);
	}
	else if (mOpStatus == DB_OP_QUERYING)
	{
		procQueryCont(MYSQL_WAIT_READ);
	}
	else if (mOpStatus == DB_OP_STORE)
	{
		procStoreCont(MYSQL_WAIT_READ);
	}
#endif
}

void EdMariaCnn::OnEventWrite()
{
	dbgd("cnn event write, status=%d", mOpStatus);
	mQuery->queryContinue(this, MYSQL_WAIT_WRITE);

//	if (mOpStatus == DB_OP_CONNECTING)
//	{
//		procCnnCont(MYSQL_WAIT_WRITE);
//	}
//	else if (mOpStatus == DB_OP_QUERYING)
//	{
//		procQueryCont(MYSQL_WAIT_WRITE);
//	}
//	else if (mOpStatus == DB_OP_STORE)
//	{
//		procStoreCont(MYSQL_WAIT_WRITE);
//	}
}

void EdMariaCnn::OnEventHangup()
{
	dbgd("cnn event hangup, status=%d", mOpStatus);
	mQuery->queryContinue(this, MYSQL_WAIT_EXCEPT);

//	if (mOpStatus == DB_OP_CONNECTING)
//	{
//		procCnnCont(MYSQL_WAIT_EXCEPT);
//	}
//	else if (mOpStatus == DB_OP_QUERYING)
//	{
//		procQueryCont(MYSQL_WAIT_EXCEPT);
//	}
//	else if (mOpStatus == DB_OP_STORE)
//	{
//		procStoreCont(MYSQL_WAIT_WRITE);
//	}
}

void EdMariaCnn::procCnnCont(int waitevt)
{
	MYSQL *ret;
	int status = mysql_real_connect_cont(&ret, mDbCnn, waitevt);
	dbgd("db connection continue, status = %0x", status);
	if (status == 0)
	{
		dbgd("  db connected,...");
		mCnnStatus = 1;
		mOpStatus = DB_OP_IDLE;
		OnDbConnected();
	}
	else
	{
		changeWaitEvent(status);
	}
}

void EdMariaCnn::procQueryCont(int waitevt)
{
	int ret;
	int status = mysql_real_query_cont(&ret, mDbCnn, waitevt);
	dbgd("db querying continue, status=%0x", status);
	if (status == 0)
	{
		dbgd("db query end, ");
		mOpStatus = 0;
		MYSQL_RES *res;
		status = mysql_store_result_start(&res, mDbCnn);
		if (status == 0)
		{
			OnQueryEnd(res);
		}
		else
		{
			mOpStatus = DB_OP_STORE;
			changeWaitEvent(status);
		}
	}
	else
	{
		changeWaitEvent(status);
	}
}

void EdMariaCnn::procStoreCont(int waitevt)
{
	int status;
	MYSQL_RES *res;
	status = mysql_store_result_cont(&res, mDbCnn, waitevt);
	dbgd("db storing continue, status=%0x", status);
	if (status == 0)
	{
		dbgd("db store end, ");
		mOpStatus = 0;
		OnQueryEnd(res);
	}
	else
	{
		changeWaitEvent(status);
	}
}

void EdMariaCnn::changeWaitEvent(int waitevt)
{
	u32 evt = 0; //evt = EVT_HANGUP;
	if (waitevt & MYSQL_WAIT_READ)
		evt |= EVT_READ;
	if (waitevt & MYSQL_WAIT_WRITE)
		evt |= EVT_WRITE;
	if (waitevt & MYSQL_WAIT_EXCEPT)
		evt |= EVT_HANGUP;
	if (waitevt & MYSQL_WAIT_TIMEOUT)
	{
		mTimer.setOnListener(this);
		mTimer.set(1000);
	}
	changeEvent(evt);
}

int EdMariaCnn::sqlQueryAndStore(const char* query)
{
	if (mCnnStatus == 0)
		return -1;

	int ret;
	int status = mysql_real_query_start(&ret, mDbCnn, query, strlen(query));
	if (status != 0)
	{
		changeWaitEvent(status);
		mOpStatus = DB_OP_QUERYING;
	}
	else
	{
		mOpStatus = DB_OP_IDLE;
	}
	return 0;
}

void EdMariaCnn::OnQueryEnd(MYSQL_RES* res)
{
	mysql_free_result(res);
}

int EdMariaCnn::sqlQueryAndFetch(const char* query)
{

}

void EdMariaCnn::_procCnnCont(EdMariaCnn* pcnn, int waitevt)
{
	pcnn->procCnnCont(waitevt);
}

void EdMariaCnn::_procQueryCont(EdMariaCnn* pcnn, int waitevt)
{
	pcnn->procQueryCont(waitevt);
}

MYSQL* EdMariaCnn::getMySql()
{
	return mDbCnn;
}

int EdMariaCnn::runQuery(EdMariaQuery* qr)
{
	if (mCnnStatus == 0)
		return -1;

	int ret;
	int status = mysql_real_query_start(&ret, mDbCnn, qr->getQueryString(), strlen(qr->getQueryString()));
	if (status != 0)
	{
		changeWaitEvent(status);
		mOpStatus = DB_OP_QUERYING;
	}
	else
	{
		mOpStatus = DB_OP_IDLE;

	}
	return 0;
}
} /* namespace edft */

