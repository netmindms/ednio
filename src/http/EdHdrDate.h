/*
 * EdHdrDate.h
 *
 *  Created on: Oct 3, 2014
 *      Author: netmind
 */

#ifndef EDHDRDATE_H_
#define EDHDRDATE_H_

#include "EdHttpHdrValue.h"

namespace edft
{

#define HEADER_DATE_INFO_COUNT 6

class EdHdrDate: public EdHttpHdrValue
{
public:
	EdHdrDate();
	virtual ~EdHdrDate();
	virtual void OnTag(const char* name, int nlen, const char* val, int vlen);
	void parse(const char* str, int len);

private:
	string mDateData[HEADER_DATE_INFO_COUNT];
	int mItemCount;
};

} /* namespace edft */

#endif /* EDHDRDATE_H_ */
