/*
 * EdHttpController.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPCONTROLLER_H_
#define EDHTTPCONTROLLER_H_

#include "../EdEventFd.h"
#include "EdHttpWriter.h"
#include "EdHttpReader.h"

namespace edft
{

class EdHttpController : public EdEventFd
{
public:
	EdHttpController();
	virtual ~EdHttpController();
	virtual void OnRequest();
	virtual void OnContentRecvComplete();
	virtual void OnContentSendComplete();
	virtual void OnComplete();
	void close();
	void setReqBodyWriter(EdHttpWriter* writer);
	void setHttpResult(const char *code);
	void setRespBodyReader(EdHttpReader* reader);
private:
	EdHttpWriter* mWriter;
	EdHttpReader* mReader;

};

} /* namespace edft */

#endif /* EDHTTPCONTROLLER_H_ */
