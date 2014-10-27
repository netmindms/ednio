/*
 * EdHdrContentDisposition.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */

#include "../ednio_config.h"

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
	else if (mName.size()==0 && !memcmp(tname, "name", 4))
	{
		mName.append(tval, vlen);
	}
	else if(mFileName.size() ==0 && !memcmp(tname, "filename", 8))
	{
		mFileName.append(tval, vlen);
	}
}

void EdHdrContentDisposition::parse(const char* str, int len)
{
	parseGeneral(str, len);
}

string* EdHdrContentDisposition::getName()
{
	return &mName;
}

string* EdHdrContentDisposition::getFileName()
{
	return &mFileName;
}

} /* namespace edft */
