/*
 * EdHttpMultipartParser.cpp
 *
 *  Created on: Oct 5, 2014
 *      Author: netmind
 */

#if 0
#define DBGTAG "MPPAR"
#define DBG_LEVEL DBG_DEBUG

#include <string>
#include <stdio.h>

#include "../edslog.h"
#include "../EdType.h"
#include "EdHttpMultipartParser.h"

using namespace std;

namespace edft
{

EdHttpMultipartParser::EdHttpMultipartParser()
{
	mBoundary = NULL;
	mParser.onPartBegin = partBeginCb;
	mParser.onHeaderField = headerFieldCb;
	mParser.onHeaderValue = headerValueCb;
	mParser.onPartData = partDataCb;
	mParser.onPartEnd = partEndCb;
	mParser.onEnd = endCb;
}

EdHttpMultipartParser::~EdHttpMultipartParser()
{
}

void EdHttpMultipartParser::init(const char* boundary)
{

	mParser.setBoundary(boundary);
	mParser.userData = (void*) this;
	mCurHdrName.clear();
	mCurHdrVal.clear();
}

void EdHttpMultipartParser::feed(const char* buf, int len)
{
	mParser.feed(buf, len);
}


void EdHttpMultipartParser::partBeginCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpMultipartParser *dg = (EdHttpMultipartParser*) userData;
	dg->dgPartBeginCb(buffer, start, end);
}

void EdHttpMultipartParser::headerFieldCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpMultipartParser *dg = (EdHttpMultipartParser*) userData;
	dg->dgHeaderFieldCb(buffer, start, end);

}

void EdHttpMultipartParser::headerValueCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpMultipartParser *dg = (EdHttpMultipartParser*) userData;
	dg->dgHeaderValueCb(buffer, start, end);
}

void EdHttpMultipartParser::partDataCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpMultipartParser *dg = (EdHttpMultipartParser*) userData;
	dg->dgPartDataCb(buffer, start, end);
}

void EdHttpMultipartParser::partEndCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpMultipartParser *dg = (EdHttpMultipartParser*) userData;
	dg->dgPartEndCb(buffer, start, end);
}

void EdHttpMultipartParser::endCb(const char *buffer, size_t start, size_t end, void *userData)
{
	EdHttpMultipartParser* mp = (EdHttpMultipartParser*) userData;
	mp->dgEndCb(buffer, start, end);

}

void EdHttpMultipartParser::dgPartBeginCb(const char* buffer, size_t start, size_t end)
{
	dbgd("part begine...");
}

void EdHttpMultipartParser::dgHeaderFieldCb(const char* buffer, size_t start, size_t end)
{
	//dbgd("hdr filed cb, slice=%s", string(buffer+start, end-start).c_str());
	if (mCurHdrVal.size() != 0)
	{
		dbgd("part header complete, name=%s, val=%s", mCurHdrName.c_str(), mCurHdrVal.c_str());
		mHdrList[mCurHdrName] = mCurHdrVal;
		mCurHdrName.clear();
		mCurHdrVal.clear();
	}

	if (mCurHdrName.size() == 0)
	{
		mCurHdrName.clear();
		mCurHdrName.append(buffer + start, end - start);
	}
	else
	{
		mCurHdrName.append(buffer + start, end - start);
	}
	//dbgd("onHeaderField: (%s)\n", string(buffer + start, end - start).c_str());
}

void EdHttpMultipartParser::dgHeaderValueCb(const char* buffer, size_t start, size_t end)
{

	dbgd("hdr val cb, hdr slice=%s", string(buffer+start, end-start).c_str());
//	if (mCurHdrVal.size() == 0)
//	{
//		mCurHdrVal.clear();
//		mCurHdrVal.append(buffer + start, end - start);
//	}
//	else
//	{
//		mCurHdrVal.append(buffer + start, end - start);
//	}
	mCurHdrVal.append(buffer + start, end - start);

}

void EdHttpMultipartParser::dgPartDataCb(const char* buffer, size_t start, size_t end)
{
	dbgd("part data cb...");
	if(mCurHdrName.size() > 0)
	{
		dbgd("last header complete, name=%s, val=%s", mCurHdrName.c_str(), mCurHdrVal.c_str());
		mHdrList[mCurHdrName] = mCurHdrVal;
		mCurHdrName.clear(); mCurHdrVal.clear();
	}
}

void EdHttpMultipartParser::dgPartEndCb(const char* buffer, size_t start, size_t end)
{
	dbgd("part end cb...");
}

string* EdHttpMultipartParser::getName()
{

}

string* EdHttpMultipartParser::getType()
{
}

string* EdHttpMultipartParser::getFilename()
{
}

string* EdHttpMultipartParser::getHdr(const char* name)
{
	auto itr = mHdrList.find(name);
	if(itr != mHdrList.end()) {
		return &(itr->second);
	}
	else
		return NULL;
}

void EdHttpMultipartParser::dgEndCb(const char* buffer, size_t start, size_t end)
{
	dbgd("end cb...");
	mCurHdrVal.clear();
	mCurHdrName.clear();
}

} /* namespace edft */
#endif
