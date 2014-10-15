/*
 * EdMdbQueryStore.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: netmind
 */

#include "../config.h"

#define DBGTAG "MQRYS"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdMdbCnn.h"
#include "EdMdbQueryStore.h"

namespace edft
{

EdMdbQueryStore::EdMdbQueryStore()
{
	mStatus = 0;
	mMysql = NULL;
	mCnn = 0;
}

EdMdbQueryStore::~EdMdbQueryStore()
{
}

int EdMdbQueryStore::queryStart(const char* qs)
{
	dbgd("query, str=%s, status=%d", qs, mStatus);
	mStatus = 1;
	mMysql = mCnn->getMysql();
	int ret=0;
	int stt = mysql_real_query_start(&ret, mMysql, qs, strlen(qs));
	dbgd("query start, waitevt=%0x", stt);
	if(stt == 0)
	{
		stt = startStore();
	}
	return stt;
}

int EdMdbQueryStore::queryContinue(int waitevt)
{
	int stt;
	MYSQL_RES* res;
	dbgd("query continue, status=%d", mStatus);
	if (mStatus == 1)
	{
		int ret;
		stt = mysql_real_query_cont(&ret, mMysql, waitevt);
		if (stt == 0)
		{
			dbgd("  query cont end, start store...");
			mStatus = 2;
			stt = mysql_store_result_start(&res, mMysql);
			if (stt == 0)
			{
				dbgd("early store result end...");
				mStatus = 0;
				OnQueryEnd(res);
				mysql_free_result(res);
			}
		}
	}
	else if (mStatus == 2)
	{
		stt = mysql_store_result_cont(&res, mMysql, waitevt);
		if (stt == 0)
		{
			mStatus = 0;
			dbgd("store result end...");
			OnQueryEnd(res);
			mysql_free_result(res);
		}
	}

	return stt;

}

void EdMdbQueryStore::OnQueryEnd(MYSQL_RES* res)
{
}


int EdMdbQueryStore::startStore()
{
	MYSQL_RES* res;
	int stt;
	stt = mysql_store_result_start(&res, mMysql);
	if(stt == 0)
	{
		dbgd("early store result...res=%0x", res);
		OnQueryEnd(res);
		mysql_free_result(res);
	}
	return stt;
}


void EdMdbQueryStore::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
}

} /* namespace edft */
