/*
 * Eslist.h
 *
 *  Created on: Feb 10, 2014
 *      Author: khkim
 */

#ifndef __EDLISTH__
#define __EDLISTH__

#include <pthread.h>
#include "EdType.h"
#include "edslog.h"

namespace edft
{

template<typename T>
class EsList
{


public:
	class Node
	{
		friend class EsList;
	private:
		Node* prev, *next;
		EsList* list;
		u32 seed;
	public:
		T obj;
		void remove(void)
		{
			list->del(this);
		}
	};
private:
	Node* mphead;
	Node* mptail;
	u32 mSeed;
	u32 mSize;

public:

	EsList()
	{
		mSeed = u32(((u64) this) >> 2);
		mSize = 0;
		mphead = mptail = NULL;
	}
	;

	void add(Node* pnode)
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
		pnode->list = this;
		mSize++;
	}

	void insert(Node* prevnode, Node* pnode)
	{
		if (prevnode->seed != mSeed)
			return;
		pnode->prev = prevnode;
		pnode->next = prevnode->next;
		prevnode->next = pnode;
		if (pnode->next)
			pnode->next->prev = pnode;
		pnode->seed = mSeed;
		pnode->list = this;
		mSize++;
	}

	void del(Node* pnode)
	{
		if (pnode->seed != mSeed)
			return;
		Node* cp = pnode->prev;
		Node* cn = pnode->next;
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

		pnode->seed = 0;
		pnode->list = NULL;
		mSize--;
	}

	static Node* allocNode(void)
	{
		return new Node();
	}

	static void freeNode(Node* pn)
	{
		delete pn;
	}

	Node* head(void)
	{
		return mphead;
	}

	Node* tail(void)
	{
		return mptail;
	}

	Node* pop_head(void)
	{
		Node* ret = mphead;
		if (mphead)
			del(mphead);
		return ret;
	}

	Node* pop_tail(void)
	{
		Node* ret = mptail;
		if (mptail)
			del(mptail);
		return ret;
	}

	Node* next(Node* pnode)
	{
		return pnode->next;
	}

	Node* prev(Node* pnode)
	{
		return pnode->prev;
	}

	int getSize(void)
	{
		return mSize;
	}

};

} /* namespace edft */

#endif /* EDIST_H_ */
