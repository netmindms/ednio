/*
 * EdNotFoundHttpController.h
 *
 *  Created on: Sep 22, 2014
 *      Author: netmind
 */

#ifndef EDNOTFOUNDHTTPCONTROLLER_H_
#define EDNOTFOUNDHTTPCONTROLLER_H_
#include "../config.h"
#include "EdHttpController.h"
#include "EdHttpStringReader.h"

namespace edft
{

class EdNotFoundHttpController: public EdHttpController
{
public:
	EdNotFoundHttpController();
	virtual ~EdNotFoundHttpController();
	void OnHttpRequestHeader();

private:
	EdHttpStringReader mReader;
};

} /* namespace edft */

#endif /* EDNOTFOUNDHTTPCONTROLLER_H_ */
