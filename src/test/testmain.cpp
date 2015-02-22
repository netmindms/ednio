/*
 * testmain.cpp
 *
 *  Created on: Jan 12, 2015
 *      Author: netmind
 */

#include <iostream>
#include <gtest/gtest.h>

using namespace std;

int main(int argc, char* argv[])
{
	cout << "Test main" << endl;
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

