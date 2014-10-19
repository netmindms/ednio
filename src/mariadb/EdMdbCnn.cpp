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
	mOpStatus = DB_OP_IDLE;
	mRes = NULL;
	mMysql = NULL;
	mCnnStatus = 0;
	mQuery = NULL;
	mOnLis = NULL;
	mTimer = new CnnTimer(this);
}

EdMdbCnn::~EdMdbCnn()
{
	closeDb();
}

void EdMdbCnn::OnDbConnected()
{
	if (mOnLis != NULL)
		mOnLis->IOnMdbCnnStatus(this, 1);
}

void EdMdbCnn::OnDbDisconnected()
{
	if (mOnLis != NULL)
		mOnLis->IOnMdbCnnStatus(this, 0);
}

int EdMdbCnn::connectDb(const char* ip, const char* dbname, const char* id, const char* pw, int port)
{
	if (mCnnStatus == DB_CNN_DISCONNECTED)
	{
		const char* ipaddr;
		ipaddr = (ip == NULL) ? "127.0.0.1" : ip;

		mMysql = (MYSQL*) malloc(sizeof(MYSQL));
		mysql_init(mMysql);
		mysql_options(mMysql, MYSQL_OPT_NONBLOCK, 0);
		MYSQL* retmysql = NULL;
		int status = mysql_real_connect_start(&retmysql, mMysql, ipaddr, id, pw, dbname, port, NULL, 0);
		int fd = mysql_get_socket(mMysql);
		dbgd("connecting db, fd=%d, waitevt=%0x, retmyql=%0x", fd, status, retmysql);
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
				setDbTimer();
			}
			registerEvent(evt);
			mOpStatus = DB_OP_CONNECTING;
			mCnnStatus = DB_CNN_CONNECTING;
		}
		else
		{
			mTimer->kill();
			if (retmysql != NULL)
			{
				dbgd("early connected...");
				registerEvent(EVT_READ | EVT_HANGUP);
				mCnnStatus = DB_CNN_CONNECTED;
				mOpStatus = DB_OP_IDLE;
			}
			else
			{
				dbgd("### Fail: connection error,...");
				closeDb();
				mCnnStatus = DB_CNN_DISCONNECTED;
			}
		}

	}
	else
	{
		dbgd("db already connected...");
	}
	return mCnnStatus;

}

void EdMdbCnn::OnEventRead()
{
	dbgv("cnn event read, status=%d", mOpStatus);
	if (mOpStatus == DB_OP_CONNECTING)
	{
		procCnnCont(MYSQL_WAIT_READ);
	}
	else if (mOpStatus == DB_OP_FREERES)
	{
		procFreeResultSet(MYSQL_WAIT_READ);
	}
	else
	{
		mQuery->IOnQueryContinue(MYSQL_WAIT_READ);
	}

}

void EdMdbCnn::OnEventWrite()
{
	dbgv("cnn event write, status=%d", mOpStatus);
	if (mOpStatus == DB_OP_CONNECTING)
	{
		procCnnCont(MYSQL_WAIT_WRITE);
	}
	else if (mOpStatus == DB_OP_QUERYING)
	{
		dbgd("changing event on write event, query=%0x", mQuery);
		mQuery->IOnQueryContinue(MYSQL_WAIT_WRITE);
	}
	else if (mOpStatus == DB_OP_FREERES)
	{
		procFreeResultSet(MYSQL_WAIT_WRITE);
	}
	else
	{
		dbge("### unexpected socket write event occured...");
		assert(0);
	}
}

void EdMdbCnn::OnEventHangup()
{
	dbgd("cnn event hangup, status=%d", mOpStatus);
	if (mOpStatus == DB_OP_CONNECTING)
	{
		procCnnCont(MYSQL_WAIT_EXCEPT);
	}
	else if (mOpStatus == DB_OP_FREERES)
	{
		procFreeResultSet(MYSQL_WAIT_EXCEPT);
	}
	else
	{
		dbgd("changing event on hangup event, query=%0x", mQuery);
		mQuery->IOnQueryContinue(MYSQL_WAIT_EXCEPT);
	}
}

void EdMdbCnn::procCnnCont(int waitevt)
{
	MYSQL *ret;
	int status = mysql_real_connect_cont(&ret, mMysql, waitevt);
	dbgd("proc connection continue, waitevt = %0x", status);
	if (status == 0)
	{
		dbgd("  connecion continue result=%0x", ret);
		changeWaitEvent(MYSQL_WAIT_READ | MYSQL_WAIT_EXCEPT);
		if (ret != NULL)
		{
			mCnnStatus = 1;
			mOpStatus = DB_OP_IDLE;
			OnDbConnected();
		}
		else
		{
			dbgd("  db disconnected,...");
			closeDb();
			OnDbDisconnected();
		}
	}
	else
	{
		changeWaitEvent(status);
	}
}

void EdMdbCnn::procFreeResultSet(int waitevt)
{
	int stt = mysql_free_result_cont(mRes, waitevt);
	if (stt == 0)
	{
		dbgd("free pending result set...");
		mRes = NULL;
		mOpStatus = DB_OP_IDLE;
	}
}

void EdMdbCnn::changeWaitEvent(int waitevt)
{
	dbgd("change wait event=%x", waitevt);
	//printf("change wait event.....waitevt=%d\n", waitevt);
	u32 evt = EVT_HANGUP;
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
	return mMysql;
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

void EdMdbCnn::closeDb()
{
	if (mMysql != NULL)
	{
		dbgd("close db ...");
		deregisterEvent();
		mysql_close(mMysql);
		free(mMysql);
		mMysql = NULL;
	}

	if (mTimer != NULL)
	{
		mTimer->kill();
		delete mTimer;
		mTimer = NULL;
	}

	mCnnStatus = DB_CNN_DISCONNECTED;
	mOpStatus = DB_OP_IDLE;
}

void EdMdbCnn::setDbTimer()
{
	int msec = mysql_get_timeout_value_ms(mMysql);
	dbgd("start wait timer for connection,... expire=%d", msec);
	mTimer->set(msec);
}

void EdMdbCnn::setOnListener(IMdbCnn* onlis)
{
	mOnLis = onlis;
}

void EdMdbCnn::disconnectDb()
{
	//if(mOpStatus)
}

void EdMdbCnn::setQuery(EdMdbQueryBase* qr)
{
	if(mQuery == NULL)
		mQuery = qr;
	else {
		dbge("### connection already query set...");
		return;
	}
}

void EdMdbCnn::freeResultSet(MYSQL_RES* res)
{
	dbgd("free result set, res=%x", res);
	int ret = mysql_free_result_start(res);
	if (ret == 0)
	{
		dbgd("  early free result end...");
		mOpStatus = DB_OP_IDLE;
	}
	else
	{
		mRes = res;
		mOpStatus = DB_OP_FREERES;
		changeWaitEvent(ret);
	}
}


void EdMdbCnn::resetQuery()
{
	mQuery = NULL;
}

} /* namespace edft */
