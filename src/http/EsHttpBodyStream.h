/*
 * EsHttpBodyStream.h
 *
 *  Created on: Jul 9, 2014
 *      Author: netmind
 */

#ifndef ESHTTPBODYSTREAM_H_
#define ESHTTPBODYSTREAM_H_

enum {
	BODYSTR_BUF,
	BODYSTR_FILE,
};

class EsHttpBodyStream {
public:
	virtual int open()=0;
	virtual const char* getContentType()=0;
	virtual int getContentLen()=0;
	virtual int getData(void *buf, int bufsize)=0;
	virtual int getStreamType()=0;
	virtual void* getBuffer()=0;
	virtual void close()=0;
};


#endif /* ESHTTPBODYSTREAM_H_ */
