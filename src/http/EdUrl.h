/*
 * EdUrl.h
 *
 *  Created on: Sep 6, 2014
 *      Author: netmind
 */

#ifndef EDURL_H_
#define EDURL_H_

#include <string>
using namespace std;

namespace edft
{

class EdUrl
{
public:
	EdUrl();
	virtual ~EdUrl();

	int parse(const char* str, int len=0);
private:
	string *mHost;
	string *mPath;
	string *mQuery;


};

} /* namespace edft */

#endif /* EDURL_H_ */
