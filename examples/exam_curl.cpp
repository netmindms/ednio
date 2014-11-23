/*
 * exam_curl.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: netmind
 */

#include <ednio/EdNio.h>
#include <ednio/edcurl/EdCurl.h>
#include "applog.h"

using namespace edft;

class MainTask: public EdTask,
		public EdEasyCurl::ICurlHeader,
		public EdEasyCurl::ICurlBody,
		public EdEasyCurl::ICurlResult {
	EdMultiCurl* mMultiCurl;
	EdEasyCurl *mCurl;

	int OnEventProc(EdMsg* pmsg) {
		if (pmsg->msgid == EDM_INIT) {
			logs("main task init...");
			mMultiCurl = new EdMultiCurl();
			mMultiCurl->open();

			mCurl = new EdEasyCurl();
			mCurl->setOnCurlListener(this, this, this);
			mCurl->open(mMultiCurl);
			mCurl->request("http://curl.haxx.se");

		} else if (pmsg->msgid == EDM_CLOSE) {
			logs("main task close...");
			mCurl->close();
			mMultiCurl->close();

			delete mCurl;
			delete mMultiCurl;
		}
		return 0;

	}

	void IOnCurlHeader(EdEasyCurl* pcurl) {
		logs("curl on header ...");
		int code = pcurl->getResponseCode();
		logs("resp code = %d", code);
		logs("content len = %ld", pcurl->getContentLength());
		logs("content type = %s", pcurl->getHeader("Content-Type"));
	}

	void IOnCurlBody(EdEasyCurl* pcurl, void* ptr, int size) {
		logs("curl on body..., size=%d", size);
	}

	void IOnCurlResult(EdEasyCurl* pcurl, int status) {
		logs("curl on result, status=%d", status);
		postExit();
	}
};

int main() {
	MainTask *task = new MainTask();
	task->runMain();
	delete task;

	return 0;
}

