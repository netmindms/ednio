/*
 * EdHttpReader.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPREADER_H_
#define EDHTTPREADER_H_

namespace edft
{

class EdHttpReader
{
public:
	virtual int Read(void *buf, int len)=0;
};

} /* namespace edft */

#endif /* EDHTTPREADER_H_ */
