/*
 * EdFile.h
 *
 *  Created on: Jan 14, 2014
 *      Author: khkim
 */

#ifndef EDFILE_H_
#define EDFILE_H_

#include <utility>
#include <string>
#include "EdFile.h"
#include <stdarg.h>
#include <fcntl.h>
#include "EdType.h"

namespace edft
{

class EdFile
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
	EdFile(const char* path, int flags=OPEN_READ, u32 mode=0);
	EdFile();
	virtual ~EdFile();

	int openFile(const std::string &path, int flags=OPEN_READ, u32 mode=0);

	int readFile(void* buf, int size);
	int writeFile(const void* buf, int size);
	int writeStr(const char* str);
	int writeStrFormat(const char* fmt, ...);

	u64 seek(u64 offset);
	u64 seek(u64 offset, u64 ref);
	int getFd(void);
	void closeFile(void);
	static long getSize(const std::string &path);
	static std::pair<std::string, std::string> splitFileName(std::string &fname);

private:
	int mFd;
};

} /* namespace edft */

#endif /* EDFILE_H_ */
