/*
 * EdHttpTask.h
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */

#ifndef EDHTTPTASK_H_
#define EDHTTPTASK_H_
#include "../EdTask.h"
#include "EdHttpController.h"

namespace edft
{

class EdHttpTask : public EdTask
{
public:
	EdHttpTask();
	virtual ~EdHttpTask();
	virtual EdHttpController* OnNewRequest(const char* method, const char* url);

};

} /* namespace edft */

#endif /* EDHTTPTASK_H_ */
