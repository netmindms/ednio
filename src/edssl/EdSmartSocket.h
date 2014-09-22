/*
 * EdSSLSocket.h
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#ifndef EDSMARTSOCKET_H_
#define EDSMARTSOCKET_H_

#include "../config.h"

#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>

#include "../EdSocket.h"

namespace edft
{

enum {
	NETEV_DISCONNECTED,
	NETEV_CONNECTED,
	NETEV_READ,
	NETEV_SENDCOMPLETE,
};

enum {
	SEND_FAIL=-1,
	SEND_OK=0,
	SEND_PENDING,
};

enum {
	SOCKET_NORMAL=0,
	SOCKET_SSL=1,
};


class EdSmartSocket : public EdSocket
{
public:

	class INet {
	public:
		virtual void IOnNet(EdSmartSocket *psock, int event)=0;
	};

public:
	EdSmartSocket();
	virtual ~EdSmartSocket();

	virtual void OnRead();
	virtual void OnWrite();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual void OnSSLConnected();
	virtual void OnSSLDisconnected();
	virtual void OnSSLRead();

	int socketOpen(int mode=0);
	int socketOpenChild(int fd, int mode=0);

	/**
	 * @brief Read data from ssl connection
	 * @remark After ssl session is complete, call this method to read received from ssl connection when OnSSLRead() or IOnSSLSocket(psock, SOCK_EVENT_READ) is called.
	 * @param buf Buffer to read
	 * @param bufsize Buffer size
	 * @return Read read count
	 */

	void sslAccept();

	int recvPacket(void* buf, int size);

	/**
	 * @brief send packet to server.
	 * @remark You should call this method after ssl session connected.
	 * @return Data count to be sent.
	 */
	int sendPacket(const void* buf, int size);

	/**
	 * @brief Close ssl connection.
	 */
	void socketClose();

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
	//void setOnSSLListener(ISSLSocketCb *cb);
	void setOnNetListener(INet* lis);
	bool isWritable();
private:
	void startHandshake();
	void procSSLRead(void);
	void procSSLConnect(void);
	void changeSSLSockEvent(int err, bool bwrite);


private:
	SSL *mSSL;
	SSL_CTX *mSSLCtx;
	bool mSessionConencted;
	//ISSLSocketCb *mSSLCallback;
	bool mIsSSLServer;
	INet* mOnLis;
	int mMode; // 0: Normal mode, 1: ssl mode
	void* mPendingBuf; int mPendingWriteCnt; int mPendingSize;
};

} /* namespace edft */

#endif /* EDSSLSOCKET_H_ */
