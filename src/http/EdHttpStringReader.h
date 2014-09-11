/*
 * EdHttpStringReader.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPSTRINGREADER_H_
#define EDHTTPSTRINGREADER_H_
#include "../config.h"

#include <string>
#include "EdHttpReader.h"
namespace edft
{

class EdHttpStringReader : public EdHttpReader
{
public:
	EdHttpStringReader();
	virtual ~EdHttpStringReader();
	virtual int Read(void *buf, int len);
private:
	std::string mString;
};

} /* namespace edft */

#endif /* EDHTTPSTRINGREADER_H_ */
