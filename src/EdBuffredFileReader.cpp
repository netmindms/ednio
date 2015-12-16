/*
 * EdBuffredFileReader.cpp
 *
 *  Created on: Sep 29, 2014
 *      Author: netmind
 */

#include "ednio_config.h"

#define DBGTAG "bffrd"
#define DBG_LEVEL DBG_WARN

#include "edslog.h"
#include "EdBuffredFileReader.h"

namespace edft
{

EdBuffredFileReader::EdBuffredFileReader()
{
	mBlockSize = 0;
	mQSize = 0;
	mIsScheduled = false;
	mEof = false;
}

EdBuffredFileReader::~EdBuffredFileReader()
{
}


int EdBuffredFileReader::open(const char* path, int block_unit, int qsize)
{
	int ret = mFile.openFile(path);
	if (ret < 0)
	{
		return -1;
	}
	mBlockSize = block_unit * 4 * 1024;
	mQSize = qsize;

	mJobEvent.setOnListener([this](EdEventFd &event, int cnt)
	{
		dbgd("on load job ...");
		mIsScheduled = false;
		int ret = loadData();
		if(mBufList.size() < mQSize && ret > 0)
		{
			event.raise();
			mIsScheduled = true;
		}
	});
	mJobEvent.open();
	return 0;

}

void EdBuffredFileReader::close()
{
	mJobEvent.close();
	mFile.closeFile();

	EdBufferInfo* pinfo;
	for (;;)
	{
		pinfo = mBufList.pop_front();
		if (pinfo != NULL)
		{
			free(pinfo->buf);
		}
		else
		{
			break;
		}
	}
}

void EdBuffredFileReader::getData(EdBufferInfo* pbufinfo)
{
	if (mBufList.size() == 0)
	{
		dbgd("no buffer, start loading...");
		loadData();
	}

	EdBufferInfo* pbf = mBufList.pop_front();
	if (pbf != NULL)
	{
		pbufinfo->buf = pbf->buf;
		pbufinfo->size = pbf->size;
		pbufinfo->takeBuffer = true;
		mBufList.freeObj(pbf);

	}
	else
	{
		pbufinfo->buf = NULL;
		pbufinfo->size = 0;
		pbufinfo->takeBuffer = false;
	}

	if (mBufList.size() <= mQSize && mEof == false)
	{
		mJobEvent.raise();
	}
}

int EdBuffredFileReader::loadData()
{

	EdBufferInfo* pbf = mBufList.allocObj();
	pbf->buf = malloc(mBlockSize);
	int rcnt = mFile.readFile(pbf->buf, mBlockSize);
	if (rcnt > 0)
	{
		pbf->size = rcnt;
		mBufList.push_back(pbf);
		return rcnt;
	}
	else
	{
		free(pbf->buf);
		mBufList.freeObj(pbf);
		mEof = true;
		return 0;
	}
}

} /* namespace edft */
