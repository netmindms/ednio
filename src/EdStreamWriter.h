/*
 * EdStreamWriter.h
 *
 *  Created on: Feb 10, 2015
 *      Author: netmind
 */

#ifndef EXTERNAL_EDNIO_EDSTREAMWRITER_H_
#define EXTERNAL_EDNIO_EDSTREAMWRITER_H_

namespace edft
{

class EdStreamWriter
{
public:
	virtual size_t remain()=0;
	virtual size_t write(char* buf, size_t len)=0;
	virtual ~EdStreamWriter(){};
};

} /* namespace edft */

#endif /* EXTERNAL_EDNIO_EDSTREAMWRITER_H_ */
