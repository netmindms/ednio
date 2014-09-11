/*
 * EdTime.cpp
 *
 *  Created on: Jul 17, 2014
 *      Author: netmind
 */

#include <time.h>
#include "EdTime.h"

namespace edft
{


u32 EdTime::msecTime()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (tp.tv_sec * 1000 + tp.tv_nsec/1000000);
}

} /* namespace edft */
