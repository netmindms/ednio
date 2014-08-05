/*
 * EdSSL.h
 *
 *  Created on: Aug 5, 2014
 *      Author: netmind
 */

#ifndef EDSSL_H_
#define EDSSL_H_

#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>

namespace edft
{

enum {
	SSL_VER_V23,
	SSL_VER_V3,
	SSL_VER_TLSV1,
	SSL_VER_TLSV11,
	SSL_VER_TLSV12,
	SSL_VER_DTLSV1,
};

class EdSSL
{
public:
	//EdSSL();
	//virtual ~EdSSL();

	static SSL_CTX* buildServerCtx(int sslmethod, const char* certfile, const char* privkeyfile);
	static void freeCtx(SSL_CTX* pctx);
};

} /* namespace edft */

#endif /* EDSSL_H_ */
