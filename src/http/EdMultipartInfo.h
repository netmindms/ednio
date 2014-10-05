/*
 * EdMultipartInfo.h
 *
 *  Created on: Oct 6, 2014
 *      Author: netmind
 */

#ifndef EDMULTIPARTINFO_H_
#define EDMULTIPARTINFO_H_

#include <string>
using namespace std;

class EdMultipartInfo
{
	virtual string *getName()=0;
	virtual string *getType()=0;
	virtual string *getFilename()=0;
	virtual string *getHdr(const char* name)=0;
};


#endif /* EDMULTIPARTINFO_H_ */
