/*
 * EsHttpMsg.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: netmind
 */

#include <stdexcept>
#include "EdHttpMsg.h"

namespace edft {

EdHttpMsg::EdHttpMsg()
{
}

EdHttpMsg::~EdHttpMsg()
{
}

void EdHttpMsg::setUrl(string* url)
{
	mUrl = *url;
}

void EdHttpMsg::setStatusLine(string* statusline)
{
	mStatusLine = *statusline;
}

void EdHttpMsg::addHdr(string* hdr, string* val)
{
	mHeaders[*hdr] = *val;
}

void EdHttpMsg::addHdr(const char* name, const char* val)
{
	mHeaders[name] = val;
}

const char* EdHttpMsg::getHdr(const char* name)
{
	try {
		string &val = mHeaders.at(name);
		return val.c_str();
	} catch(out_of_range &exp) {
		return NULL;
	}
}

void EdHttpMsg::encodeRespMsg(string* outbuf)
{
	*outbuf += mStatusLine;

	unordered_map<string, string>::iterator it;
	for(it = mHeaders.begin();it != mHeaders.end(); it++) {
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


const string* EdHttpMsg::getUrl()
{
	return &mUrl;
}

} // namespace edft
