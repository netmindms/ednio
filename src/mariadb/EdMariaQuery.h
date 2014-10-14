/*
 * EdMariaQuery.h
 *
 *  Created on: Oct 12, 2014
 *      Author: netmind
 */

#ifndef EDMARIAQUERY_H_
#define EDMARIAQUERY_H_

#include <string>
#include <mysql.h>

using namespace std;

namespace edft
{
class EdMariaCnn;

class EdMariaQuery
{
public:
	EdMariaQuery(const char* qr);
	virtual ~EdMariaQuery();
	virtual void queryContinue(EdMariaCnn* pcnn, int waitevt);
	void setQuery(const char* str);
	int queryStore(const char* query);
	const char* getQueryString();

private:
	EdMariaCnn* mCnn;
	MYSQL *mMySql;
	string mQuery;
};

} /* namespace edft */

#endif /* EDMARIAQUERY_H_ */
