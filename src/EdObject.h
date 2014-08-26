/*
 * EdObject.h
 *
 *  Created on: Aug 26, 2014
 *      Author: netmind
 */

#ifndef EDOBJECT_H_
#define EDOBJECT_H_
#include "config.h"

#include "EdMutex.h"

namespace edft
{

class EdObject
{
public:
	EdObject();
	virtual ~EdObject();
	void lock();
	void unlock();
private:
	EdMutex mMutex;
};

} /* namespace edft */

#endif /* EDOBJECT_H_ */
