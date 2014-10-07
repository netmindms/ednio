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

typedef struct
{
	string name;
	string val;
} _hdr_t;

class EdHttpContent
{
	friend class EdHttpCnn;

public:
	EdHttpContent(bool multi);
	virtual ~EdHttpContent();
//	const char* getName();
//	const char* getFilename();
	string *getName();
	string* getFileName();

	void setUser(void *obj);
	void setUser(uint64_t ldata);
	void setUser(uint32_t wdata);
	void* getUserObj();
	uint64_t getUserLong();
	uint32_t getUserInt();

private:
	//void addHdr(const char* name, int len);
	void addHdr(string *name, string *val);
	bool isValidMp();
	void lookup();

private:
	union
	{
		uint32_t uwdata;
		uint64_t uldata;
		void* uobj;
	};
	bool mIsMultipart;
	EdObjList<_hdr_t> mHdrList;
	EdHdrContentDisposition* mCDisp;

};

} /* namespace edft */

#endif /* EDHTTPCONTENT_H_ */
