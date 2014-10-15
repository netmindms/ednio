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
	//EdMdbQueryBase();
	//virtual ~EdMdbQueryBase();
	virtual void setConnection(EdMdbCnn* pcnn)=0;
	virtual int queryStart(const char* qs)=0;
	virtual int queryContinue(int waitevt)=0;
};

} /* namespace edft */

#endif /* EDMDBQUERYBASE_H_ */
