/*
 * EdHttpFileWriter.cpp
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */

#include "../config.h"

#define DBGTAG "FILWR"
#define DBG_LEVEL DBG_WARN

#include "../edslog.h"
#include "EdHttpFileWriter.h"

namespace edft
{

EdHttpFileWriter::EdHttpFileWriter()
{
	dbgd("const file writer ...");
	mWriteCnt = 0;
}

EdHttpFileWriter::~EdHttpFileWriter()
{
	dbgd("dest file writer ...");
}

int EdHttpFileWriter::open(const char* path)
{
	return mFile.openFile(path, EdFile::OPEN_RWTC);
}

long EdHttpFileWriter::writeData(const void* buf, long len)
{
	mWriteCnt += len;
	return mFile.writeFile(buf, len);
}

long edft::EdHttpFileWriter::getWriteCount()
{
	return mWriteCnt;
}

void EdHttpFileWriter::close()
{
	mFile.closeFile();
}




} /* namespace edft */
