/*
 * EdHdrContentType.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */
#include "../ednio_config.h"

#include "EdHdrContentType.h"

namespace edft
{

EdHdrContentType::EdHdrContentType()
{
}

EdHdrContentType::~EdHdrContentType()
{
}

void EdHdrContentType::OnTag(const char* name, int nlen, const char* val, int vlen)
{
	if(mType.size()==0)
	{
		mType.append(name, nlen);
	}
	else
	{
		mParaName.append(name, nlen);
		mParaVal.append(val, vlen);
	}
}

const char* EdHdrContentType::getType()
{
	return mType.c_str();
}

const char* EdHdrContentType::getParam(const char* name)
{
	if(!mParaName.compare(name))
	{
		return mParaVal.c_str();
	}
	else
		return NULL;
}


void EdHdrContentType::parse(const char* name, int len)
{
	parseGeneral(name, len);
}

} /* namespace edft */
