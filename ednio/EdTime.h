/*
 * EdTime.h
 *
 *  Created on: Jul 17, 2014
 *      Author: netmind
 */

#ifndef EDTIME_H_
#define EDTIME_H_
#include "ednio_config.h"

#include "EdType.h"

namespace edft
{

class EdTime
{
public:
	static u32 msecTime();
	static u64 usecTime();
};

} /* namespace edft */

#endif /* EDTIME_H_ */
