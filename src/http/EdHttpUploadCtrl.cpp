/*
 * EdHttpUploadCtrl.cpp
 *
 *  Created on: Oct 8, 2014
 *      Author: netmind
 */
#include "../config.h"

#define DBGTAG "UPCTR"
#define DBG_LEVEL DBG_WARN

#include "EdHttpUploadCtrl.h"

namespace edft
{

EdHttpUploadCtrl::EdHttpUploadCtrl()
{
	mWriter = NULL;
}

EdHttpUploadCtrl::~EdHttpUploadCtrl()
{
	CHECK_DELETE_OBJ(mWriter);
}

void EdHttpUploadCtrl::OnHttpRequestHeader()
{
	dbgd("upfile request,...");
	mWriter = new EdHttpFileWriter;
	if(mPath.size()>0)
		mWriter->open(mPath.c_str());
}

void EdHttpUploadCtrl::OnHttpDataNew(EdHttpContent *pctt)
{
	dbgd("upfile data new ...");
}
void EdHttpUploadCtrl::OnHttpDataContinue(EdHttpContent *pctt, const void *buf, int len)
{
	dbgd("upfile data continue, len=%d", len);
	mWriter->writeData(buf, len);
}

void EdHttpUploadCtrl::OnHttpDataRecvComplete(EdHttpContent *pctt)
{
	dbgd("upfile data complete ...");
	mWriter->close();
	CHECK_DELETE_OBJ(mWriter);
}


void EdHttpUploadCtrl::setPath(const char *path)
{
	mPath = path;
}

} /* namespace edft */
