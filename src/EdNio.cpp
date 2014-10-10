/*
 * EdNio.cpp
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */
#define DBGTAG "EDNIO"
#define DBG_LEVEL DBG_WARN

#include "config.h"

#include "EdNio.h"
#include "EdTask.h"
#include <string.h>
#include <signal.h>

namespace edft {


const char* EdNioGetVer()
{
	return "0.5.0";
}

int EdNioInit()
{
	signal(SIGPIPE, SIG_IGN);
	if(strcmp(EdNioGetVer(), EDNIO_VER))
	{
		return -1;
	}
	return 0;
}




}
