/*
 * EdSSL.cpp
 *
 *  Created on: Aug 5, 2014
 *      Author: netmind
 */
#define DBGTAG "edssl"
#define DBG_LEVEL DBG_WARN

#include "../edslog.h"
#include "EdSSL.h"

namespace edft
{

bool _gSSLIsInit=false;

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

SSL_CTX* EdSSL::buildServerCtx(int sslmethod, const char* certfile, const char* privkeyfile)
{
	const SSL_METHOD *method;
	switch (sslmethod)
	{
	case SSL_VER_TLSV1:
		method = TLSv1_server_method();
		break;
	case SSL_VER_TLSV11:
		method = TLSv1_1_server_method();
		break;
	case SSL_VER_V23:
		method = SSLv23_server_method();
		break;
	case SSL_VER_V3:
		method = SSLv3_server_method();
		break;
	case SSL_VER_DTLSV1:
		method = DTLSv1_server_method();
		break;
	default:
		method = NULL;
		break;
	}

	if (method == NULL)
		return NULL;

	SSL_CTX* pctx = SSL_CTX_new(method);

	int ret;
	ret = SSL_CTX_use_certificate_file(pctx, certfile, SSL_FILETYPE_PEM);
	dbgd("set cert file, ret=%d", ret);
	ret = SSL_CTX_use_PrivateKey_file(pctx, privkeyfile, SSL_FILETYPE_PEM);
	dbgd("set key file, ret=%d", ret);

	if (!SSL_CTX_check_private_key(pctx))
	{
		dbge("### private key check error......");
		SSL_CTX_free(pctx);
		return NULL;
	}
	return pctx;
}

void EdSSL::freeCtx(SSL_CTX* pctx)
{
	SSL_CTX_free(pctx);
}

SSL_CTX* EdSSL::buildClientCtx(int ver)
{
	SSL_CTX *pctx;
	const SSL_METHOD *method;
	switch (ver)
	{
	case SSL_VER_TLSV1:
		method = TLSv1_method();
		break;
	case SSL_VER_TLSV11:
		method = TLSv1_1_client_method();
		break;
	case SSL_VER_V23:
		method = SSLv23_client_method();
		break;
	case SSL_VER_V3:
		method = SSLv3_client_method();
		break;
	case SSL_VER_DTLSV1:
		method = DTLSv1_client_method();
		break;
	default:
		method = NULL;
		break;
	}
	if (method == NULL)
		return NULL;

	pctx = SSL_CTX_new(method);

	return pctx;
}

SSL_CTX* EdSSL::buildCtx(int ver)
{
	SSL_CTX *pctx;
	const SSL_METHOD *method;
	switch (ver)
	{
	case SSL_VER_TLSV1:
		method = TLSv1_method();
		break;
	case SSL_VER_TLSV11:
		method = TLSv1_1_method();
		break;
	case SSL_VER_V23:
		method = SSLv23_method();
		break;
	case SSL_VER_V3:
		method = SSLv3_method();
		break;
	case SSL_VER_DTLSV1:
		method = DTLSv1_method();
		break;
	default:
		method = NULL;
		break;
	}
	if (method == NULL)
		return NULL;

	pctx = SSL_CTX_new(method);

	return pctx;
}

} /* namespace edft */
