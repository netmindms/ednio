/*
 * EdSSL.cpp
 *
 *  Created on: Aug 5, 2014
 *      Author: netmind
 */
#define DBGTAG "edssl"
#define DBG_LEVEL DBG_DEBUG

#include "../edslog.h"
#include "EdSSLContext.h"

namespace edft
{

__thread class EdSSLContext *_tDefEdSSL = NULL;

bool _gSSLIsInit = false;

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

EdSSLContext::EdSSLContext()
{
	mCtx = NULL;
}

EdSSLContext::~EdSSLContext()
{
}

SSL_CTX* EdSSLContext::buildServerCtx(int sslmethod, const char* certfile, const char* privkeyfile)
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

void EdSSLContext::close()
{
	if(mCtx != NULL)
	{
		SSL_CTX_free(mCtx);
		mCtx = NULL;
	}
}

SSL_CTX* EdSSLContext::buildClientCtx(int ver)
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

SSL_CTX* EdSSLContext::buildCtx(int ver)
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

EdSSLContext* EdSSLContext::getDefaultEdSSL()
{
	if (_tDefEdSSL == NULL)
	{
		if (EdSSLIsInit() == false)
		{
			EdSSLInit();
		}

		EdSSLContext *pssl =new EdSSLContext;
		pssl->open(SSL_VER_TLSV1);
		_tDefEdSSL = pssl;
	}

	return _tDefEdSSL;
}



void EdSSLContext::freeDefaultEdSSL()
{

	if(_tDefEdSSL != NULL)
	{
		_tDefEdSSL->close();
		delete _tDefEdSSL; _tDefEdSSL=NULL;
	}

}


SSL_CTX* EdSSLContext::getContext()
{
	return mCtx;
}

int EdSSLContext::setSSLCertFile(const char* certfile, const char* privkeyfile)
{
	int ret;

	ret = SSL_CTX_use_certificate_file(mCtx, certfile, SSL_FILETYPE_PEM);
	dbgd("set cert file, ret=%d", ret);
	ret = SSL_CTX_use_PrivateKey_file(mCtx, privkeyfile, SSL_FILETYPE_PEM);
	dbgd("set key file, ret=%d", ret);

	if (!SSL_CTX_check_private_key(mCtx))
	{
		dbge("### private key check error......");
		return -1;
	}
	return 0;
}


void EdSSLContext::open(int ver)
{
	mCtx = EdSSLContext::buildCtx(ver);
	SSL_CTX_set_default_passwd_cb(mCtx, password_cb);
	SSL_CTX_set_default_passwd_cb_userdata(mCtx, (void*)this);

}

void EdSSLContext::setCertPassword(const char* pw)
{
	strncpy(mPasswd, pw, sizeof(mPasswd)-1);
}


int EdSSLContext::password_cb(char* buf, int size, int rwflag, void* userdata)
{
	EdSSLContext* pssl = (EdSSLContext*)userdata;
	return pssl->dgPasswordCb(buf, size, rwflag);
}

int EdSSLContext::dgPasswordCb(char* buf, int size, int rwflag)
{
	dbgd("cert password callback,...size=%d, rwflag=%d", size, rwflag);
	strcpy(buf, mPasswd);
	return strlen(mPasswd);
}

} /* namespace edft */
