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
	DB_OP_MAX,
};

enum {
	DB_CNN_DISCONNECTED,
	DB_CNN_CONNECTING,
	DB_CNN_CONNECTED,
};
//class EdMdbCnn;
class EdMdbQueryBase;


class EdMdbCnn : public EdEvent
{
	friend class CnnTimer;
public:
	class IMdbCnn {
	public:
		virtual void IOnMdbCnnStatus(EdMdbCnn* pcnn, int status)=0;
	};
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
	virtual void OnDbDisconnected();

	void setOnListener(IMdbCnn* onlis);
	int connectDb(const char* ip, const char* dbname=NULL, const char* id=NULL, const char* pw=NULL, int port=0);
	void disconnectDb();
	void closeDb();

	MYSQL* getMysql();
	void setQuery(EdMdbQueryBase *qr);
	int runQuery(EdMdbQueryBase* qr, const char* qs);
	void changeWaitEvent(int waitevt);

private:
	void procCnnCont(int  waitevt);
	void procQueryEnd();
	void setDbTimer();

private:
	MYSQL* mMysql;
	CnnTimer *mTimer;
	int mCnnStatus;
	int mOpStatus;
	EdMdbQueryBase* mQuery;
	IMdbCnn* mOnLis;
};

} /* namespace edft */

#endif /* EdMdbCnn_H_ */
