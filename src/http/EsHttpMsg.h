/*
 * EsHttpMsg.h
 *
 *  Created on: Jul 7, 2014
 *      Author: netmind
 */

#ifndef ESHTTPMSG_H_
#define ESHTTPMSG_H_

#include <string>
#include <unordered_map>

using namespace std;

namespace edft {

class EsHttpMsg
{
	friend class EsHttpTrans;
public:
	EsHttpMsg();
	virtual ~EsHttpMsg();

	void setUrl(string *url);
	const string* getUrl();
	void setStatusLine(string *statusline);
	void addHdr(string *hdr, string *val);
	void addHdr(const char* name, const char* val);
	const char* getHdr(const char* name);

	void encodeRespMsg(string *outbuf);

	void free();

private:
	unordered_map<string, string> mHeaders;
	string mStatusLine;
	string mUrl;
};

}

#endif /* ESHTTPMSG_H_ */
