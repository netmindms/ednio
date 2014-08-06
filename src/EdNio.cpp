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

#include <signal.h>

#if USE_SSL
bool _gSSLIsInit=false;
#endif

const char* EdNioGetVer()
{
	return "0.5.0";
}

int EdNioInit()
{
	signal(SIGPIPE, SIG_IGN);
	return 0;
}

#if USE_SSL
int EdSSLInit()
{
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	_gSSLIsInit = true;
	return 0;
}

bool EdSSLIsInit()
{
	return _gSSLIsInit;
}
#endif
