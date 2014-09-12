/*
 * EdHttpController.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: netmind
 */
#define DBGTAG "htctr"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdHttpController.h"

namespace edft
{

EdHttpController::EdHttpController()
{
	// TODO Auto-generated constructor stub
	mWriter = NULL;
}

EdHttpController::~EdHttpController()
{
	// TODO Auto-generated destructor stub
}


void EdHttpController::close()
{
}


void EdHttpController::OnRequest()
{
}

void EdHttpController::OnContentRecvComplete()
{
}

void EdHttpController::OnContentSendComplete()
{
}


void EdHttpController::OnComplete()
{
}

void EdHttpController::setReqBodyWriter(EdHttpWriter* writer)
{
	if(mWriter == NULL)
	{
		mWriter = writer;
	}
	else
	{
		dbge("### Body writer already set...");
	}
}


void EdHttpController::setHttpResult(const char* code)
{
}

} /* namespace edft */
