/*
 * EdMdbQueryStore.h
 *
 *  Created on: Oct 14, 2014
 *      Author: netmind
 */

#ifndef EDMDBQUERYSTORE_H_
#define EDMDBQUERYSTORE_H_

#include "EdMdbQueryBase.h"
#include "EdMdbCnn.h"

namespace edft
{

class EdMdbCnn;

class EdMdbQueryStore : public EdMdbQueryBase
{
private:
	enum {
		OP_INIT,
		OP_QUERYING,
		OP_STORING,
	};
public:
	EdMdbQueryStore(EdMdbCnn* pcnn);
	virtual ~EdMdbQueryStore();
	virtual void OnQueryEnd(MYSQL_RES *res);
	int query(const char* qs, int *perr);
	MYSQL_RES* getResult();
	MYSQL_ROW getRow();
	void close();
private:
	int queryStart(const char *qs);
	int queryContinue(int waitevt);
	void setConnection(EdMdbCnn* pcnn);
	int startStore();

private:
	EdMdbCnn* mCnn;
	MYSQL* mMysql;
	MYSQL_RES *mRes;
	int mOpStatus;
};

} /* namespace edft */

#endif /* EDMDBQUERYSTORE_H_ */
