/*
 * EsHttpTask.h
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#ifndef ESHTTPTASK_H_
#define ESHTTPTASK_H_
#include "../ednio_config.h"

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


typedef struct {
	int recv_buf_size;
	int cnn_time_out;
} http_server_cfg_t;

typedef struct {

} url_map_info_t;

/**
 * @author netmind
 * @class EdHttpTask
 * @brief Process http protocol and events.
 * @remark EdHttpTask is a task which processes http request/response and the protocols.\n
 * This provides the events of http protocol to http url controller.
 * In General, this is used as inheriting rather than using itself directly.
 *
 */
class EdHttpTask: public EdTask
{
	friend class EdHttpCnn;

public:
	EdHttpTask();
	virtual ~EdHttpTask();
	virtual int OnEventProc(EdMsg* pmsg);
	virtual void OnInitHttp();

#if USE_SSL
	int setDefaultCertFile(const char* crtfile, const char* keyfile);
	void setDefaultCertPassword(const char* pw);
	int openDefaultCertFile(const char* crtfile, const char* keyfile, const char* pw);
#endif
	http_server_cfg_t* getConfig();

	typedef EdHttpController* (*__alloc_controller)();
	struct urlmapinfo_t {
		__alloc_controller alloc;
		string path;
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

	template<typename T>
		void regControllerPath(const char* url, void *user) {

			class __defalloc {
			public:
				static T* alloc() { return new T; }
				static bool pathcomp(urlmapinfo_t* &left, urlmapinfo_t* &right) {
					if(left->path.size() > right->path.size()) {
						return true;
					} else {
						return false;
					}
				}
			};
			struct urlmapinfo_t *info = new urlmapinfo_t;
			info->path = url;
			info->alloc = (__alloc_controller)__defalloc::alloc;;
			info->userObj = user;
			mUrlPathMap.push_back(info);
			mUrlPathMap.sort(__defalloc::pathcomp);
		};



private:
	http_server_cfg_t mConfig;
	EdObjList<EdHttpCnn> mCnns;
	list<urlmapinfo_t*> mUrlPathMap;
	unordered_map<string, urlmapinfo_t*> mAllocMap;
	EdHttpController*allocController(const char *url);
	void freeController(EdHttpController* pctrl);
	void release();
	void removeConnection(EdHttpCnn *pcnn);
	int mTaskId;
	u32 mCnnHandleSeed;
};

} // namespae edft

#endif /* ESHTTPTASK_H_ */
