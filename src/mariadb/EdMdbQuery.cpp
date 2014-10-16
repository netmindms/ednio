/*
 * EdMdbQuery.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: netmind
 */

#define DBGTAG "MYQRY"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdMdbCnn.h"
#include "EdMdbQuery.h"

namespace edft
{

EdMdbQuery::EdMdbQuery()
{
	mCnn = NULL;
	mMysql = NULL;
	mStatus = 0;
}

EdMdbQuery::~EdMdbQuery()
{
}

void EdMdbQuery::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
	mMysql = pcnn->getMysql();
}

int EdMdbQuery::queryStart(const char* qs)
{
	dbgd("query, str=%s, ", qs);
	mMysql = mCnn->getMysql();
	int err = 0;
	int stt = mysql_real_query_start(&err, mMysql, qs, strlen(qs));
	dbgd("query start, waitevt=%0x, err=%d", stt, err);
	if (stt == 0)
	{
		dbgd("  early query end, err=%d", err);
		OnQueryResult(err);
	}
	return stt;
}

int EdMdbQuery::queryContinue(int waitevt)
{
	dbgd("query continue, arg waitevt=%0x", waitevt);
	int stt;
	int err;
	stt = mysql_real_query_cont(&err, mMysql, waitevt);
	dbgd("  do query cont, waitevt=%0x, err=%d", stt, err);
	if(stt == 0)
	{
		dbgd("  query result=%d", err);
		OnQueryResult(err);
	}
	return stt;
}


void EdMdbQuery::OnQueryResult(int result)
{
}


int EdMdbQuery::fetch(int num)
{
	MYSQL_ROW row;
	MYSQL_RES* res = mysql_use_result(mMysql);
	int stt = mysql_fetch_row_start(&row, res);
	if(stt == 0)
	{
		OnFetchEnd(row, 1);
	}
	return stt;
}


void EdMdbQuery::OnFetchEnd(MYSQL_ROW row, int num)
{
}

} /* namespace edft */
