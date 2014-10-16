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
	mMysql = NULL;
	mRes = NULL;
}

EdMdbQueryFetch::~EdMdbQueryFetch()
{
	if (mRes != NULL)
	{
		mysql_free_result(mRes);
	}
}

void EdMdbQueryFetch::setConnection(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
	mMysql = pcnn->getMysql();
}

int EdMdbQueryFetch::queryStart(const char* qs)
{
	MYSQL_ROW row;
	MYSQL_RES *res = mysql_use_result(mMysql);
	int stt = mysql_fetch_row_start(&row, res);
	if (stt == 0)
	{
		OnFetchResult(row, 1);
	}
	return stt;
}


int EdMdbQueryFetch::queryContinue(int waitevt)
{
	int stt;
	MYSQL_ROW row;
	MYSQL_RES *res = mysql_use_result(mMysql);
	stt = mysql_fetch_row_cont(&row, res, waitevt);
	if (stt == 0)
	{
		OnFetchResult(row, 1);
	}
	return stt;
}


void EdMdbQueryFetch::OnFetchResult(MYSQL_ROW row, int num)
{
}


} /* namespace edft */
