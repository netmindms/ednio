/*
 * EsHttpPaser.h
 *
 *  Created on: Jun 20, 2014
 *      Author: khkim
 */

#ifndef ESHTTPPASER_H_
#define ESHTTPPASER_H_

#include <string>
using namespace std;


#define REM_CHAR(PTR, C, CNT) { for(;*PTR==C && *PTR!='\r' && CNT>0;PTR++, CNT--); }
#define CON_CHAR(PTR, DEL, CNT) { for(;*PTR!=DEL && *PTR!='\r' && CNT>0;PTR++, CNT--);}
#define THROW_ERR(CODE) throw CODE



class EsHttpPaser
{
public:
	enum {
		REQLINE_S,
		HDRNAME_S,
		HDRVAL_S,
	};
public:
	EsHttpPaser();
	virtual ~EsHttpPaser();
	char* getBuffer(int *size);
	void consumeData(int size);

	void putData(char *str, int len);
	bool mIsResp;
	int mState;
	char* mBuffer;
	char* mElem;
	int mBufSize;
	int mRcnt;
	int mWcnt;
	string httpVer;
	string statusCode;
	string respDesc;
};


#endif /* ESHTTPPASER_H_ */
