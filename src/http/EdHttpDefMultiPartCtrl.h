/*
 * EdHttpDefMultiPartCtrl.h
 *
 *  Created on: Oct 7, 2014
 *      Author: netmind
 */

#ifndef EDHTTPDEFMULTIPARTCTRL_H_
#define EDHTTPDEFMULTIPARTCTRL_H_

#include <unordered_map>
#include "EdHttpController.h"

using namespace std;

namespace edft
{


class EdHttpDefMultiPartCtrl : public EdHttpController
{
public:
	EdHttpDefMultiPartCtrl();
	virtual ~EdHttpDefMultiPartCtrl();
	void OnHttpDataNew(EdHttpContent* pctt);
	void OnHttpDataContinue(EdHttpContent* pctt, const void* buf, int len);
	void OnHttpDataRecvComplete(EdHttpContent *pct);

	void setFileFolder(const char* path);
	string getData(const char* name);
	string getFile(const char* name, long *plen);

private:
	typedef struct {
		string fileName;
		EdHttpWriter *writer;
	} _cinfo_t;
	unordered_map<string, _cinfo_t*> mCttList;
	string mFolder;

};

} /* namespace edft */

#endif /* EDHTTPDEFMULTIPARTCTRL_H_ */
