/*
 * EdObject.cpp
 *
 *  Created on: Aug 26, 2014
 *      Author: netmind
 */

#include "EdObject.h"

namespace edft
{

EdObject::EdObject()
{
	// TODO Auto-generated constructor stub

}

EdObject::~EdObject()
{
	// TODO Auto-generated destructor stub
}

void EdObject::lock()
{
	mMutex.lock();
}

void EdObject::unlock()
{
	mMutex.unlock();
}

} /* namespace edft */
