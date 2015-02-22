/*
 * EdStreamReader.h
 *
 *  Created on: Jan 31, 2015
 *      Author: netmind
 */

#ifndef EXTERNAL_EDNIO_EDSTREAMREADER_H_
#define EXTERNAL_EDNIO_EDSTREAMREADER_H_

#include <memory>
#include <unistd.h>

using namespace std;

namespace edft
{

class EdStreamReader
{
public:
	EdStreamReader(){};
	virtual ~EdStreamReader(){};
	virtual size_t read(char* buf, size_t)=0;
	virtual size_t remain()=0;
};

typedef unique_ptr< EdStreamReader > upEdStreamReader;

} /* namespace edft */

#endif /* EXTERNAL_EDNIO_EDSTREAMREADER_H_ */
