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

class EdMdbCnn;

class EdMdbQueryBase
{
public:
	virtual int IOnQueryContinue(int waitevt)=0;
};

} /* namespace edft */

#endif /* EDMDBQUERYBASE_H_ */
