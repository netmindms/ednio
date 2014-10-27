#ifndef __EDSLOGH__
#define __EDSLOGH__
#include "ednio_config.h"
void edlog(const char *tagstr, int lineno, const char *fmtstr, ...);


#define DBG_ERR 1
#define DBG_WARN 2
#define DBG_INFO 4
#define DBG_DEBUG 8
#define DBG_VERBOSE 16
#define DBG_ALL 32

#ifndef DBG_LEVEL
#define DBG_LEVEL DBG_WARN
#endif

#ifndef DBGTAG
#define DBGTAG __FILE__
#endif


#ifndef _DEBUG_NONE
#define edprt(...) edlog(DBGTAG, __LINE__, __VA_ARGS__)
#else
#define edprt(...)
#endif



#define dbge(...) if(DBG_LEVEL>=DBG_ERR) edprt(__VA_ARGS__)
#define dbgw(...) if(DBG_LEVEL>=DBG_WARN) edprt(__VA_ARGS__)
#define dbgd(...) if(DBG_LEVEL>=DBG_DEBUG) edprt(__VA_ARGS__)
#define dbgi(...) if(DBG_LEVEL>=DBG_INFO) edprt(__VA_ARGS__)
#define dbgv(...) if(DBG_LEVEL>=DBG_VERBOSE) edprt(__VA_ARGS__)
#define dbga(...) if(DBG_LEVEL>=DBG_ALL) edprt(__VA_ARGS__)


#endif
