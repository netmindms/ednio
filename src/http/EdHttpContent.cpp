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
	mName = NULL;
	mFilename = NULL;
	mIsMultipart = multi;
	mWriter = NULL;
	mCDisp = NULL;
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

const char* EdHttpContent::getName()
{
	return mName;
}

const char* EdHttpContent::getFilename()
{
	return mFilename;
}

void EdHttpContent::addHdr(string *pname, string *pval)
{
	_hdr_t *ph = mHdrList.allocObj();
	ph->name = *pname;
	ph->val = *pval;
	mHdrList.push_back(ph);
	if(mCDisp==NULL && strcasecmp(pname->c_str(), "Content-Disposition") ) {
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

} /* namespace edft */
