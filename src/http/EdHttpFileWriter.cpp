/*
 * EdHttpFileWriter.cpp
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */

#include "EdHttpFileWriter.h"

namespace edft
{

EdHttpFileWriter::EdHttpFileWriter()
{

}

EdHttpFileWriter::~EdHttpFileWriter()
{
}

int EdHttpFileWriter::open(const char* path)
{
	return mFile.openFile(path, EdFile::OPEN_RWTC);
}

long EdHttpFileWriter::writeData(const void* buf, long len)
{
	return mFile.writeFile(buf, len);
}

void EdHttpFileWriter::close()
{
	mFile.closeFile();
}

} /* namespace edft */
