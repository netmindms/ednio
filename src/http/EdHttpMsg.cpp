/*
 * EsHttpMsg.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: netmind
 */
#include "../ednio_config.h"

#define DBGTAG "HTMSG"
#define DBG_LEVEL DBG_WARN

#include <stdexcept>
#include "../edslog.h"
#include "EdHttpMsg.h"

namespace edft
{

EdHttpMsg::EdHttpMsg()
{
}

EdHttpMsg::~EdHttpMsg()
{
}

void EdHttpMsg::setUrl(string url)
{
	mUrl = url;
}

void EdHttpMsg::setStatusLine(string* statusline)
{
	mStatusLine = *statusline;
}

void EdHttpMsg::addHdr(string hdr, string val)
{
	mHeaders[hdr] = val;
}

void EdHttpMsg::addHdr(const char* name, const char* val)
{
	mHeaders[name] = val;
}

const char* EdHttpMsg::getHdr(const char* name)
{
#if 1
	return getHdrString(name).c_str();
//	const string ps = getHdrString(name);
//	if(ps !=NULL) {
//		return ps->c_str();
//	} else {
//		return NULL;
//	}

#else
	try
	{
		string &val = mHeaders.at(name);
		return val.c_str();
	} catch (out_of_range &exp)
	{
		return NULL;
	}
#endif
}

const string EdHttpMsg::getHdrString(const char* name)
{
	try
	{
		string &val = mHeaders.at(name);
		dbgd("  req hdr val=%s", val.c_str());
		return val;
	} catch (out_of_range &exp)
	{
		return "";
	}
}

void EdHttpMsg::encodeRespMsg(string* outbuf)
{
	*outbuf += mStatusLine;

	unordered_map<string, string>::iterator it;
	for (it = mHeaders.begin(); it != mHeaders.end(); it++)
	{
		*outbuf += (it->first + ": " + it->second + "\r\n");
	}

	*outbuf += "\r\n";
}

void EdHttpMsg::free()
{
	mHeaders.clear();
	mStatusLine = "";
	mUrl = "";
}

string EdHttpMsg::getUrl()
{
	return mUrl;
}

} // namespace edft
