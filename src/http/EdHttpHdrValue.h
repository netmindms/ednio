/*
 * EdHttpHdrValue.h
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */

#ifndef EDHTTPHDRVALUE_H_
#define EDHTTPHDRVALUE_H_

namespace edft
{

class EdHttpHdrValue
{
public:
	EdHttpHdrValue();
	virtual ~EdHttpHdrValue();
	void parse(const char *str, int len);
};

} /* namespace edft */

#endif /* EDHTTPHDRVALUE_H_ */
