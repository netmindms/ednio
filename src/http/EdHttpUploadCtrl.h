/*
 * EdHttpUploadCtrl.h
 *
 *  Created on: Oct 8, 2014
 *      Author: netmind
 */

#ifndef EDHTTPUPLOADCTRL_H_
#define EDHTTPUPLOADCTRL_H_

#include <string>
#include "EdHttpFileWriter.h"
#include "EdHttpController.h"

using namespace std;

namespace edft
{

class EdHttpUploadCtrl: public EdHttpController
{
public:
	EdHttpUploadCtrl();
	virtual ~EdHttpUploadCtrl();
	// >>>>>>>>>>>>>>> virtual functions
	void OnRequestHeader();
	void OnDataNew(EdHttpContent *pctt);
	void OnDataContinue(EdHttpContent *pctt, const void *buf, int len);
	void OnDataRecvComplete(EdHttpContent *pctt);
	void OnComplete(int result);
	// <<<<<<<<<<<<<<<

	void setPath(const char* path);

private:
	EdHttpFileWriter *mWriter;
	string mPath;
};

} /* namespace edft */

#endif /* EDHTTPUPLOADCTRL_H_ */
