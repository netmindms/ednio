/*
 * EdCurlSocket.h
 *
 *  Created on: Aug 10, 2014
 *      Author: netmind
 */

#ifndef EDCURLSOCKET_H_
#define EDCURLSOCKET_H_
#include "../ednio_config.h"

#include <curl/curl.h>
#include "../EdEvent.h"

namespace edft {

class EdMultiCurl;
class EdCurlSocket : public EdEvent
{
	friend class EdMultiCurl;
public:
	EdCurlSocket();
	virtual ~EdCurlSocket();
	void open(EdMultiCurl* edmcurl, int fd);
	void setEvent(int eventflag);
	virtual void OnEventRead();
	virtual void OnEventWrite();
	virtual void OnEventHangup();

private:
	EdMultiCurl *mEdMultiCurl;
	void curlSockCb(int what);

};

} // namespace edft

#endif /* EDCURLSOCKET_H_ */
