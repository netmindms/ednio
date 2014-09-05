/*
 * EsFile.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: khkim
 */


#include "EsFile.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace edft
{

EsFile::EsFile(void)
{
	mFd = -1;
}

EsFile::EsFile(const char* path, int flags, u32 mode)
{
	mFd = openFile(path, flags, mode);
}

EsFile::~EsFile()
{
	if (mFd >= 0)
		close(mFd);
}

int EsFile::readFile(void* buf, int size)
{
	return read(mFd, buf, size);
}

int EsFile::writeFile(void* buf, int size)
{
	return write(mFd, buf, size);
}

u64 EsFile::seek(u64 offset)
{
	return lseek(mFd, offset, SEEK_SET);
}

u64 EsFile::seek(u64 offset, u64 ref)
{
	return lseek(mFd, ref, offset);
}

int EsFile::openFile(const char* path, int flags, u32 mode)
{
	if (mode == 0)
	{
		if (flags & O_CREAT)
			mode = (S_IWUSR | S_IRUSR);
	}
	mFd = open(path, flags, mode);
	return mFd;
}

int EsFile::getFd(void)
{
	return mFd;
}

void EsFile::closeFile(void)
{
	if (mFd >= 0)
	{
		close(mFd);
		mFd = -1;
	}
}

int EsFile::writeStr(const char* str)
{
	return write(mFd, str, strlen(str));
}

int EsFile::writeStrFormat(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(buf, sizeof(buf)-1, fmt, ap);
	va_end(ap);
	return writeFile((u8*)buf, size);
}

} /* namespace tevt */
