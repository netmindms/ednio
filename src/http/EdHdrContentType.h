/*
 * EdHdrContentType.h
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */

#ifndef EDHDRCONTENTTYPE_H_
#define EDHDRCONTENTTYPE_H_

#include "EdHttpHdrValue.h"

namespace edft
{
//Content-Type: text/html; charset=ISO-8859-4

class EdHdrContentType : public EdHttpHdrValue
{
public:
	EdHdrContentType();
	virtual ~EdHdrContentType();
	virtual void OnTag(const char* name, int nlen, const char* val, int vlen);
	void parse(const char* name, int len);
	const char* getType();
	const char* getParam(const char* name);

private:
	string mType;
	string mParaName, mParaVal;
};

} /* namespace edft */

#endif /* EDHDRCONTENTTYPE_H_ */
