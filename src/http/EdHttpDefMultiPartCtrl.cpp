/*
 * EdHttpDefMultiPartCtrl.cpp
 *
 *  Created on: Oct 7, 2014
 *      Author: netmind
 */
#include "../config.h"

#define DBGTAG "DMCTR"
#define DBG_LEVEL DBG_DEBUG

#include <stdexcept>
#include "EdHttpDefMultiPartCtrl.h"
#include "EdHttpFileWriter.h"
#include "EdHttpStringWriter.h"

namespace edft
{

EdHttpDefMultiPartCtrl::EdHttpDefMultiPartCtrl()
{

}

EdHttpDefMultiPartCtrl::~EdHttpDefMultiPartCtrl()
{
	for(auto itr = mCttList.begin() ; itr != mCttList.end();itr++)
	{
		delete itr->second->writer;
		delete (itr->second);
	}
}

void EdHttpDefMultiPartCtrl::OnDataNew(EdHttpContent* pctt)
{
	_cinfo_t *pc = new _cinfo_t;
	pc->fileName = *(pctt->getFileName());
	if (pc->fileName.size() > 0 && mFolder.size() > 0)
	{
		EdHttpFileWriter *fwr = new EdHttpFileWriter;
		fwr->open(string(mFolder + "/" + pc->fileName).c_str());
		pc->writer = fwr;
		pctt->setWriter(fwr);
	}
	else
	{
		pc->writer = new EdHttpStringWriter;
		pctt->setWriter(pc->writer);
	}
	//pctt->setUser(pc);
	mCttList[*pctt->getName()] = pc; // TODO: duplicate check
}

void EdHttpDefMultiPartCtrl::OnDataContinue(EdHttpContent* pctt, const void* buf, int len)
{
	pctt->writer->writeData(buf, len);
}

void EdHttpDefMultiPartCtrl::OnDataRecvComplete(EdHttpContent* pct)
{
	pct->writer->close();
}

void EdHttpDefMultiPartCtrl::setFileFolder(const char* path)
{
	mFolder = path;
}

string* EdHttpDefMultiPartCtrl::getData(const char* name)
{
	try
	{
		auto pc = mCttList.at(name);
		EdHttpStringWriter* wr = (EdHttpStringWriter*) pc->writer;
		return wr->getString();
	} catch (out_of_range &err)
	{
		return NULL;
	}
}

string* EdHttpDefMultiPartCtrl::getFile(const char* name, long* plen)
{
	try
	{
		auto pc = mCttList.at("filename");
		EdHttpFileWriter *wr = (EdHttpFileWriter*)pc->writer;
		*plen = wr->getWriteCount();
		return &(pc->fileName);
	} catch (out_of_range &err)
	{
		*plen = 0;
		return NULL;
	}
}




} /* namespace edft */
