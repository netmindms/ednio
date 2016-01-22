/*
 * EdSignalFd.h
 *
 *  Created on: Apr 4, 2015
 *      Author: netmind
 */

#ifndef EDNIO_EDSIGNALFD_H_
#define EDNIO_EDSIGNALFD_H_

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <functional>
#include "EdType.h"
#include "EdEvent.h"

namespace edft
{

class EdSignalFd: public EdEvent
{
public:
	EdSignalFd();
	virtual ~EdSignalFd();
	void OnEventRead() override final;
	void setOnListener(lfvu lis);
	int setSignal(std::vector<int> mask_list);
	void close();

private:
	lfvu mLis;
};

}
#endif /* EDNIO_EDSIGNALFD_H_ */
