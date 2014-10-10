/*
 * EdObjList.h
 *
 *  Created on: Feb 10, 2014
 *      Author: khkim
 */

#ifndef __EDOBJLISTH__
#define __EDOBJLISTH__
#include "config.h"

#include <pthread.h>
#include <string.h>
#include "edslog.h"
#include "EdType.h"
#include "EdMutex.h"

namespace edft
{


template<typename T>
class EdObjList : public EdMutex
{


public:
	struct listhdr_t {
		listhdr_t *prev;
		listhdr_t *next;
		u32 seed;
		EdObjList *list;
	};

	struct Node {
		listhdr_t hdr;
		T obj;
	};
#define OBJ2HDR(OBJ) ((listhdr_t*)OBJ-1)
#define HDR2OBJ(HDR) ( (T*)(HDR+1) )

private:
	listhdr_t* mphead;
	listhdr_t* mptail;

	u32 mSeed;
	u32 mSize;

public:

	EdObjList()
	{
		mSeed = u32(((u64) this) >> 2);
		//ESASSERT(mSeed!=0, "seed must not be zero\n");
		mSize = 0;
		mphead = mptail = NULL;
	}
	;

	void push_back(T *objptr)
	{

		listhdr_t* phdr = OBJ2HDR(objptr);
		if(phdr->seed || phdr->list) {
			//ESASSERT(0, "invalid add item...");
			return;
		}

		phdr->seed = mSeed;

		if (mptail == NULL)
		{
			mphead = mptail = phdr;
			phdr->prev = NULL;
			phdr->next = NULL;
		}
		else
		{
			phdr->prev = mptail;
			phdr->next = NULL;
			mptail->next = phdr;
			mptail = phdr;
		}
		phdr->list = this;
		mSize++;
	}

	void insert(T* prevobjptr, T* objptr)
	{
		listhdr_t* prevhdr = (listhdr_t*)prevobjptr-1;
		listhdr_t* phdr = (listhdr_t*)objptr - 1;
		//Node* prevnode = (listhdr_t*)prevobjptr-1;
		//Node* pnode = (listhdr_t*)objptr - 1;

		if (prevhdr->seed != mSeed || phdr->seed || phdr->list)
		{
			//ESASSERT(0, "invalid insert item...");
			return;
		}
		phdr->prev = prevhdr;
		phdr->next = prevhdr->next;
		prevhdr->next = phdr;
		if (phdr->next)
			phdr->next->prev = phdr;
		phdr->seed = mSeed;
		phdr->list = this;
		mSize++;
	}

	void remove(T* objptr)
	{
		//Node* pnode = (listhdr_t*)objptr - 1;
		listhdr_t* phdr = (listhdr_t*)objptr - 1;
		if (phdr->seed != mSeed)
		{
			//ESASSERT(0, "### invalid item for delete from list...");
			return;
		}
		listhdr_t* cp = phdr->prev;
		listhdr_t* cn = phdr->next;
		if (cp)
		{
			cp->next = cn;
		}
		else
		{
			if (cn)
				cn->prev = NULL;
			mphead = cn;
		}

		if (cn)
		{
			cn->prev = cp;
		}
		else
		{
			mptail = cp;
			if (cp)
				cp->next = NULL;
		}

		phdr->seed = 0;
		phdr->list = NULL;
		mSize--;
	}

	static T *allocObj(void)
	{
		Node *retVal = new Node();
		//loge("allocobj=%p", retVal);
		memset(&(retVal->hdr), 0, sizeof(listhdr_t));
		return &(retVal->obj);

	}

	static void freeObj(T* objptr)
	{
		Node* pnode = (Node*)((listhdr_t*)objptr - 1);
		//loge("freeobj=%p", pnode);
		delete  pnode;
	}

	static EdObjList* getList(T* objptr)
	{
		return OBJ2HDR(objptr)->list;
	}

	T* front(void)
	{
		return mphead ? (T*)(mphead+1) : NULL;
	}

	T* end(void)
	{
		return mptail ? (T*)(mptail+1) : NULL;
	}

	T* pop_front(void)
	{
		Node* ret = (Node*)mphead;
		if (mphead)
		{
			remove((T*)(mphead+1));
			return &(ret->obj);
		}
		else
			return NULL;
	}

	T* pop_back(void)
	{
		Node* ret = mptail;
		if (mptail)
		{
			remove((T*)(mptail+1));
			return &(ret->obj);
		}
		else
			return NULL;
	}

	T* next(T* objptr)
	{
		listhdr_t* phdr = (listhdr_t*)objptr - 1;
		return phdr->next ? (T*)(phdr->next+1) : NULL;
	}

	T* prev(T* objptr)
	{
		listhdr_t* phdr = (listhdr_t*)objptr - 1;
		return phdr->prev ? (T*)(phdr->prev+1) : NULL;
	}

	int size(void)
	{
		return mSize;
	}

	void clear()
	{
		mphead = NULL;
		mptail = NULL;

		mSeed = 0;
		mSize = 0;
	}

};

} /* namespace edft */

#endif /* EDOBJLIST_H_ */
