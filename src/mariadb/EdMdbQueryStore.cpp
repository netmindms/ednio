/*
 * EdMdbQueryStore.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: netmind
 */

#include "../ednio_config.h"

#define DBGTAG "MQRYS"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdMdbType.h"
#include "EdMdbCnn.h"
#include "EdMdbQueryStore.h"

namespace edft
{

EdMdbQueryStore::EdMdbQueryStore(EdMdbCnn* pcnn)
{
	mOpStatus = 0;
	mMysql = NULL;
	mCnn = pcnn;
	mRes = NULL;
}

EdMdbQueryStore::~EdMdbQueryStore()
{
	close();
}

int EdMdbQueryStore::query(const char* qs, int *perr)
{
	dbgd("query, str=%s, status=%d", qs, mOpStatus);
	int ret;
	mMysql = mCnn->getMysql();
	mCnn->setQuery(this);

	int stt = mysql_real_query_start(perr, mMysql, qs, strlen(qs));
	dbgd("query start, waitevt=%0x", stt);
	if (stt == 0)
	{
		dbgd("  early result end, err=%d", *perr);
		if (*perr == 0)
		{
			ret = startStore();
		}
		else
		{
			mOpStatus = OP_INIT;
			ret = MDB_COMPLETE;
		}
	}
	else if (stt != 0)
	{
		dbgd("  query pending...");
		mOpStatus = OP_QUERYING;
		mCnn->changeWaitEvent(stt);
		ret = MDB_CONTINUE;
	}
	return ret;
}

int EdMdbQueryStore::IOnQueryContinue(int waitevt)
{
	int stt;
	int ret;
	dbgd("query continue, status=%d", mOpStatus);
	if (mOpStatus == OP_QUERYING)
	{
		int err;
		stt = mysql_real_query_cont(&err, mMysql, waitevt);
		if (stt == 0)
		{
			dbgd("  query cont end, start store...");
#if 1
			ret = startStore();
			if (ret == MDB_COMPLETE)
			{
				OnQueryEnd(mRes);
			}
#else
			mStatus = 2;
			stt = mysql_store_result_start(&mRes, mMysql);
			if (stt == 0)
			{
				dbgd("early store result end...");
				mStatus = 0;
				close();
				OnQueryEnd (mRes);
			}
#endif
		}
	}
	else if (mOpStatus == OP_STORING)
	{
		stt = mysql_store_result_cont(&mRes, mMysql, waitevt);
		if (stt == 0)
		{
			dbgd("store result end...");
			mOpStatus = OP_INIT;
			OnQueryEnd(mRes);
			//mysql_free_result(mRes);
		}
	}

	return stt;

}

void EdMdbQueryStore::OnQueryEnd(MYSQL_RES* res)
{
}

int EdMdbQueryStore::startStore()
{
	dbgd("start storing...");
	int stt;
	stt = mysql_store_result_start(&mRes, mMysql);
	if (stt == 0)
	{
		dbgd("  early store result...res=%0x", mRes);
		//OnQueryEnd(res);
		//mysql_free_result(res);
		mOpStatus = OP_INIT;
		return MDB_COMPLETE;
	}
	else
	{
		dbgd("  storing pending, ...");
		mOpStatus = OP_STORING;
		mCnn->changeWaitEvent(stt);
		return MDB_CONTINUE;
	}
}

void EdMdbQueryStore::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
}

void EdMdbQueryStore::close()
{
	if (mMysql != NULL)
	{
		mCnn->resetQuery();
		mMysql = NULL;
		if (mRes != NULL)
		{
			dbgd("close query...");
			mCnn->freeResultSet(mRes);
			mRes = NULL;
		}
	}
}

MYSQL_RES* EdMdbQueryStore::getResult()
{
	return mRes;
}

MYSQL_ROW EdMdbQueryStore::getRow()
{
	return mysql_fetch_row(mRes);
}

} /* namespace edft */
