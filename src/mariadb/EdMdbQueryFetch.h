/*
 * EdMdbQueryFetch.h
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#ifndef EDMDBQUERYFETCH_H_
#define EDMDBQUERYFETCH_H_

#include "EdMdbCnn.h"
#include "EdMdbQueryBase.h"

namespace edft
{

class EdMdbQueryFetch : public EdMdbQueryBase
{
public:
	EdMdbQueryFetch();
	virtual ~EdMdbQueryFetch();
	virtual void OnFetchResult(MYSQL_ROW row, int num);
private:
	void setConnection(EdMdbCnn* pcnn);
	int queryStart(const char* qs);
	int queryContinue(int waitevt);
private:
	EdMdbCnn* mCnn;
	MYSQL* mMysql;
	MYSQL_RES* mRes;
};

} /* namespace edft */

#endif /* EDMDBQUERYFETCH_H_ */
