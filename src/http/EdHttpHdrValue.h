/*
 * EdHttpHdrValue.h
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */

#ifndef EDHTTPHDRVALUE_H_
#define EDHTTPHDRVALUE_H_

#include <string>

using namespace std;

namespace edft
{

class EdHttpHdrValue
{
public:
	EdHttpHdrValue();
	virtual ~EdHttpHdrValue();
	virtual void parse(const char *str, int len)=0;
	void parseGeneral(const char *str, int len);
	void parseDate(const char *str, int len);
	virtual void OnTag(const char* name, int nlen, const char* val, int vlen);
};

} /* namespace edft */

#endif /* EDHTTPHDRVALUE_H_ */
