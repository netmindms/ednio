/*
 * EdMdbQueryBase.h
 *
 *  Created on: Oct 14, 2014
 *      Author: netmind
 */

#ifndef EDMDBQUERYBASE_H_
#define EDMDBQUERYBASE_H_

namespace edft
{

class EdMdbQueryBase
{
public:
	//EdMdbQueryBase();
	//virtual ~EdMdbQueryBase();
	virtual int query(const char* qs)=0;
	virtual int queryContinue(int waitevt)=0;
};

} /* namespace edft */

#endif /* EDMDBQUERYBASE_H_ */
