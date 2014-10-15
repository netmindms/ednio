/*
 * EdMdbQuery.h
 *
 *  Created on: Oct 15, 2014
 *      Author: netmind
 */

#ifndef EDMDBQUERY_H_
#define EDMDBQUERY_H_

#include <mysql.h>
#include "EdMdbQueryBase.h"

namespace edft
{

class EdMdbQuery: public EdMdbQueryBase
{

public:
	EdMdbQuery();
	virtual ~EdMdbQuery();
	virtual void OnQueryResult(int result);

private:
	void setConnection(EdMdbCnn* pcnn);
	int queryStart(const char* qs);
	int queryContinue(int waitevt);
private:
	EdMdbCnn* mCnn;
	MYSQL* mMysql;
};

} /* namespace edft */

#endif /* EDMDBQUERY_H_ */
