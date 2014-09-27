/*
 * EsHttpMsg.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: netmind
 */

#include <stdexcept>
#include "EsHttpMsg.h"

namespace edft {

EsHttpMsg::EsHttpMsg()
{
}

EsHttpMsg::~EsHttpMsg()
{
}

void EsHttpMsg::setUrl(string* url)
{
	mUrl = *url;
}

void EsHttpMsg::setStatusLine(string* statusline)
{
	mStatusLine = *statusline;
}

void EsHttpMsg::addHdr(string* hdr, string* val)
{
	mHeaders[*hdr] = *val;
}

void EsHttpMsg::addHdr(const char* name, const char* val)
{
	mHeaders[name] = val;
}

const char* EsHttpMsg::getHdr(const char* name)
{
	try {
		string &val = mHeaders.at(name);
		return val.c_str();
	} catch(out_of_range &exp) {
		return NULL;
	}
}

void EsHttpMsg::encodeRespMsg(string* outbuf)
{
	*outbuf += mStatusLine;

	unordered_map<string, string>::iterator it;
	for(it = mHeaders.begin();it != mHeaders.end(); it++) {
		*outbuf += (it->first + ": " + it->second + "\r\n");
	}

	*outbuf += "\r\n";
}

void EsHttpMsg::free()
{
	mHeaders.clear();
	mStatusLine = "";
	mUrl = "";
}


const string* EsHttpMsg::getUrl()
{
	return &mUrl;
}

} // namespace edft
