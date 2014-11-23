/*
 * EdHttpReader.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPREADER_H_
#define EDHTTPREADER_H_

#include "../ednio_config.h"

namespace edft
{

class EdHttpReader
{
public:
	virtual long IGetBodySize()=0;
	virtual long IReadBodyData(void *buf, long len)=0;
};

} /* namespace edft */

#endif /* EDHTTPREADER_H_ */
