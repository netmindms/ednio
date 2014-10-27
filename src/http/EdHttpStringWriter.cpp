/*
 * EdHttpStringWriter.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#include "../ednio_config.h"

#define DBGTAG "HTSWR"
#define DBG_LEVEL DBG_WARN

#include "../edslog.h"
#include "EdHttpStringWriter.h"
namespace edft
{

EdHttpStringWriter::EdHttpStringWriter()
{
	dbgd("const str writer ...");
}

EdHttpStringWriter::~EdHttpStringWriter()
{
	dbgd("dest str writer ... ");
}

long EdHttpStringWriter::writeData(const void* buf, long len)
{
	mString.append((char*)buf, len);
	return len;
}

string EdHttpStringWriter::getString()
{
	return mString;
}


long edft::EdHttpStringWriter::getWriteCount()
{
	return mString.size();
}


void edft::EdHttpStringWriter::close()
{
}

} /* namespace edft */
