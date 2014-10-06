/*
 * EdHttpContent.h
 *
 *  Created on: Oct 6, 2014
 *      Author: netmind
 */

#ifndef EDHTTPCONTENT_H_
#define EDHTTPCONTENT_H_

#include <string>
#include "../EdObjList.h"
#include "EdHttpWriter.h"
#include "EdHdrContentDisposition.h"

using namespace std;

namespace edft
{


typedef struct {
	string name;
	string val;
} _hdr_t;

class EdHttpContent
{
public:
	EdHttpContent(bool multi);
	virtual ~EdHttpContent();
	const char* getName();
	const char* getFilename();
private:
	//void addHdr(const char* name, int len);
	void addHdr(string *name, string *val);
	void lookup();

private:
	char* mName;
	char* mFilename;
	bool mIsMultipart;
	EdObjList<_hdr_t> mHdrList;
	EdHttpWriter* mWriter;
	EdHdrContentDisposition* mCDisp;
};

} /* namespace edft */

#endif /* EDHTTPCONTENT_H_ */
