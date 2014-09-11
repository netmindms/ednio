/*
 * EdHttpWriter.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPWRITER_H_
#define EDHTTPWRITER_H_

namespace edft
{

class EdHttpWriter
{
public:
	virtual int Write(void *buf, int len)=0;
};

} /* namespace edft */

#endif /* EDHTTPWRITER_H_ */
