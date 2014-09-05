/*
 * EsHttpParser.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#if 0
#ifndef ESHTTPPARSER_H_
#define ESHTTPPARSER_H_

#include <string>
#include "eslib/http/http_parser.h"

class EsHttpParser
{
public:
	EsHttpParser();
	virtual ~EsHttpParser();
private:
	static int head_field_cb(http_parser*, const char *at, size_t length);
	int headerName(http_parser*, const char *at, size_t length);
	static int head_val_cb(http_parser*, const char *at, size_t length);
	int headerVal(http_parser*, const char *at, size_t length);
	static int on_headers_complete(http_parser *parser);
	int headerComplete(http_parser *parser);
	static int body_cb(http_parser*, const char *at, size_t length);
	static int body_cb(http_parser*, const char *at, size_t length);
	static int msg_begin(http_parser* parser);
	static int msg_end(http_parser*);
	static int on_url(http_parser *parser, const char *at, size_t length);
	static int on_status(http_parser *parser, const char *at, size_t length);

	std::string mCurHdrName, mCurHdrVal;
	bool mIsHdrVal;

	int mParsingStatus;
	http_parser mParser;
	http_parser_settings mParserSettings;
};

#endif /* ESHTTPPARSER_H_ */
#endif
