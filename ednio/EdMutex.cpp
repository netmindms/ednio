/*
 * EdMutex.cpp
 *
 *  Created on: Jan 13, 2014
 *      Author: khkim
 */

#include "EdMutex.h"

namespace edft
{

EdMutex::EdMutex()
{
	pthread_mutex_init(&mMutex, NULL);
}

EdMutex::~EdMutex()
{
	pthread_mutex_destroy(&mMutex);
}

void EdMutex::lock(void)
{
	pthread_mutex_lock(&mMutex);
}

void EdMutex::unlock(void)
{
	pthread_mutex_unlock(&mMutex);
}

} /* namespace edft */
