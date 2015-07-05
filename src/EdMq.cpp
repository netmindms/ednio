/*
 * EdMq.cpp
 *
 *  Created on: Jun 24, 2015
 *      Author: netmind
 */

#define DBG_LEVEL DBG_DEBUG
#define DBGTAG "MSGQU"

#include <errno.h>
#include <sstream>
#include <streambuf>
#include <algorithm>
#include "edslog.h"
#include "EdMq.h"

namespace edft {

EdMq::EdMq() {
	mMqd = -1;
	memset(&mQAttr, 0, sizeof(mQAttr));
}

EdMq::~EdMq() {
}

int EdMq::open(const char* name) {
	return open(name, O_RDWR|O_NONBLOCK, S_IRUSR|S_IWUSR, nullptr);
}

void EdMq::close() {
	if(mMqd>0) {
		deregisterEvent();
		mq_close(mMqd);
		mMqd = 0;
	}
}

int EdMq::open(const char* name, int oflag, mode_t mode, struct mq_attr* attr) {
	if(mMqd>0) {
		dbgd("close existing message queue: %d", mMqd);
		mq_close(mMqd);
	}

	if(mode==0) {
		if(oflag & O_CREAT) {
			mode = (S_IWUSR | S_IRUSR);
		}
	}

	mMqd = mq_open(name, oflag, mode, attr);
	if(mMqd<0) {
		dbge("### Error: message queue open error, errno=%d", errno);
		return mMqd;
	}

	mq_getattr(mMqd, &mQAttr);

	dbgd("open msg queue: name=%s, fd=%d", name, mMqd);
	mQueueName = name;
	setFd(mMqd);
	uint32_t eflag=0;
	uint32_t rw = oflag & 0x03;
	if(oflag & O_CREAT) {
		// In case of creating, consider the role as msg sing.
		// so, let the queue watch read event
		eflag = EVT_READ;
	}
	registerEvent(eflag);
	return mMqd;
}


void EdMq::unlink() {
	if(mQueueName.empty() == false) {
		mq_unlink(mQueueName.data());
	}
}


void EdMq::OnEventRead(void) {
	dbgd("on event read fd=%d", getFd());
	if(mLis) {
		mLis(EVT_READ);
	}
}

void EdMq::OnEventWrite(void) {
	dbgd("on write event");
	if(mLis) {
		mLis(EVT_WRITE);
	}
}

void EdMq::OnEventHangup(void) {
	dbgd("on event hangup");
}



void EdMq::unlink(const char* name) {
	mq_unlink(name);
}

int EdMq::create(const char* name, long msgsize, long maxmsg, bool exist_err) {
	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;
	attr.mq_maxmsg = maxmsg;
	attr.mq_msgsize = msgsize;
	attr.mq_curmsgs = 0;
	int oflag = O_RDWR|O_CREAT|O_NONBLOCK;
	if(exist_err)
		oflag |= O_EXCL;
	return open(name, oflag, S_IRUSR|S_IWUSR, &attr);
}


void EdMq::setOnListener(Listener lis) {
	mLis = lis;
}

int EdMq::send(const char* msg, size_t len, unsigned msg_prio) {
	return mq_send(mMqd, msg, len, msg_prio);
}


int EdMq::recv(char* msg, unsigned *prio) {
	return mq_receive(mMqd, msg, (size_t)mQAttr.mq_msgsize, prio);
}

int EdMq::recv(char* msg) {
	return recv(msg, nullptr);
}


string EdMq::recvString(unsigned * prio) {
	string s;
	s.resize(mQAttr.mq_msgsize);
	auto rcnt = recv((char*)s.data(), prio);
	if(rcnt>0) {
		s.resize(rcnt);
	} else {
		s.resize(0);
	}
	return move(s);
}


int EdMq::send(const string& s, unsigned prio) {
	return send(s.data(), s.size(), prio);
}


int EdMq::getAttr(struct mq_attr &attr) {
	return mq_getattr(mMqd, &attr);
}


string EdMq::dumpAttr() {
	struct mq_attr attr;
	auto ret = getAttr(attr);
	if(ret)
		return "";
	char buf[500];
	sprintf(buf, "flags: %0x\n"
				"maxmsg: %ld\n"
				"msgsize: %ld\n"
				"curmsggs: %ld\n",
				attr.mq_flags, attr.mq_maxmsg,
				attr.mq_msgsize, attr.mq_curmsgs
			);
	return string(buf);
}

long EdMq::getMsgSize() const {
	return mQAttr.mq_msgsize;
}

long EdMq::getMaxMsg() const {
	return mQAttr.mq_maxmsg;
}

} /* namespace edft */
