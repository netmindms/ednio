/*
 * EdSocketChannel.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: netmind
 */
#define DBGTAG "sckch"
#define DBG_LEVEL DBG_WARN

#include "edslog.h"
#include "EdSocketChannel.h"

namespace edft
{

EdSocketChannel::EdSocketChannel()
{
	mMaxChunk = 0;
	mCurChk = NULL;
	mChannelCb = NULL;
}

EdSocketChannel::~EdSocketChannel()
{
	closeChannel();
}

void EdSocketChannel::OnWrite(void)
{
	dbgd("*** write event...");
	ChunkInfo *chk = mCurChk;
	for (; chk;)
	{
		int retVal = tx();
		if (retVal == SR_COMPLETE)
		{
			// 수정 내용
			//    OnTxComplete 안에서 close하거나 object를 해제할 경우를 대비해 OnTxComplete() 호출을 가장 나중에 하도로 수정.
			//     검증 필요....
			u8* data = chk->data;
			void* user = chk->user;
			if (getFd() >= 0)
			{
				//ch->mChks->put_empty_nolock(chk);
				mFreeChks.push_back(chk);

				// transmit next pending packet
				dbgd("qued chks size=%d", mChks.size());
				if (!mChks.empty())
				{
					mCurChk = mChks.front();
					mChks.pop_front();
				}
				else
				{
					mCurChk = NULL;
				}

				if (mCurChk)
				{
					chk = mCurChk;
				}
				else
				{
					dbgd("change evt read.....");
					changeEvent(EVT_READ);
					chk = NULL;
				}
			}
			OnTxComplete(data, user);

		}
		else
		{
			if (retVal == SR_FAIL)
			{
				// no operation
			}
			break;
		}
	}
}

int EdSocketChannel::sendPacket(const void* buf, int len, const void* user, bool takebuf)
{
	int retVal;
	ChunkInfo *chk;
	dbgd("free chunk cnt=%d", mFreeChks.size());
	if (!mFreeChks.empty())
	{
		chk = mFreeChks.front();
		memset(chk, 0, sizeof(ChunkInfo));
		mFreeChks.pop_front();
	}
	else
	{
		chk = NULL;
	}

	if (!chk)
	{
		return SR_BUFFULL;
	}

	chk->curCnt = 0;
	if (!takebuf)
	{
		chk->data = (u8*) malloc(len);
		chk->allocated = true;
		memcpy(chk->data, buf, len);
	}
	else
	{
		chk->data = (u8*) buf;
		chk->allocated = false;
	}
	chk->size = len;
	chk->user = (void*) user;

	if (mCurChk)
	{
		mChks.push_back(chk);
		return SR_CONTINUE;
	}
	else
	{
		mCurChk = chk;
		retVal = tx();
		if (retVal == SR_COMPLETE)
		{
			mFreeChks.push_back(mCurChk);
			mCurChk = NULL;
		}
		else
		{
			changeEvent(EVT_READ | EVT_WRITE);
		}
	}
	return retVal;
}

void EdSocketChannel::closeChannel()
{
	dbgd("close channel.......");
	if (mCurChk)
	{
		if (mCurChk->allocated)
		{
			free(mCurChk->data);
			mCurChk->data = NULL;
			mCurChk->allocated = false;
		}
		delete mCurChk;
	}

	for (auto itr = mChks.begin(); itr != mChks.end(); itr++)
	{
		if ((*itr)->allocated)
		{
			free((*itr)->data);
		}

		delete (*itr);
	}
	mCurChk = NULL;
	mChks.clear();

	for (auto itr = mFreeChks.begin(); itr != mFreeChks.end(); itr++)
	{
		if ((*itr)->allocated)
		{
			free((*itr)->data);
		}

		delete (*itr);
	}
	mFreeChks.clear();

	close();
}

int EdSocketChannel::init(int maxchk)
{
	for (int i = 0; i < maxchk; i++)
	{
		ChunkInfo* chk = new ChunkInfo;
		memset(chk, 0, sizeof(ChunkInfo));
		mFreeChks.push_back(chk);
	}

	mMaxChunk = maxchk;
	return 0;
}

void EdSocketChannel::OnTxComplete(u8* buf, void* user)
{
	if(mChannelCb)
		mChannelCb->IOnSocketChannelBuffer(this, buf, user);
}

int EdSocketChannel::tx(void)
{
	int retVal;
	int remain = mCurChk->size - mCurChk->curCnt;
	int wcnt = send(mCurChk->data + mCurChk->curCnt, remain);

	dbgd("tx write, size=%d, written=%d", remain, wcnt);
	if (wcnt == remain)
	{
		retVal = SR_COMPLETE;
		mCurChk->curCnt += remain;
		if (mCurChk->allocated)
		{

			free(mCurChk->data);
			mCurChk->data = NULL;
			mCurChk->allocated = false;
		}
	}
	else if (wcnt > 0)
	{
		mCurChk->curCnt += wcnt;
		retVal = SR_CONTINUE;
	}
	else
	{
		// This is not an error
		dbge("### write fail, ret=%d", wcnt);
		retVal = SR_CONTINUE;
	}
	return retVal;
}


void EdSocketChannel::setChannelCallback(ISocketChannelCb* cb)
{
	mChannelCb = cb;
}

} /* namespace edft */
