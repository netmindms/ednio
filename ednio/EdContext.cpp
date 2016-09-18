/*
 * EdContext.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: netmind
 */

#include <stdio.h>
#include "EdContext.h"

namespace edft {

#if EDNIO_TLS_PTHREAD == 1
	#include <pthread.h>
	pthread_key_t _gEdContextTLSKey;
	class MODULE_INIT_EDCONTEXT {
	public:
		MODULE_INIT_EDCONTEXT() {
			pthread_key_create(&_gEdContextTLSKey, NULL);
		};
	};
	static MODULE_INIT_EDCONTEXT _gInitModEdContext;
#else
__thread EdContext *_tEdContext;
#endif

}
