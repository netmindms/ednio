/*
 * EdHdrContentDisposition.h
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */

#ifndef EDHDRCONTENTDISPOSITION_H_
#define EDHDRCONTENTDISPOSITION_H_

#include "EdHttpHdrValue.h"
namespace edft
{

#define BOUNDARY_TAG "boundary"
#define FORM_DATA "form-data"
#define ATTACHMENT "attachment"

class EdHdrContentDisposition : public EdHttpHdrValue
{
public:
	EdHdrContentDisposition();
	virtual ~EdHdrContentDisposition();
	void parse(const char* str, int len);
	void OnTag(const char *tname, int nlen, const char* tval, int vlen);
	const char* getBoundary();

private:
	string mDesc;
	string mBoundary;
	string mName;
	string mFileName;
};

} /* namespace edft */

#endif /* EDHDRCONTENTDISPOSITION_H_ */
