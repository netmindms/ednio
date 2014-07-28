/*
 * EdTime.h
 *
 *  Created on: Jul 17, 2014
 *      Author: netmind
 */

#ifndef EDTIME_H_
#define EDTIME_H_
#include "EdType.h"

namespace edft
{

class EdTime
{
public:
	EdTime();
	virtual ~EdTime();


	static u32 msecTime();
private:
	static u32 mStartMsec;
};

} /* namespace edft */

#endif /* EDTIME_H_ */
