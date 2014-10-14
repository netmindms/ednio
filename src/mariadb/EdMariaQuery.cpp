/*
 * EdMariaQuery.cpp
 *
 *  Created on: Oct 12, 2014
 *      Author: netmind
 */
#if 0
#include "../config.h"

#define DBGTAG "MDBQR"

#include "../edslog.h"
#include "EdMariaQuery.h"
#include "EdMariaCnn.h"

namespace edft
{

EdMariaQuery::EdMariaQuery(const char *qr)
{
	mQuery = qr;
}

EdMariaQuery::~EdMariaQuery()
{
	// TODO Auto-generated destructor stub
}

void EdMariaQuery::queryContinue(EdMariaCnn* pcnn, int waitevt)
{
	int status;
	MYSQL_RES *res;
	status = mysql_store_result_cont(&res, pcnn->getMySql(), waitevt);
	dbgd("db storing continue, status=%0x", status);
	if (status == 0)
	{
		dbgd("db store end, ");
		//OnQueryEnd(res);
	}
	else
	{
		pcnn->changeWaitEvent(status);
	}
}

const char* EdMariaQuery::getQueryString()
{
	return mQuery.c_str();
}

} /* namespace edft */
#endif
