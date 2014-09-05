/*
 * IUriController.h
 *
 *  Created on: Jul 7, 2014
 *      Author: netmind
 */

#ifndef IURICONTROLLER_H_
#define IURICONTROLLER_H_

class EsHttpTrans;

class IUriControllerCb
{
public:
	virtual void IOnNewHttpTrans(u32 hcnn, u32 htrans, EsHttpTrans* ptrans)=0;
	virtual void IOnCloseHttpTrans(u32 hcnn, u32 htrans)=0;
	//virtual void IOnMsgBody(u32 hcnn, u32 htrans, EsHttpTrans* ptrans, void *buf, int len)=0;
};

#endif /* IURICONTROLLER_H_ */
