/*
 * EsLog.h
 *
 *  Created on: 2014. 2. 26.
 *      Author: netmind
 */

#ifndef EDLOG_H_
#define EDLOG_H_

#include "EdRingQue.h"

namespace edft
{

class EdLog
{
public:
	EdLog(int bsize);
	virtual ~EdLog();
public:
	int print(const char* str);
	void flush(int count);
	void dumpAll(void);
	void peekLog(char* buf);
private:
	EdRingQue mQue;

};

} /* namespace edft */

#endif /* EDLOG_H_ */
