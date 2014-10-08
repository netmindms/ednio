/*
 * EdHttpMultipartParser.h
 *
 *  Created on: Oct 5, 2014
 *      Author: netmind
 */
#if 0
#ifndef EDHTTPMULTIPARTPARSER_H_
#define EDHTTPMULTIPARTPARSER_H_

#include <unordered_map>
#include <string>
#include "EdMultipartInfo.h"
#include "MultipartParser.h"
#include "EdHttpHdr.h"

using namespace std;

namespace edft
{

class EdHttpMultipartParser: public EdMultipartInfo
{
public:
	EdHttpMultipartParser();
	virtual ~EdHttpMultipartParser();
	virtual string *getName();
	virtual string *getType();
	virtual string *getFilename();
	virtual string *getHdr(const char* name);

public:
	void init(const char* boundary);
	void feed(const char* buf, int len);

private:
	static void partBeginCb(const char *buffer, size_t start, size_t end, void *userData);
	static void headerFieldCb(const char *buffer, size_t start, size_t end, void *userData);
	static void headerValueCb(const char *buffer, size_t start, size_t end, void *userData);
	static void partDataCb(const char *buffer, size_t start, size_t end, void *userData);
	static void partEndCb(const char *buffer, size_t start, size_t end, void *userData);
	static void endCb(const char *buffer, size_t start, size_t end, void *userData);

	void dgPartBeginCb(const char *buffer, size_t start, size_t end);
	void dgHeaderFieldCb(const char *buffer, size_t start, size_t end);
	void dgHeaderValueCb(const char *buffer, size_t start, size_t end);
	void dgPartDataCb(const char *buffer, size_t start, size_t end);
	void dgPartEndCb(const char *buffer, size_t start, size_t end);
	void dgEndCb(const char *buffer, size_t start, size_t end);

private:
	MultipartParser mParser;
	string mCurHdrName;
	string mCurHdrVal;
	string *mBoundary;
	unordered_map<string, string> mHdrList;
	EdHdrContentDisposition *mMpCDisp;
	EdHdrContentType *mMpCType;
};

} /* namespace edft */

#endif /* EDHTTPMULTIPARTPARSER_H_ */
#endif
