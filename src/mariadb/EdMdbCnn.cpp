/*
 * EdMdbCnn.cpp
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
#include "EdMdbCnn.h"
#include "EdMdbQueryBase.h"

namespace edft
{

EdMdbCnn::EdMdbCnn()
{
	mDbCnn = NULL;
	mCnnStatus = 0;
	mQuery = NULL;
	mTimer = new CnnTimer(this);
}

EdMdbCnn::~EdMdbCnn()
{
	CHECK_FREE_MEM(mDbCnn);
	CHECK_DELETE_OBJ(mTimer);
}

void EdMdbCnn::OnDbConnected()
{
}

int EdMdbCnn::connect(const char* hostaddr, int port, const char* id, const char* pw, const char* dbname)
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
				mTimer->set(1000);
			}
			registerEvent(evt);
			mOpStatus = DB_OP_CONNECTING;
		}

	}

	return 0;
}

void EdMdbCnn::OnEventRead()
{
	dbgd("cnn event read, status=%d", mOpStatus);
	if (mOpStatus == DB_OP_CONNECTING)
		procCnnCont(MYSQL_WAIT_READ);
	else
		mQuery->queryContinue(MYSQL_WAIT_READ);

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

void EdMdbCnn::OnEventWrite()
{
	dbgd("cnn event write, status=%d", mOpStatus);
	if (mOpStatus == DB_OP_CONNECTING)
		procCnnCont(MYSQL_WAIT_WRITE);
	else
		mQuery->queryContinue(MYSQL_WAIT_WRITE);

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

void EdMdbCnn::OnEventHangup()
{
	dbgd("cnn event hangup, status=%d", mOpStatus);
	if (mOpStatus == DB_OP_CONNECTING)
		procCnnCont(MYSQL_WAIT_EXCEPT);
	else
		mQuery->queryContinue(MYSQL_WAIT_EXCEPT);

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

void EdMdbCnn::procCnnCont(int waitevt)
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

void EdMdbCnn::changeWaitEvent(int waitevt)
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
		mTimer->set(1000);
	}
	changeEvent(evt);
}

MYSQL* EdMdbCnn::getMysql()
{
	return mDbCnn;
}

int EdMdbCnn::runQuery(EdMdbQueryBase* qr, const char *qs)
{
	if (mCnnStatus == 0 || mOpStatus != DB_OP_IDLE)
	{
		dbgd("### Fail: db connection error, cnn status=%d, op status=%d", mCnnStatus, mOpStatus);
		return -1;
	}

	int status = qr->query(qs);
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

EdMdbCnn::CnnTimer::CnnTimer(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
}

void EdMdbCnn::CnnTimer::OnTimer()
{
	kill();
	mCnn->procCnnCont(MYSQL_WAIT_TIMEOUT);
}

} /* namespace edft */
