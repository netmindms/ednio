/*
 * EdHttpContent.cpp
 *
 *  Created on: Oct 6, 2014
 *      Author: netmind
 */

#include "EdHttpContent.h"

namespace edft
{

EdHttpContent::EdHttpContent(bool multi)
{
	//mName = NULL;
	//mFilename = NULL;
	mIsMultipart = multi;
	mCDisp = NULL;
	uobj = NULL;
}

EdHttpContent::~EdHttpContent()
{
	_hdr_t *ph;
	for(;;)
	{
		ph = mHdrList.pop_front();
		if(ph == NULL)
			break;
		mHdrList.freeObj(ph);
	}
}

string* EdHttpContent::getName()
{
	return mCDisp->getName();
}

string* EdHttpContent::getFileName()
{
	return mCDisp->getFileName();
}


void EdHttpContent::addHdr(string *pname, string *pval)
{
	_hdr_t *ph = mHdrList.allocObj();
	ph->name = *pname;
	ph->val = *pval;
	mHdrList.push_back(ph);
	if(mCDisp==NULL && !strcasecmp(pname->c_str(), "Content-Disposition") ) {
		mCDisp = new EdHdrContentDisposition;
		mCDisp->parse(pval->c_str(), pval->size());
	}
}


void EdHttpContent::lookup()
{
	_hdr_t *ph;
	for(ph=mHdrList.front(); ph != NULL;)
	{
		ph = mHdrList.next(ph);
	}
}


bool EdHttpContent::isValidMp()
{
	if(mCDisp && mCDisp->getName() != NULL)
		return true;
	else
		return false;
}


void EdHttpContent::setUser(void* obj)
{
	uobj = obj;
}

void EdHttpContent::setUser(uint64_t ldata)
{
	uldata = ldata;
}

void EdHttpContent::setUser(uint32_t wdata)
{
	uwdata = wdata;
}

void* EdHttpContent::getUserObj()
{
	return uobj;
}

uint64_t EdHttpContent::getUserLong()
{
	return uldata;
}

uint32_t EdHttpContent::getUserInt()
{
	return uwdata;
}

} /* namespace edft */
