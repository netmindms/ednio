/*
 * EdBuffredFileReader.h
 *
 *  Created on: Sep 29, 2014
 *      Author: netmind
 */

#ifndef EDBUFFREDFILEREADER_H_
#define EDBUFFREDFILEREADER_H_

#include "ednio_config.h"


#include "EdType.h"
#include "EdEventFd.h"
#include "EdFile.h"
#include "EdObjList.h"

namespace edft
{


class EdBuffredFileReader
{

public:
	EdBuffredFileReader();
	virtual ~EdBuffredFileReader();
	int open(const char* path, int block_unit, int qsize);
	void close();
	void getData(EdBufferInfo *pinfo);
private:
	int loadData();

private:
	EdFile mFile;
	int mBlockSize;
	int mQSize;
	EdEventFd mJobEvent;
	EdObjList<EdBufferInfo> mBufList;
	bool mIsScheduled;
	bool mEof;
};

} /* namespace edft */

#endif /* EDBUFFREDFILEREADER_H_ */
