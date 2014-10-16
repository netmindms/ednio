/*
 * EdMdbCmd.h
 *
 *  Created on: Oct 16, 2014
 *      Author: netmind
 */

#ifndef EDMDBCMD_H_
#define EDMDBCMD_H_
#include <string>
using namespace std;

namespace edft
{
class EdMdbCnn;

class EdMdbCmd
{
public:
	EdMdbCmd(EdMdbCnn* pcnn, const char* str);
	virtual ~EdMdbCmd();
private:
	string mCmd;
	EdMdbCnn *mCnn;
};

} /* namespace edft */

#endif /* EDMDBCMD_H_ */
