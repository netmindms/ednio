/*
 * EdHttpStringReader.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPSTRINGREADER_H_
#define EDHTTPSTRINGREADER_H_
#include "../ednio_config.h"

#include <string>
#include "EdHttpReader.h"
using namespace std;

namespace edft
{

class EdHttpStringReader : public EdHttpReader
{
public:
	EdHttpStringReader();
	virtual ~EdHttpStringReader();
	virtual long Read(void *buf, long len);
	virtual long getSize();
	void setString(string *s);
	void setString(const char* ptr);
	void takeString(string *s);

private:
	void* mBuf;
	long mReadIdx;
	long mSize;
	string *mTakenString;
};

} /* namespace edft */

#endif /* EDHTTPSTRINGREADER_H_ */
