/*
 * EdHttpStringWriter.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#include "../config.h"
#include "EdHttpStringWriter.h"
namespace edft
{

EdHttpStringWriter::EdHttpStringWriter()
{

}

EdHttpStringWriter::~EdHttpStringWriter()
{

}

long EdHttpStringWriter::writeData(void* buf, long len)
{
	mString.append((char*)buf, len);
	return len;
}

string* EdHttpStringWriter::getString()
{
	return &mString;
}

} /* namespace edft */
