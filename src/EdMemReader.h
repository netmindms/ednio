/*
 * EdMemReader.h
 *
 *  Created on: Feb 2, 2015
 *      Author: netmind
 */

#ifndef EXTERNAL_EDNIO_EDMEMREADER_H_
#define EXTERNAL_EDNIO_EDMEMREADER_H_
#include "EdStreamReader.h"
namespace edft
{

class EdMemReader: public EdStreamReader
{
public:
	EdMemReader();
	virtual ~EdMemReader();
	void setBuffer(char *buf, size_t bsize);
	virtual size_t read(char* buf, size_t) override;
	virtual size_t remain() override;

private:
	char *mBuf;
	size_t mSize;
	size_t mReadCnt;
};

} /* namespace edft */

#endif /* EXTERNAL_EDNIO_EDMEMREADER_H_ */
