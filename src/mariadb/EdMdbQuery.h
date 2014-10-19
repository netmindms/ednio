/*
 * EdMdbCmd.h
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#ifndef EDMDBCMD_H_
#define EDMDBCMD_H_

#include <string>
#include <mysql.h>

#include "../EdEventFd.h"
#include "EdMdbType.h"
#include "EdMdbQueryBase.h"
using namespace std;

namespace edft
{
class EdMdbCnn;

class EdMdbQuery: public EdMdbQueryBase
{
private:
	enum
	{
		STATUS_INIT = 0, STATUS_QUERY,
	};

	enum
	{
		OP_IDLE = 0, OP_QUERY, OP_FETCH,
#if __MULTIROW
	//OP_GETMULTIROWS,
#endif
	};

	class _EarlyQueryEvent: public EdEventFd
	{
		friend class EdMdbQuery;
		EdMdbQuery *mMdbQuery;
		_EarlyQueryEvent(EdMdbQuery* qr);
		void OnEventFd(int cnt);
	};

public:
	EdMdbQuery(EdMdbCnn* pcnn);
	virtual ~EdMdbQuery();
	void setConnection(EdMdbCnn* pcnn);

	virtual void OnQueryResult(int err);
	virtual void OnFetchRow(MYSQL_ROW row);
	int query(const char* qs, int *perr=NULL);
	void close();
	int fetchRow(MYSQL_ROW* row=NULL);

#if __MULTIROW // TODO
	int getMultiRows(MDB_ROWS *rows, int maxnum);
#endif

private:
	virtual int IOnQueryContinue(int waitevt);
	void raiseEarlyEvent();

private:
	string mCmd;
	EdMdbCnn *mCnn;
	_EarlyQueryEvent *mEarlyEvent;
	MYSQL* mMysql;
	MYSQL_RES* mRes;
	MYSQL_ROW mEarlyRow;
	int mOpStatus;
	int mMaxFetch;
	int mQueryErr;

};

} /* namespace edft */

#endif /* EDMDBCMD_H_ */
