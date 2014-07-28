/*
 * EdNio.cpp
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */


#include <signal.h>

const char* EdNioGetVer()
{
	return "0.5.0";
}

int EdNioInit()
{
	signal(SIGPIPE, SIG_IGN);
	return 0;
}
