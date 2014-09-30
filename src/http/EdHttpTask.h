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
#include "EdHttpCnn.h"
#include "EdHttpController.h"


using namespace std;
namespace edft {

enum {
	EDMX_HTTPCNN=EDM_USER-1,
};


class EdHttpTask: public EdTask, public EdSocket::ISocketCb
{
	friend class EdHttpCnn;

public:
	EdHttpTask();
	virtual ~EdHttpTask();
	virtual int OnEventProc(EdMsg* pmsg);
	virtual void IOnSocketEvent(EdSocket *psock, int event);

	int setDefaultCertFile(const char* crtfile, const char* keyfile);
	void setDefaultCertPassword(const char* pw);
	int openDefaultCertFile(const char* crtfile, const char* keyfile, const char* pw);


	typedef EdHttpController* (*__alloc_controller)();
	struct urlmapinfo_t {
		__alloc_controller alloc;
		union {
		void* userObj;
		u64 ldata;
		u32 wdata;
		};
	};

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
	EdObjList<EdHttpCnn> mCnns;
	unordered_map<string, EdHttpController*> mUrlMap;
	unordered_map<string, urlmapinfo_t*> mAllocMap;
	EdHttpController*allocController(const char *url);
	void freeController(EdHttpController* pctrl);
	void release();
	void freeConnection(EdHttpCnn *pcnn);


};

} // namespae edft

#endif /* ESHTTPTASK_H_ */
