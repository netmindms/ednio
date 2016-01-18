/*
 * EdMq.h
 *
 *  Created on: Jun 24, 2015
 *      Author: netmind
 */

#ifndef SRC_EDMQ_H_
#define SRC_EDMQ_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string>
#include <functional>
#include "EdEvent.h"

using namespace std;

namespace edft {



class EdMq: public EdEvent {
public:
	typedef function<void (int)> Listener;
	EdMq();
	virtual ~EdMq();

	/*
	 * @brief open message queue as source
	 * @param name queue name. start with '/'
	 * @return if it is successful, return fd of message queue or return -1
	 * @remark as default, do not watch any event.
	 */
	int open(const char* name);

	/*
	 * @brief create and open message queue as sink
	 * @param name queue name to create.
	 * @param msgsize size of a message.
	 * @param maxmsg capacity of message count
	 * @param exist_err if true, fail when queue already exists.
	 * @return If it is successful, return fd of message queue or return -1
	 * @remark As default, watch just read event.
	 */
	int create(const char* name, long msgsize, long maxmsg, bool exist_err=false);
	int open(const char* name , int oflag, mode_t mode, struct mq_attr *attr);
	int send(const char* msg, size_t len, unsigned prio=0);
	int send(const string& s, unsigned prio=0);

	/*
	 * @brief receive a message from queue
	 * @param buf buffer to read a message from queue
	 * @param prio priority
	 * @return If successful, return read count or -1
	 * @warn Buffer size for reading must be greater than message queue size.
	 */
	int recv(char* buf, unsigned *prio);
	int recv(char* buf);
	string recvString(unsigned * prio=nullptr);
	int getAttr(struct mq_attr &attr);
	string dumpAttr();
	void close();
	void unlink();
	void setOnListener(Listener lis);
	static void unlink(const char* name);
	long getMsgSize() const;
	long getMaxMsg() const;
private:
	mqd_t mMqd;
	string mQueueName;
	Listener mLis;
	struct mq_attr mQAttr;

private:
	virtual void OnEventRead(void) final override;
	virtual void OnEventWrite(void)final override;
	virtual void OnEventHangup(void) final override;
};

} /* namespace edft */


#endif /* SRC_EDMQ_H_ */
