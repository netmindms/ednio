/*
 * EdHttpFileWriter.h
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */

#ifndef EDHTTPFILEWRITER_H_
#define EDHTTPFILEWRITER_H_

#include "EdHttpWriter.h"
#include "../EdFile.h"

namespace edft
{

class EdHttpFileWriter: public EdHttpWriter
{
public:
	EdHttpFileWriter();
	virtual ~EdHttpFileWriter();
	int open(const char* path);
	long writeData(const void *buf, long len);
	long getWriteCount();
	void close();
private:
	EdFile mFile;
	long mWriteCnt;
};

} /* namespace edft */

#endif /* EDHTTPFILEWRITER_H_ */
