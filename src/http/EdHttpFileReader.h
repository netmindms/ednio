/*
 * EdHttpFileReader.h
 *
 *  Created on: Sep 27, 2014
 *      Author: netmind
 */

#ifndef EDHTTPFILEREADER_H_
#define EDHTTPFILEREADER_H_

#include "../ednio_config.h"

#include "EdHttpReader.h"
#include "../EdFile.h"

namespace edft
{

class EdHttpFileReader : public EdHttpReader
{
public:
	EdHttpFileReader();
	virtual ~EdHttpFileReader();
	int open(const char *path);
	void close();
	virtual long getSize();
	virtual long Read(void *buf, long len);

private:
	EdFile mFile;
	long mSize;
};

} /* namespace edft */

#endif /* EDHTTPFILEREADER_H_ */
