/*
 * EdHttpFileReader.cpp
 *
 *  Created on: Sep 27, 2014
 *      Author: netmind
 */

#include "../ednio_config.h"

#include "EdHttpFileReader.h"

namespace edft
{

EdHttpFileReader::EdHttpFileReader()
{
	mSize = 0;
}

EdHttpFileReader::~EdHttpFileReader()
{
}

int EdHttpFileReader::open(const char* path)
{
	mSize = mFile.getSize(path);
	return mFile.openFile(path);
}

void EdHttpFileReader::close()
{
	mFile.closeFile();
}

long EdHttpFileReader::getSize()
{
	return mSize;
}

long EdHttpFileReader::Read(void* buf, long len)
{
	return mFile.readFile(buf, len);
}

} /* namespace edft */
