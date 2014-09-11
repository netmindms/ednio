/*
 * EdHttpStringWriter.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPSTRINGWRITER_H_
#define EDHTTPSTRINGWRITER_H_
#include "../config.h"
#include <string>
#include "EdHttpWriter.h"

using namespace std;

namespace edft
{

class EdHttpStringWriter : public EdHttpWriter
{
public:
	EdHttpStringWriter();
	virtual ~EdHttpStringWriter();
	virtual int Write(void* buf, int len);
	string *getString();

private:
	std::string mString;
};

} /* namespace edft */

#endif /* EDHTTPSTRINGWRITER_H_ */
