/*
 * test_etc.cpp
 *
 *  Created on: Nov 10, 2015
 *      Author: netmind
 */

#include "../ednio/EdNio.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <random>
#include <chrono>
#include "../ednio/EsList.h"
using namespace edft;
using namespace std;

class MyObj {
public:
	virtual ~MyObj() {
		num = 0;
	}
	int num;
};

TEST(etc, list) {
	EsList<MyObj> objlist;
	auto *ptr = objlist.allocNode();
	ptr->obj.num = 100;
	objlist.add(ptr);
	auto *next_node = objlist.next(ptr);
	ASSERT_EQ(next_node, nullptr);

	ptr = objlist.allocNode();
	ptr->obj.num = 200;
	objlist.add(ptr);

	ptr = objlist.allocNode();
	ptr->obj.num = 300;
	objlist.add(ptr);

	{
		int tv[] = { 100, 200, 300, -1 };
		int i = 0;
		for (auto node = objlist.head(); node; node = objlist.next(node)) {
			ASSERT_EQ(tv[i++], node->obj.num);
		}
	}

	{
		// remove 200
		auto tnode = objlist.head();
		tnode = objlist.next(tnode);
		objlist.del(tnode);
		objlist.freeNode(tnode);
		int tv[] = { 100, 300, -1 };
		int i = 0;
		for (auto node = objlist.head(); node; node = objlist.next(node)) {
			ASSERT_EQ(tv[i++], node->obj.num);
		}
	}

	// free
	for (;;) {
		auto node = objlist.pop_head();
		if (!node)
			break;
		objlist.freeNode(node);
	}
	ASSERT_EQ(objlist.getSize(), 0);
}
