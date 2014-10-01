/*
 * EdNotFoundHttpController.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: netmind
 */
#include "../config.h"
#include "EdNotFoundHttpController.h"

namespace edft
{

EdNotFoundHttpController::EdNotFoundHttpController()
{

}

EdNotFoundHttpController::~EdNotFoundHttpController()
{
}

void EdNotFoundHttpController::OnRequest()
{
	mReader.setString("<h1>Page Not Found......<h1>\n");
	setRespBodyReader(&mReader,"text/html");
	setHttpResult("404");
}

} /* namespace edft */
