/*
 * EdMdbQueryFetch.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#include "EdMdbQueryFetch.h"

namespace edft
{

EdMdbQueryFetch::EdMdbQueryFetch()
{
	mCnn = NULL;
}

EdMdbQueryFetch::~EdMdbQueryFetch()
{
	// TODO Auto-generated destructor stub
}

void EdMdbQueryFetch::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
	mMysql = pcnn->getMysql();
}

int EdMdbQueryFetch::queryStart(const char* qs)
{
	MYSQL_ROW row;
	MYSQL_RES* res = mysql_use_result(mMysql);
	int stt = mysql_fetch_row_start(&row, res);
	if(stt == 0 && res)
	{
		OnFetchResult(row, 1);
		mysql_free_result(&row);
	}
	return stt;
}

void EdMdbQueryFetch::OnFetchResult(MYSQL_ROW row, int num)
{
}

int EdMdbQueryFetch::queryContinue(int waitevt)
{
	return 0;
}

} /* namespace edft */
