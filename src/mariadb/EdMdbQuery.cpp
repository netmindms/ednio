/*
 * EdMdbCmd.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#define DBGTAG "MYQRY"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdMdbCnn.h"
#include "EdMdbQuery.h"

namespace edft
{

EdMdbQuery::EdMdbQuery(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
	mEarlyEvent = NULL;
	mEarlyRow = NULL;
	mQueryErr = 0;
	mOpStatus = OP_IDLE;
	mMysql = NULL;
	mRes = NULL;
	mMaxFetch = 0;
}

EdMdbQuery::~EdMdbQuery()
{
	close();
}

void EdMdbQuery::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
}

int EdMdbQuery::IOnQueryContinue(int waitevt)
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
			mQueryErr = 0;
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
		assert(0);
	}
	return 0;
}

void EdMdbQuery::OnQueryResult(int err)
{
}

void EdMdbQuery::OnFetchRow(MYSQL_ROW row)
{
}

int EdMdbQuery::query(const char* qs, int *perr)
{

	dbgd("query, qs=%s", qs);
	if (mOpStatus != OP_IDLE)
	{
		dbgd("*** Fail: query busy...");
		return MDB_FAIL;
	}

	mMysql = mCnn->getMysql();
	mCnn->setQuery(this);

	int stt;
	stt = mysql_real_query_start(&mQueryErr, mMysql, qs, strlen(qs));
	mOpStatus = OP_QUERY;
	if (stt != 0)
	{
		dbgd("  query continue expected...");
		mCnn->changeWaitEvent(stt);
		return MDB_CONTINUE;
	}
	else
	{
		dbgd("  early query end...");
		if (perr == NULL)
		{
			dbgd("    intentionally pending query end...");
			raiseEarlyEvent();
			return MDB_CONTINUE;
		}
		else
		{
			mOpStatus = OP_IDLE;
			*perr = mQueryErr;
			mQueryErr = 0;
			return MDB_COMPLETE;
		}
	}
}

int EdMdbQuery::fetchRow(MYSQL_ROW* row)
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
	stt = mysql_fetch_row_start(&mEarlyRow, mRes);
	mOpStatus = OP_FETCH;
	if (stt != 0)
	{
		mCnn->changeWaitEvent(stt);
		return MDB_CONTINUE;
	}
	else
	{
		dbgd("  early fetch end...");
		if (row != NULL)
		{
			*row = mEarlyRow;
			//dbgd("== name=%s, adr=%s", mEarlyRow[0], mEarlyRow[1]);
			//mEarlyRow=NULL;
			mOpStatus = OP_IDLE;
			return MDB_COMPLETE;
		}
		else
		{
			dbgd("    intentionally pending fetch end...");
			raiseEarlyEvent();
			return MDB_CONTINUE;
		}
	}
}

void EdMdbQuery::close()
{
	if (mMysql != NULL)
	{
		if (mEarlyEvent != NULL)
		{
			mEarlyEvent->close();
			delete mEarlyEvent;
			mEarlyEvent = NULL;
		}
		mCnn->resetQuery();
		mMysql = NULL;
		if (mRes != NULL)
		{
			//mysql_free_result(mRes);
			mCnn->freeResultSet(mRes);
			mRes = NULL;
		}
	}

}

#if __MULTIROW
int EdMdbQuery::getMultiRows(MDB_ROWS* rows, int maxnum)
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

EdMdbQuery::_EarlyQueryEvent::_EarlyQueryEvent(EdMdbQuery* qr)
{
	mMdbQuery = qr;
}

void EdMdbQuery::_EarlyQueryEvent::OnEventFd(int cnt)
{
	dbgd("early event, op_status=%d, cnt=%d", mMdbQuery->mOpStatus, cnt);
	if (mMdbQuery->mOpStatus == OP_QUERY)
	{
		int err = mMdbQuery->mQueryErr;
		mMdbQuery->mQueryErr = 0;
		mMdbQuery->OnQueryResult(err);
	}
	else if (mMdbQuery->mOpStatus == OP_FETCH)
	{
		mMdbQuery->OnFetchRow(mMdbQuery->mEarlyRow);
	}
}

void EdMdbQuery::raiseEarlyEvent()
{
	if (mEarlyEvent == NULL)
	{
		mEarlyEvent = new _EarlyQueryEvent(this);
		mEarlyEvent->open();
	}
	mEarlyEvent->raise();
}

} /* namespace edft */
