/*
 * EdObject.h
 *
 *  Created on: Aug 26, 2014
 *      Author: netmind
 */

#ifndef EDOBJECT_H_
#define EDOBJECT_H_
#include "config.h"


namespace edft
{

class EdObject
{
	friend class EdTask;
public:
	EdObject();
	virtual ~EdObject();
private:
	bool mIsFree;
};

} /* namespace edft */

#endif /* EDOBJECT_H_ */
