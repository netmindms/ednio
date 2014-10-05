/*
 * EdHdrContentDisposition.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */

#include "../config.h"

#include <string.h>
#include "EdHdrContentDisposition.h"

namespace edft
{

EdHdrContentDisposition::EdHdrContentDisposition()
{

}

EdHdrContentDisposition::~EdHdrContentDisposition()
{
}

void EdHdrContentDisposition::OnTag(const char* tname, int nlen, const char* tval, int vlen)
{
	if (vlen == 0 && mDesc.size() == 0)
	{
		mDesc.append(tname, nlen);
	}
	else if (!memcmp(tname, BOUNDARY_TAG, sizeof(BOUNDARY_TAG)))
	{
		mBoundary.append(tval, vlen);
	}
}

void EdHdrContentDisposition::parse(const char* str, int len)
{
	parseGeneral(str, len);
}

const char* EdHdrContentDisposition::getBoundary()
{
	return mBoundary.c_str();
}

} /* namespace edft */
