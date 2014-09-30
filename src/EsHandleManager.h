/*
 * EsHandleManager.h
 *
 *  Created on: 2013. 12. 29.
 *      Author: netmind
 */

#ifndef ESHANDLEMANAGER_H_
#define ESHANDLEMANAGER_H_

#include "EdType.h"
#include "EdMutex.h"
#include "EsList.h"

namespace edft {


struct handleinfo
{
	u32 handle;
	u16 index;
	void* obj;
};

template<typename T>
class EsHandleManager: public EdMutex
{
public:

	u16 mMaxHandle;
	EsList<handleinfo> mHandleFreeList;
	EsList<handleinfo>::Node *mRes;
	u16 mHandleSeed;
	bool mIsObjAlloc;
	//EsMutex mLock;


public:
	EsHandleManager(u16 max, bool isobjalloc = false)
	{
		mHandleSeed = u16((u64) this) >> 2;
		mMaxHandle = max;
		mIsObjAlloc = isobjalloc;
		mRes = new EsList<handleinfo>::Node[max];

		int i;
		for (i = 0; i < mMaxHandle; i++)
		{

			mRes[i].obj.handle = 0;
			mRes[i].obj.index = i;
			if (isobjalloc)
				mRes[i].obj.obj = new T;
			else
				mRes[i].obj.obj = NULL;
			mHandleFreeList.add(mRes + i);
		}

	}
	;
	virtual ~EsHandleManager()
	{
		if (mIsObjAlloc == true)
		{
			for (int i = 0; i < mMaxHandle; i++)
			{
				if (mRes[i].obj.obj != NULL)
					delete mRes[i].obj.obj;
			}
		}
		delete[] mRes;
	}
	;

	u32 allocHandle(void)
	{
		EsList<handleinfo>::Node *p = mHandleFreeList.pop_head();
		if (p)
		{
			if (++mHandleSeed == 0)
				mHandleSeed = 1;
			p->obj.handle = (mHandleSeed << (sizeof(mHandleSeed) * 8)) | p->obj.index;
			return p->obj.handle;
		}
		return 0;
	}

	void freeHandle(u32 handle)
	{

		EsList<handleinfo>::Node *p = HANDLE_TO_OBJ(handle, mRes, obj.handle, mMaxHandle);
		if (p)
		{
			p->obj.handle = 0;
			mHandleFreeList.add(p);
		}

	}

	void setObject(u32 handle, T* obj)
	{
		EsList<handleinfo>::Node *p = HANDLE_TO_OBJ(handle, mRes, obj.handle, mMaxHandle);
		if (p)
		{
			p->obj.obj = obj;
		}

	}

	T* getObject(u32 handle)
	{
		EsList<handleinfo>::Node *p = HANDLE_TO_OBJ(handle, mRes, obj.handle, mMaxHandle);
		if (p)
			return (T*) (p->obj.obj);
		else
			return NULL;
	}

	int getHandleCount(void)
	{
		return mMaxHandle - mHandleFreeList.getSize();
	}

#if 0
	void lock(void)
	{
		mLock.lock();
	}

	void unlock(void)
	{
		mLock.unlock();
	}
#endif

};

} // namespace edft

#endif /* ESHANDLEMANAGER_H_ */
