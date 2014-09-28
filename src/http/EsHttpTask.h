/*
 * EsHttpTask.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPTASK_H_
#define ESHTTPTASK_H_
#include "../config.h"

#include <string>
#include <unordered_map>

#include "../EsHandleManager.h"
#include "../EdTask.h"
#include "../EdObjList.h"
#include "EsHttpTrans.h"
#include "EsHttpCnn.h"
#include "EdHttpController.h"


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
	template<typename T> void addInCtrl(const char* method, const char *url) {
		T* pctrl = new T;
		mUrlMap[url] = pctrl;
	};

	typedef EdHttpController* (*__alloc_controller)();
	struct urlmapinfo_t {
		__alloc_controller alloc;
		union {
		void* userObj;
		u64 ldata;
		u32 wdata;
		};
	};
public:

	template<typename T>
	void regController(const char* url, void *user) {

		class __defalloc {
		public:
			static T* alloc() { return new T; }
		};
		struct urlmapinfo_t *info = new urlmapinfo_t;
		info->alloc = (__alloc_controller)__defalloc::alloc;;
		info->userObj = user;
		mAllocMap[url] = info;

	};


private:
	//EsHandleManager<EsHttpCnn> mCnns;
	EdObjList<EsHttpCnn> mCnns;
	unordered_map<string, EdHttpController*> mUrlMap;
	//unordered_map<string, __alloc_controller> mAllocMap;
	unordered_map<string, urlmapinfo_t*> mAllocMap;
	EdHttpController*allocController(const char *url);
	void freeController(EdHttpController* pctrl);
	void release();
	void freeConnection(EsHttpCnn *pcnn);


};

} // namespae edft

#endif /* ESHTTPTASK_H_ */
