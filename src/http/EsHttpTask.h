/*
 * EsHttpTask.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPTASK_H_
#define ESHTTPTASK_H_

#include <string>
#include <unordered_map>

#include "../EsHandleManager.h"
#include "../EdTask.h"
#include "../EdObjList.h"
#include "EsHttpTrans.h"
#include "EsHttpCnn.h"
#include "EdHttpController.h"


#include "IUriController.h"

using namespace std;
namespace edft {

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
	virtual EdHttpController* OnNewRequest(const char *method, const char *url);

public:
	void setController(char* uri, IUriControllerCb *cb);

private:
	//EsHandleManager<EsHttpCnn> mCnns;
	EdObjList<EsHttpCnn> mCnns;
	unordered_map<string, IUriControllerCb*> mContMap;
	IUriControllerCb* getController(string* uri);
};

} // namespae edft

#endif /* ESHTTPTASK_H_ */
