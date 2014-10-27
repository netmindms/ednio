/*
 * EsHttpMsg.h
 *
 *  Created on: Jul 7, 2014
 *      Author: netmind
 */

#ifndef ESHTTPMSG_H_
#define ESHTTPMSG_H_

#include "../ednio_config.h"

#include <string>
#include <unordered_map>

using namespace std;

namespace edft {


class EdHttpMsg
{
	friend class EsHttpTrans;
public:
	EdHttpMsg();
	virtual ~EdHttpMsg();

	void setUrl(string url);
	string getUrl();
	void setStatusLine(string *statusline);
	void addHdr(string hdr, string val);
	void addHdr(const char* name, const char* val);
	const char* getHdr(const char* name);
	const string getHdrString(const char* name);
	void encodeRespMsg(string *outbuf);

	void free();

private:
	unordered_map<string, string> mHeaders;
	string mStatusLine;
	string mUrl;
};

}

#endif /* ESHTTPMSG_H_ */
