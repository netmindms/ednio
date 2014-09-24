/*
 * EdNio.cpp
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */
#define DBGTAG "edcmm"
#define DBG_LEVEL DBG_DEBUG

#include "config.h"

#if USE_SSL
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#endif
#include "EdTask.h"
#include <signal.h>

namespace edft {


const char* EdNioGetVer()
{
	return "0.5.0";
}

int EdNioInit()
{
	signal(SIGPIPE, SIG_IGN);
	return 0;
}




}
