/*
 * EdMdbCmd.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#include "EdMdbCnn.h"
#include "EdMdbCmd.h"


namespace edft
{

EdMdbCmd::EdMdbCmd(EdMdbCnn* pcnn, const char* str)
{
	mCnn = pcnn;
	mCmd = str;
}

EdMdbCmd::~EdMdbCmd()
{
	// TODO Auto-generated destructor stub
}

} /* namespace edft */
