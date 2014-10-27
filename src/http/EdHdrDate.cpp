/*
 * EdHdrDate.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */

#include "../ednio_config.h"

#define DBGTAG "HDRDA"
#define DBG_LEVEL DBG_WARN

#include "../edslog.h"
#include "EdHdrDate.h"

namespace edft
{

EdHdrDate::EdHdrDate()
{
	mItemCount = 0;
}

EdHdrDate::~EdHdrDate()
{
}

void EdHdrDate::OnTag(const char* name, int nlen, const char* val, int vlen)
{
	if (mItemCount < HEADER_DATE_INFO_COUNT)
	{
		mDateData[mItemCount].append(name, nlen);
		dbgd("date tag=%s", mDateData[mItemCount].c_str());
		mItemCount++;
	}
}

void EdHdrDate::parse(const char* str, int len)
{
	parseDate(str, len);
}

} /* namespace edft */
