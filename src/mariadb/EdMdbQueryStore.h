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

class EdMdbQueryStore : public EdMdbQueryBase
{
public:
	EdMdbQueryStore();
	virtual ~EdMdbQueryStore();
	virtual void OnQueryEnd(MYSQL_RES *res);
private:
	int queryStart(const char *qs);
	int queryContinue(int waitevt);
	void setConnection(EdMdbCnn* pcnn);
	int startStore();

private:
	EdMdbCnn* mCnn;
	MYSQL* mMysql;
	int mStatus; // 1: querying, 2: storing
};

} /* namespace edft */

#endif /* EDMDBQUERYSTORE_H_ */
