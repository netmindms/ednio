/*
 * EdMdbQueryStore.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: netmind
 */

#include "EdMdbCnn.h"
#include "EdMdbQueryStore.h"

namespace edft
{

EdMdbQueryStore::EdMdbQueryStore()
{
	// TODO Auto-generated constructor stub

}

EdMdbQueryStore::~EdMdbQueryStore()
{
	// TODO Auto-generated destructor stub
}

int EdMdbQueryStore::query(const char* qs)
{
	mMysql = mCnn->getMysql();
	int ret;
	int status = mysql_real_query_start(&ret, mMysql, qs, strlen(qs));
	return status;
}

int EdMdbQueryStore::queryContinue(int waitevt)
{
	int stt;
	MYSQL_RES* res;
	if (mStatus == 1)
	{
		int ret;
		stt = mysql_real_query_cont(&ret, mMysql, waitevt);
		if (stt == 0)
		{
			mStatus = 2;
			stt = mysql_store_result_start(&res, mMysql);
			assert(res == NULL);
		}
	}
	else if (mStatus == 2)
	{
		stt = mysql_store_result_cont(&res, mMysql, waitevt);
		if (stt == 0)
		{
			mStatus = 0;
			OnQueryEnd(res);
			mysql_free_result(res);
		}
	}

	return stt;

}

void EdMdbQueryStore::OnQueryEnd(MYSQL_RES* res)
{
}


void EdMdbQueryStore::setCnn(EdMdbCnn* pcnn)
{
	mCnn = pcnn;
}

} /* namespace edft */
