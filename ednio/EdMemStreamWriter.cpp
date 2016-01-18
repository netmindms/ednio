/*
 * EdMemStreamWriter.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: netmind
 */

#include <string.h>
#include <algorithm>
#include "EdMemStreamWriter.h"

using namespace std;

namespace edft
{

EdMemStreamWriter::EdMemStreamWriter()
{
	// TODO Auto-generated constructor stub
	mWritePos = 0;
}

EdMemStreamWriter::~EdMemStreamWriter()
{
	// TODO Auto-generated destructor stub
}

size_t EdMemStreamWriter::remain()
{
	return (mVecBuf.capacity() - mWritePos);
}

size_t EdMemStreamWriter::write(char* buf, size_t len)
{
	auto wcnt = min(len, remain());
	memcpy(mVecBuf.data()+mWritePos, buf, wcnt);
	return wcnt;
}

void EdMemStreamWriter::reserve(size_t size)
{
	mVecBuf.reserve(size);
}

} /* namespace edft */
