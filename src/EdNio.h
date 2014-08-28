/*
 * EdNio.h
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */

#ifndef EDNIO_H_
#define EDNIO_H_
#include "config.h"
#include "EdType.h"
#include "EdContext.h"
#include "EdTask.h"
#include "EdSocket.h"
#include "EdSocketChannel.h"
#include "EdPipe.h"
#include "EdTimer.h"

namespace edft {

const char* EdNioGetVer();
int EdNioInit();
EdTask* getCurrentTask();

#if USE_SSL
int EdSSLInit();
bool EdSSLIsInit();
#endif

}
#endif /* EDNIO_H_ */
