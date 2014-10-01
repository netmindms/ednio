/*
 * EdHttpWriter.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPWRITER_H_
#define EDHTTPWRITER_H_
#include "../config.h"
namespace edft
{

class EdHttpWriter
{
public:
	virtual long Write(void *buf, long len)=0;
};

} /* namespace edft */

#endif /* EDHTTPWRITER_H_ */
