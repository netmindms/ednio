/*
 * testmain.cpp
 *
 *  Created on: Jan 12, 2015
 *      Author: netmind
 */

#include <iostream>
#include <gtest/gtest.h>
#include <ednio/EdNio.h>

using namespace std;
using namespace edft;


int main(int argc, char* argv[])
{
	cout << "Test main" << endl;

	::testing::GTEST_FLAG(filter) = "task.msg";

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();


}

