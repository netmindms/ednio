/*
 * EsHttpParser.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */
#if 0
#define TAG "hpars"
#define LOG_LEVEL LOG_DEBUG

#include "eslib/calog.h"
#include "EsHttpParser.h"

EsHttpParser::EsHttpParser()
{
	memset(&mParserSettings, 0, sizeof(mParserSettings));
	mParserSettings.on_message_begin = msg_begin;
	mParserSettings.on_message_complete = msg_end;
	mParserSettings.on_url = on_url;
	mParserSettings.on_status = on_status;
	mParserSettings.on_header_field = head_field_cb;
	mParserSettings.on_header_value = head_val_cb;
	mParserSettings.on_body = body_cb;

	memset(&mParser, 0, sizeof(mParser));
}

EsHttpParser::~EsHttpParser()
{
}


int EsHttpCnn::head_field_cb(http_parser*, const char *at, size_t length)
{
//	string s(at, length);
//	pl("f: %s", s.c_str());
	if(mIsHdrVal)
	{
		dbgd("header comp, name=%s, val=%s", mCurHdrName.c_str(), mCurHdrVal.c_str());
	}
	mCurHdrName.append(at, length);
	return 0;
}

int EsHttpCnn::head_val_cb(http_parser*, const char *at, size_t length)
{
//	string s(at, length);
//	pl("v: %s", s.c_str());
	mCurHdrVal.append(at, length);
	mIsHdrVal = true;
	return 0;
}

int EsHttpCnn::body_cb(http_parser*, const char *at, size_t length)
{
	pl("body...len=%d", length);
	return 0;
}

int EsHttpCnn::msg_begin(http_parser* parser)
{
	mIsHdrVal = false;
	mCurHdrName = "";
	mCurHdrVal = "";

	return 0;
}

int EsHttpCnn::msg_end(http_parser*)
{
	pl("msg end...");
	return 0;
}

int EsHttpCnn::on_url(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

int EsHttpCnn::on_headers_complete(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*)parser->data;
	if(pcnn->mIsHdrVal)
	{
		dbgd("header comp, name=%s, val=%s", mCurHdrName.c_str(), mCurHdrVal.c_str());
		pcnn->mIsHdrVal = false;
	}
	return 0;
}

int EsHttpCnn::on_status(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

#endif
