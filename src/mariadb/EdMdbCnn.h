/*
 * EdMdbCnn.h
 *
 *  Created on: Oct 11, 2014
 *      Author: netmind
 */

#ifndef EdMdbCnn_H_
#define EdMdbCnn_H_

#include <mysql.h>
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

//class EdMdbCnn;
class EdMdbQueryBase;


class EdMdbCnn : public EdEvent
{
	friend class CnnTimer;
private:
	class CnnTimer : public EdTimer {
	public:
		CnnTimer(EdMdbCnn*);
		virtual void OnTimer();
		EdMdbCnn* mCnn;
	};
public:
	EdMdbCnn();
	virtual ~EdMdbCnn();
	virtual void OnEventRead();
	virtual void OnEventWrite();
	virtual void OnEventHangup();
	virtual void OnDbConnected();

	int connect(const char* ip, int port, const char* id, const char* pw, const char* dbname);
	MYSQL* getMysql();
	int runQuery(EdMdbQueryBase* qr, const char* qs);

private:
	void procCnnCont(int  waitevt);
	void changeWaitEvent(int waitevt);
private:
	MYSQL* mDbCnn;
	CnnTimer *mTimer;
	int mCnnStatus;
	int mOpStatus;
	EdMdbQueryBase* mQuery;
};

} /* namespace edft */

#endif /* EdMdbCnn_H_ */
