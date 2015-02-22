/*
 * EdFile.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: khkim
 */

#include "EdFile.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace edft
{

EdFile::EdFile(void)
{
	mFd = -1;
}

EdFile::EdFile(const char* path, int flags, u32 mode)
{
	mFd = openFile(path, flags, mode);
}

EdFile::~EdFile()
{
	if (mFd >= 0)
		close(mFd);
}

int EdFile::readFile(void* buf, int size)
{
	return read(mFd, buf, size);
}

int EdFile::writeFile(const void* buf, int size)
{
	return write(mFd, buf, size);
}

u64 EdFile::seek(u64 offset)
{
	return lseek(mFd, offset, SEEK_SET);
}

u64 EdFile::seek(u64 offset, u64 ref)
{
	return lseek(mFd, ref, offset);
}

int EdFile::openFile(const string &path, int flags, u32 mode)
{
	if (mode == 0)
	{
		if (flags & O_CREAT)
			mode = (S_IWUSR | S_IRUSR);
	}
	mFd = open(path.c_str(), flags, mode);
	return mFd;
}

int EdFile::getFd(void)
{
	return mFd;
}

void EdFile::closeFile(void)
{
	if (mFd >= 0)
	{
		close(mFd);
		mFd = -1;
	}
}

int EdFile::writeStr(const char* str)
{
	return write(mFd, str, strlen(str));
}

int EdFile::writeStrFormat(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);
	return writeFile((u8*) buf, size);
}

long EdFile::getSize(const string &path)
{
	struct stat st;
	int ret = stat(path.c_str(), &st);
	if(!ret)
		return st.st_size;
	else
		return -1;
}


pair<string, string> EdFile::splitFileName(string& fname)
{
	pair<string, string> rv;
	size_t idx = fname.find_last_of('.');
	if(idx != string::npos) {
		rv.second = fname.substr(idx+1);
		rv.first = fname.substr(0, idx);
		return rv;
	} else {
		rv.first = fname;
		return rv;
	}
}

} /* namespace edft */
