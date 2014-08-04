/*
 * EdSSLSocket.h
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#ifndef EDSSLSOCKET_H_
#define EDSSLSOCKET_H_

#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>

#include "EdSocket.h"

namespace edft
{

class EdSSLSocket : public EdSocket
{
public:
	class ISSLSocketCb {
	public:
		virtual void IOnSSLSocket(EdSSLSocket *psock, int event)=0;
	};

public:
	EdSSLSocket();
	virtual ~EdSSLSocket();

	virtual void OnRead();
	virtual void OnWrite();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual void OnSSLConnected();
	virtual void OnSSLDisconnected();
	virtual void OnSSLRead();


	/**
	 * @brief Read data from ssl connection
	 * @remark After ssl session is complete, call this method to read received from ssl connection when OnSSLRead() or IOnSSLSocket(psock, SOCK_EVENT_READ) is called.
	 * @param buf Buffer to read
	 * @param bufsize Buffer size
	 * @return Read read count
	 */
	int sslRecv(void* buf, int bufsize);
	int sslSend(const void* buf, int bufsize);

	/**
	 * @brief Close ssl connection.
	 */
	void sslClose();

	/**
	 * @brief Get a openssl session.
	 * @return openssl session session pointer
	 */
	SSL *getSSL();
	SSL_CTX* getSSLContext();
	void setSSLCallback(ISSLSocketCb *cb);

private:
	void startHandshake();
	void procSSLRead(void);
	void changeSSLSockEvent(int err, bool bwrite);
private:
	SSL *mSSL;
	SSL_CTX *mSSLCtx;
	bool mIsHandshake;
	ISSLSocketCb *mSSLCallback;

};

} /* namespace edft */

#endif /* EDSSLSOCKET_H_ */
