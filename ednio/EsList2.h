/*
 * EdList2.h
 *
 *  Created on: Dec 30, 2013
 *      Author: khkim
 *  EsList와의 차이는 다른 template 파일안에서 사용될때 EsList의 경우 inner class인 EsList::Node 가 정의 될 수 없다.
 *  예를들어, EsList<T>::Node *node 와 같이 사용하면 컴파일 에러가 발생한다. 이유는 예상컨대 inner class가 타입이 확정이 안되면 out class가 정의 될 수 없기 때문인것 같다.
 *  이런 경우 EsList2 와 EsListNode를 사용한다.
 */

#ifndef __EDLIST2H__
#define __EDLIST2H__

#include <pthread.h>
#include "EdType.h"
#include "edslog.h"

namespace edft
{

template<typename T>
struct EsListNode
{
	EsListNode* prev;
	EsListNode* next;
	union
	{
		u32 data;
		u64 ldata;
		void* user;
	};
	u32 seed;
	T obj;
	EsListNode(T c)
	{
		obj = c;
		seed = 0, prev = next = NULL;
	}
	;
	EsListNode()
	{
		seed = 0;
		prev = next = NULL;
	}
	;

};

template<typename T>
class EsList2
{

private:
	EsListNode<T>* mphead;
	EsListNode<T>* mptail;
	u32 mSeed;
	u32 mSize;


public:

	EsList2()
	{
		mSeed =  u32( ((u64)this)>>2 );
		mSize = 0;
		mphead = mptail = NULL;
	}
	;

	void add(EsListNode<T>* pnode)
	{
		pnode->seed = mSeed;
		if (mptail == NULL)
		{
			mphead = mptail = pnode;
			pnode->prev = NULL;
			pnode->next = NULL;
		}
		else
		{
			pnode->prev = mptail;
			pnode->next = NULL;
			mptail->next = pnode;
			mptail = pnode;
		}
		mSize++;
	}

	void insert(EsListNode<T>* prevnode, EsListNode<T>* pnode)
	{
		if(prevnode->seed != mSeed)
			return;
		pnode->prev = prevnode;
		pnode->next = prevnode->next;
		prevnode->next = pnode;
		if(pnode->next)
			pnode->next->prev = pnode;
		pnode->seed = mSeed;
		mSize++;
	}

	void del(EsListNode<T>* pnode)
	{
		if (pnode->seed != mSeed)
			return;
		EsListNode<T>* cp = pnode->prev;
		EsListNode<T>* cn = pnode->next;
		if (cp)
		{
			cp->next = cn;
		}
		else
		{
			if(cn)
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
			if(cp)
				cp->next = NULL;
		}

		pnode->seed = 0;
		mSize--;
	}

	EsListNode<T>* head(void)
	{
		return mphead;
	}

	EsListNode<T>* tail(void)
	{
		return mptail;
	}

	EsListNode<T>* pop_head(void)
	{
		EsListNode<T>* ret = mphead;
		if(mphead)
			del(mphead);
		return ret;
	}

	EsListNode<T>* pop_tail(void)
	{
		EsListNode<T>* ret = mptail;
		if(mptail)
			del(mptail);
		return ret;
	}

	EsListNode<T>* next(EsListNode<T>* pnode)
	{
		return pnode->next;
	}

	EsListNode<T>* prev(EsListNode<T>* pnode)
	{
		return pnode->prev;
	}

	int getSize(void)
	{
		return mSize;
	}

};

} /* namespace edft */

#endif /* EDLIST2_H_ */
