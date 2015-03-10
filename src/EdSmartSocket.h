/*
 * EdSSLSocket.h
 *
 *  Created on: Jul 31, 2014
 *      Author: netmind
 */

#ifndef EDSMARTSOCKET_H_
#define EDSMARTSOCKET_H_

#include "ednio_config.h"

#include <vector>
#include <functional>

#if USE_SSL
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include "edssl/EdSSLContext.h"
#endif

#include "EdSocket.h"

using namespace std;

namespace edft
{

enum
{
	NETEV_DISCONNECTED, NETEV_CONNECTED, NETEV_READABLE, NETEV_WRITABLE,
};

enum
{
	SEND_FAIL = -1, SEND_OK = 0, SEND_PENDING,
};

enum
{
	SOCKET_NORMAL = 0, SOCKET_SSL = 1,
};

class EdSmartSocket;

typedef function<void (EdSmartSocket&, int event)> SmartSocketLis;

class EdSmartSocket
{
private:
	class RawSocket: public EdSocket
	{
		friend class EdSmartSocket;
	private:
		RawSocket(EdSmartSocket* smt) {
			mSmartSock = smt;
		};
		void OnRead() final override;
		void OnWrite() final override;
		void OnConnected() final override;
		void OnDisconnected() final override;
		EdSmartSocket* mSmartSock;
	};
	void fireRead();
	void fireWrite();
	void fireConnected();
	void fireDisconnected();

public:
	EdSmartSocket();
	virtual ~EdSmartSocket();



	// TODO: alpn support -	string getSelectedAlpnProto();

	virtual void OnSSLConnected();
	virtual void OnSSLDisconnected();
	virtual void OnSSLRead();
	// TODO: alpn support - virtual void OnAlpnSelect(int* selecton_idx, const vector<string> &protos);

	/*
	 virtual void OnNetConnected();
	 virtual void OnNetDisconnected();
	 virtual void OnNetRead();
	 virtual void OnNetSendComplete();
	 */
	int openClient(int mode = 0);
	int openChild(int fd, int mode = 0);

	int connect(const string &addr, int port);

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
	int sendPacket(const void* buf, int size, bool takebuffer = false);

	/**
	 * @brief Close ssl connection.
	 */
	void close();
	/**
	 * @brief Set ssl event callback.
	 */
	void setOnListener(SmartSocketLis lis);
	bool isWritable();
	void reserveWrite();
	// TODO: alpn	void setAlpnProtocols(const vector<string> &protocols);

private:
	void procNormalOnWrite();
#if 0 // alpn support
	static int sm_ssl_alpn_cb(SSL *ssl,
			const unsigned char **out,
			unsigned char *outlen,
			const unsigned char *in,
			unsigned int inlen,
			void *arg);
#endif
	int mMode; // 0: Normal mode, 1: ssl mode
	void* mPendingBuf;
	int mPendingWriteCnt;
	int mPendingSize;
	RawSocket mSock;

#if USE_SSL
public:
	/**
	 * @brief Get a openssl session.
	 * @return openssl session session pointer
	 */
	SSL *getSSL();
	SSL_CTX* getSSLContext();

	/**
	 * @brief Open a socket for incoming ssl connection.
	 */

private:
	void startHandshake();
	void procSSLRead(void);
	void procSSLConnect(void);
	void changeSSLSockEvent(int err, bool bwrite);
	void procSSLOnWrite();
	void procSSLErrCloseNeedEnd();
	int openSSLClientSock(EdSSLContext *pctx = NULL);
	void openSSLChildSock(int fd, EdSSLContext* psslctx = NULL);
	SSL *mSSL;
	SSL_CTX *mSSLCtx;
	EdSSLContext *mEdSSLCtx;
	bool mSessionConencted;
	bool mIsServer;
	int mSSLWantEvent;
	// TODO: alpn support
	//string mAlpnSelectProto;
	SmartSocketLis mOnLis;

#endif

};

} /* namespace edft */

#endif /* EDSSLSOCKET_H_ */
