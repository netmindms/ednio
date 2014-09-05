/*
 * EsHttpTask.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPTASK_H_
#define ESHTTPTASK_H_

#include "../EsHandleManager.h"
#include "../EdTask.h"
#include "EsHttpTrans.h"
#include "EsHttpCnn.h"


#include <string>
#include <unordered_map>
#include "IUriController.h"

using namespace std;
using namespace edft;

enum {
	UV_HTTPCNN=EDM_USER+1,
};

extern __thread EsHttpTask *_tHttpTask;

class EsHttpTask: public EdTask, public EdSocket::ISocketCb
{
	friend class EsHttpCnn;

public:
	EsHttpTask();
	virtual ~EsHttpTask();

	virtual int OnEventProc(EdMsg* pmsg);
	virtual void IOnSocketEvent(EdSocket *psock, int event);

public:
	void setController(char* uri, IUriControllerCb *cb);

private:
	EsHandleManager<EsHttpCnn> mCnns;
	unordered_map<string, IUriControllerCb*> mContMap;
	IUriControllerCb* getController(string* uri);
};


#endif /* ESHTTPTASK_H_ */
