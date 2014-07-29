/*
 * EdMutex.h
 *
 *  Created on: Jan 13, 2014
 *      Author: khkim
 */

#ifndef EDMUTEX_H_
#define EDMUTEX_H_
#include "config.h"

#include <pthread.h>

namespace edft
{

class EdMutex
{
public:
	EdMutex();
	virtual ~EdMutex();
	void lock(void);
	void unlock(void);

protected:
	pthread_mutex_t mMutex;
};

} /* namespace edft */

#endif /* EDMUTEX_H_ */
