/*
 * EdNio.h
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */

#ifndef EDNIO_H_
#define EDNIO_H_

#include "ednio_config.h"

#include "EdType.h"
#include "EdConst.h"
#include "EdContext.h"
#include "EdTask.h"
#include "EdSocket.h"
#include "EdSocketChannel.h"
#include "EdPipe.h"
#include "EdTimer.h"
#include "EdTime.h"
#include "EdSmartSocket.h"
#if USE_SSL
#include "edssl/EdSSLContext.h"
#include "edssl/EdSSLSocket.h"
#endif



namespace edft {

#define EDNIO_VER "0.5.0"

const char* EdNioGetVer();
int EdNioInit();


} // namespace edft


#endif /* EDNIO_H_ */
