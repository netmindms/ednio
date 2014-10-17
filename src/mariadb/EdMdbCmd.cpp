/*
 * EdMdbCmd.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#define DBGTAG "MYCMD"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdMdbCnn.h"
#include "EdMdbCmd.h"

namespace edft
{

EdMdbCmd::EdMdbCmd(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
	mStatus = STATUS_INIT;
	mOpStatus = OP_QUERY;
	mMysql = NULL;
	mRes = NULL;
	mMaxFetch = 0;
}

EdMdbCmd::~EdMdbCmd()
{
}

void EdMdbCmd::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
}

int EdMdbCmd::queryStart(const char* qs)
{
	return -1;
}

int EdMdbCmd::queryContinue(int waitevt)
{
	dbgd("query continue, waitevt=%d, op_status=%d", waitevt, mOpStatus);
	int stt, err;
	if (mOpStatus == OP_QUERY)
	{
		stt = mysql_real_query_cont(&err, mMysql, waitevt);
		if (stt == 0)
		{
			dbgd("  query end, err=%d", err);
			mOpStatus = OP_IDLE;
			OnQueryResult(err);
		}
		return stt;
	}
	else if (mOpStatus == OP_FETCH)
	{
		MYSQL_ROW row;
		stt = mysql_fetch_row_cont(&row, mRes, waitevt);
		if (stt == 0)
		{
			mOpStatus = OP_IDLE;
			OnFetchRow(row);
		}
		return stt;
	}
#if __MULTIROW
	else if(mOpStatus == OP_GETMULTIROWS)
	{

	}
#endif
	else
	{
		dbge("### unknown status=%d", mStatus);
		assert(0);
	}
	return 0;
}

void EdMdbCmd::OnQueryResult(int err)
{
}

void EdMdbCmd::OnFetchRow(MYSQL_ROW row)
{
}

int EdMdbCmd::query(const char* qs, int* err)
{
//	if(mStatus != STATUS_INIT)
//	{
//		*err = 1;
//		return 0;
//	}
	dbgd("query, qs=%s", qs);

	mMysql = mCnn->getMysql();
	mCnn->setQuery(this);

	int stt;
	stt = mysql_real_query_start(err, mMysql, qs, strlen(qs));
	if (stt != 0)
	{
		dbgd("  query continue expected...");
		mOpStatus = OP_QUERY;
		mCnn->changeWaitEvent(stt);
		return MDB_CONTINUE;
	}
	else
	{
		dbgd("  early query end...");
		mOpStatus = OP_IDLE;
		mStatus = STATUS_QUERY;
		return MDB_COMPLETE;
	}
}

int EdMdbCmd::fetchRow(MYSQL_ROW* row)
{
	int stt;
	if (mRes == NULL)
	{
		mRes = mysql_use_result(mMysql);
		if (mRes == NULL)
		{
			return MDB_COMPLETE;
		}
	}
	stt = mysql_fetch_row_start(row, mRes);
	if (stt != 0)
	{
		mOpStatus = OP_FETCH;
		mCnn->changeWaitEvent(stt);
		return MDB_CONTINUE;
	}
	else
	{
		dbgd("  early fetch end...");
		mOpStatus = OP_IDLE;
		return MDB_COMPLETE;
	}
}

void EdMdbCmd::close()
{
	if (mRes != NULL)
	{
		mysql_free_result(mRes);
		mRes = NULL;
	}
}

#if __MULTIROW
int EdMdbCmd::getMultiRows(MDB_ROWS* rows, int maxnum)
{
	int stt;
	mMaxFetch = maxnum;
	if (mRes == NULL)
	{
		mRes = mysql_use_result(mMysql);
		if (mRes == NULL)
		{
			return MDB_COMPLETE;
		}
	}
	MYSQL_ROW eachrow;
	mdb_col_t col;
	for (;;)
	{
		stt = mysql_fetch_row_start(&eachrow, mRes);
		if (stt != 0)
		{
			mOpStatus = OP_GETMULTIROWS;
			mCnn->changeWaitEvent(stt);
			return MDB_CONTINUE;
		}
		else
		{
			dbgd("  early fetch end...");
			if (eachrow != NULL)
			{
				auto lens = mysql_fetch_lengths(mRes);
				int nc = mysql_num_fields(mRes);
				dbgd("  col_num=%d", nc);
				for (int i = 0; i < nc; i++)
				{
					dbgd("    col:%d len=%d", lens[i]);
					col.len = lens[i];
					col.buf = malloc(col.len);
					memcpy(col.buf, eachrow[i], col.len);
				}
				rows->push_back(col);
			}
			else
			{
				mOpStatus = OP_IDLE;
				return MDB_COMPLETE;
			}
		}
	}
}
#endif

} /* namespace edft */
