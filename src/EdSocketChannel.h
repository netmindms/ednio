/*
 * EdSocketChannel.h
 *
 *  Created on: Jul 18, 2014
 *      Author: netmind
 */

#ifndef EDSOCKETCHANNEL_H_
#define EDSOCKETCHANNEL_H_
#include "config.h"
#include <list>
#include "EdSocket.h"

using namespace std;

namespace edft
{

class EdSocketChannel: public EdSocket
{
private:
	typedef struct
	{
		u8* data;
		int size;
		void* user;
		int curCnt;
		bool allocated;

	} ChunkInfo;

public:
	enum
	{
		SR_ERRMAX = -100,
		SR_FAIL,
		SR_BUFFULL,
		SR_COMPLETE = 0,
		SR_CONTINUE,
	};
	class ISocketChannelCb {
	public:
		virtual void IOnSocketChannelBuffer(EdSocketChannel *pch, void* buffer, void* user)=0;
	};

public:
	EdSocketChannel();
	virtual ~EdSocketChannel();
	virtual void OnWrite(void);
	virtual void OnTxComplete(u8* buf, void* user);

	int init(int maxchk=1);
	int sendPacket(const void* buf, int len, const void *user = NULL, bool takebuf = false);
	void closeChannel();
	void setChannelCallback(ISocketChannelCb* cb);

private:
	int tx(void);

private:
	int mMaxChunk;
	std::list<ChunkInfo*> mChks;
	std::list<ChunkInfo*> mFreeChks;
	ChunkInfo *mCurChk;

	ISocketChannelCb *mChannelCb;
};

} /* namespace edft */

#endif /* EDSOCKETCHANNEL_H_ */
