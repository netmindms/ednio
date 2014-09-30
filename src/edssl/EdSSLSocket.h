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

#include "../EdSocket.h"

namespace edft
{

enum {
		SSL_EVENT_DISCONNECTED=0,
		SSL_EVENT_CONNECTED,
		SSL_EVENT_READ,
		SSL_EVENT_WRITE,
	};

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
	 * @brief Connect to SSL server.
	 */
	int connect(const char *ipaddr, int port);
	int connect(uint32_t ip, int port);

	/**
	 * @brief Read data from ssl connection
	 * @remark After ssl session is complete, call this method to read received from ssl connection when OnSSLRead() or IOnSSLSocket(psock, SOCK_EVENT_READ) is called.
	 * @param buf Buffer to read
	 * @param bufsize Buffer size
	 * @return Read read count
	 */

	void sslAccept();

	int recv(void* buf, int size);

	/**
	 * @brief send packet to server.
	 * @remark You should call this method after ssl session connected.
	 * @return Data count to be sent.
	 */
	int send(const void* buf, int size);

	/**
	 * @brief Close ssl connection.
	 */
	void close();

	/**
	 * @brief Get a openssl session.
	 * @return openssl session session pointer
	 */
	SSL *getSSL();
	SSL_CTX* getSSLContext();

	int openSSLClientSock(SSL_CTX *pctx=NULL);

	/**
	 * @brief Open a socket for incoming ssl connection.
	 */
	void openSSLChildSock(int fd, SSL_CTX* psslctx=NULL);

	/**
	 * @brief Set ssl event callback.
	 */
	void setOnSSLListener(ISSLSocketCb *cb);

private:
	void startHandshake();
	void procSSLRead(void);
	void procSSLConnect(void);
	void changeSSLSockEvent(int err, bool bwrite);


private:
	SSL *mSSL;
	SSL_CTX *mSSLCtx;
	bool mSessionConencted;
	ISSLSocketCb *mSSLCallback;
	bool mIsSSLServer;

};

} /* namespace edft */

#endif /* EDSSLSOCKET_H_ */
