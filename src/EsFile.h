/*
 * EdFile.h
 *
 *  Created on: Jan 14, 2014
 *      Author: khkim
 */

#ifndef EDFILE_H_
#define EDFILE_H_
#include "EsFile.h"
#include <stdarg.h>
#include <fcntl.h>
#include "EdType.h"


namespace edft
{

class EsFile
{
public:
	enum {
		OPEN_READ=O_RDONLY,
		OPEN_WRITE=O_RDWR,
		OPEN_CREATE=O_CREAT,
		OPEN_TRUNC=O_TRUNC,
		OPEN_RWC=(O_RDWR|O_CREAT),
		OPEN_RWT = (O_RDWR|O_TRUNC),
		OPEN_RWTC = (O_RDWR|O_TRUNC|O_CREAT),
	};
public:
	EsFile(const char* path, int flags=OPEN_READ, u32 mode=0);
	EsFile();
	virtual ~EsFile();

	int openFile(const char* path, int flags=OPEN_READ, u32 mode=0);

	int readFile(void* buf, int size);
	int writeFile(void* buf, int size);
	int writeStr(const char* str);
	int writeStrFormat(const char* fmt, ...);

	u64 seek(u64 offset);
	u64 seek(u64 offset, u64 ref);
	int getFd(void);
	void closeFile(void);
private:
	int mFd;
};

} /* namespace edft */

#endif /* EDFILE_H_ */
