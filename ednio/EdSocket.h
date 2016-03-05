/*
 * EdSocket.h
 *
 *  Created on: Jun 11, 2014
 *      Author: khkim
 */

#ifndef EDSOCKET_H_
#define EDSOCKET_H_
#include "ednio_config.h"

#include <string>
#include <functional>
#include <errno.h>
#include <sys/un.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "EdEvent.h"
#include "EdType.h"

namespace edft
{

#define SOCK_TYPE_TCP 0
#define SOCK_TYPE_UDP 1
#if 1
#define SOCK_TYPE_UNIXSTREAM 2
#define SOCK_TYPE_UNIXDGRAM 3
#endif

typedef enum
{
	SOCK_STATUS_DISCONNECTED = 0,
	SOCK_STATUS_CONNECTING,
	SOCK_STATUS_CONNECTED,
	SOCK_STATUS_LISTENING,
} EDSOCK_STATUS_E;

typedef enum
{
	SOCK_EVENT_CONNECTED = 1 << 0,
	SOCK_EVENT_DISCONNECTED = 1 << 1,
	SOCK_EVENT_READ = 1 << 2,
	SOCK_EVENT_WRITE = 1 << 3,
	SOCK_EVENT_INCOMING_ACCEPT = 1 << 4,
} EDSOCK_EVENT_E;


class EdSocket;



class EdSocket: public EdEvent
{
//public:
//	class ISocketCb {
//		public: virtual void IOnSocketEvent(EdSocket *psock, int event)=0;
//	};
public:
	typedef std::function<void (int event)> Lis;
	EdSocket();
	virtual ~EdSocket();
	/**
	 * @brief read received data from socket.
	 * @param buf destination buffer
	 * @param size buffer size
	 * @return Real read bytes.
	 * @warning Whenever OnRead or read callback is called, you should call this method. Otherwise, disconnected event cannot be raised.
	 */
	int recv(void *buf, int size);

	ssize_t recvFromUnix(void *buf, int size, std::string *fromaddr);

	ssize_t recvFrom(void* buf, int size, unsigned int *ipaddr=NULL, unsigned short *port=NULL);
	/**
	 * @brief Write data to socket.
	 * @param buf
	 * @param size
	 * @return Real written bytes
	 */
	int send(const void *buf, int size);

	int sendto(const char* destaddr, unsigned int addrlen, const void* buf, int len);
	int sendto(const char* destaddr, const void* buf, int len);
	int sendto(uint32_t ip, int port, const void* buf, int len);

	/**
	 * @brief Connect to peer socket.
	 * @param ip
	 * @param port
	 * @return
	 */
	int connect(const char* ip, int port, Lis lis=nullptr);
	int connect(uint32_t ip, int port, Lis lis=nullptr);

	/**
	 * @brief Disconnect conneciton or close socket.
	 */
	void close();

	/**
	 * @brief listen socket.
	 * @param port
	 * @param ip
	 * @return if successful, return 0
	 */
	int listenSock(int port, const char* ip = "0.0.0.0", Lis lis=nullptr);

	/**
	 * @brief bind socket to specified address.
	 * @param port
	 * @param ip
	 * @return If successful, return 0
	 */
	int bindSock(int port, const char* ip = "0.0.0.0");


	/**
	 * @brief accept incoming connection from listening socket.
	 * @return child socket fd
	 */
	int accept();

	/**
	 * @brief Open child socket from the child socket fd accepted.
	 * @param fd returned by accept()
	 */
	void openChildSock(int fd, Lis lis=nullptr);

	/**
	 *
	 * @param pchild
	 * @param cb
	 */
	int acceptSock(EdSocket* pchild, Lis lis=nullptr);

	/**
	 * @brief Reject incoming connection on listening socket.
	 * @remark This is available for listen socket
	 */
	void rejectSock(void);

	/**
	 * @brief Get socket status
	 * @return socket status
	 */
	int getStatus(){return mStatus;};
	void getPeerAddr(char *ipaddr, u16 *port);

	virtual void OnEventRead(void) final override;
	virtual void OnEventWrite(void)final override;
	virtual void OnEventHangup(void) final override;

	/**
	 * @brief Called when read event is activated
	 */

	virtual void OnRead(void);

	/**
	 * @brief Called when connection is disonnected.
	 * @warning For this event, you make sure to call recv() whenever OnRead event occur. Because ednio recognize the disconnection from read() return value.
	 */
	virtual void OnDisconnected(void);

	/**
	 * @brief
	 */
	virtual void OnWrite(void);

	/**
	 * @brief Called when connected with peer.
	 */
	virtual void OnConnected(void);

	/**
	 * @brief
	 */
	virtual void OnIncomingConnection(void);

	/**
	 * @brief Open a socket specified by type
	 * @param type
	 * @return
	 */
	int openSock(int type, Lis lis=nullptr);

	/**
	 * @brief Set socket interface callback.
	 * @param cb
	 */
	void setOnListener( Lis dg);

	void setNoTimewait();
private:
	void clearInternal();
protected:
	void postReserveDisconnect();

private:
	int mStatus;
	int mType;
	int mFamily;
	int mRaiseDisconnect;
	bool mIsListen;
	bool mIsBinded;
	Lis mLis;

};

} /* namespace edft */

#endif /* EDSOCKET_H_ */
