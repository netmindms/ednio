/*
 * EdMariaCnn.h
 *
 *  Created on: Oct 11, 2014
 *      Author: netmind
 */

#ifndef EDMARIACNN_H_
#define EDMARIACNN_H_

#include <mysql/mysql.h>
#include "../EdEvent.h"
#include "../EdTimer.h"

namespace edft
{

enum {
	DB_OP_IDLE,
	DB_OP_CONNECTING,
	DB_OP_QUERYING,
	DB_OP_STORE,
	DB_OP_FETCHING,

	DB_OP_MAX,
};

class EdMariaCnn;
class EdMariaQuery;

typedef void (*CONTPROC)(EdMariaCnn* pcnn, int waitevt);

class EdMariaCnn : public EdEvent, public EdTimer::ITimerCb
{
	friend class EdMariaQuery;

public:
	EdMariaCnn();
	virtual ~EdMariaCnn();
	virtual void OnEventRead();
	virtual void OnEventWrite();
	virtual void OnEventHangup();

	virtual void OnDbConnected();
	virtual void OnQueryEnd(MYSQL_RES* res);

	virtual void IOnTimerEvent(EdTimer* ptimer);
	int connect(const char* ip, int port, const char* id, const char* pw, const char* dbname);
	int sqlQueryAndStore(const char* query);
	int sqlQueryAndFetch(const char* query);
	MYSQL* getMySql();
	int runQuery(EdMariaQuery* qr);

private:
	void procCnnCont(int  waitevt);
	void procQueryCont(int waitevt);
	void procStoreCont(int waitevt);
	void changeWaitEvent(int waitevt);

	static void _procCnnCont(EdMariaCnn* pcnn, int waitevt);
	static void _procQueryCont(EdMariaCnn* pcnn, int waitevt);

private:
	MYSQL* mDbCnn;
	EdTimer mTimer;
	int mCnnStatus;
	int mOpStatus;
	CONTPROC mContProc;
	EdMariaQuery* mQuery;
};

} /* namespace edft */

#endif /* EDMARIACNN_H_ */
