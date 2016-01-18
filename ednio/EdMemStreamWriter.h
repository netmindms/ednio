/*
 * EdMemStreamWriter.h
 *
 *  Created on: Feb 11, 2015
 *      Author: netmind
 */

#ifndef EXTERNAL_EDNIO_EDMEMSTREAMWRITER_H_
#define EXTERNAL_EDNIO_EDMEMSTREAMWRITER_H_

#include <vector>
#include "EdStreamWriter.h"
using namespace std;
namespace edft
{

class EdMemStreamWriter : public EdStreamWriter
{
public:
	EdMemStreamWriter();
	virtual ~EdMemStreamWriter();

	size_t remain() override ;
	size_t write(char *buf, size_t len) override ;
	void reserve(size_t size);

private:
	vector<char> mVecBuf;
	size_t mWritePos;
};

} /* namespace edft */

#endif /* EXTERNAL_EDNIO_EDMEMSTREAMWRITER_H_ */
