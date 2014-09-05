/*
 * EsHttpTextBody.h
 *
 *  Created on: Jul 9, 2014
 *      Author: netmind
 */

#ifndef ESHTTPTEXTBODY_H_
#define ESHTTPTEXTBODY_H_

#include "EsHttpBodyStream.h"


class EsHttpTextBody: public EsHttpBodyStream
{
public:
	EsHttpTextBody(char *txt, int len);
	virtual ~EsHttpTextBody();
	virtual int open();
	virtual const char* getContentType();
	virtual int getContentLen();
	virtual int getData(void *buf, int bufsize);
	virtual int getStreamType();
	virtual void* getBuffer();
	virtual void close();
private:
	int mLen;
	int mType;
	void* mBuf;


};

#endif /* ESHTTPTEXTBODY_H_ */
