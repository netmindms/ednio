#ifndef __EDCONTEXTH__
#define __EDCONTEXTH__


#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event.h>

//#define USE_LIBEVENT


#define MAX_GET_EVENTS 100

#define EDM_INIT 1
#define EDM_CLOSE 2
#define EDM_TIMER 3
#define EDM_SOCKET 4
#define EDM_EXIT 999
#define EDM_USER 1000

#define OBJECT_MSG(P1, P2) ((void*)( P1 | ((uint64_t)P2<<32)))
#define OBJECT_MSG_P1(OBJ) ( (uint32_t)((uint64_t)OBJ) )
#define OBJECT_MSG_P2(OBJ)   ( (uint32_t)((uint64_t)OBJ >> 32) )
enum { MODE_EDEV=0, MODE_LIBEVENT=1, };

namespace edft {



struct EdContext;
struct edevt_t;

typedef int (*EVTMSG_PROC)(EdContext* pctx, uint16_t msg, uint32_t p1, uint32_t p2);

struct EdContext
{
	int epfd;
	int mode; // MODE_EDEV==0, MODE_LIBEVENT==1
	int exit_flag;
	int opened;

	int evt_count;

	void *user;

#ifdef USE_LIBEVENT
	event_base *eventBase;
#endif

	// for debugging
	int evt_alloc_cnt;
	int timer_alloc_cnt;
} ;

extern __thread EdContext *_tEdContext;

typedef struct edevt_t
{
	int fd;
	void* user;
	EdContext *pEdCtx;
	void (*evtcb)(edevt_t* pevt, int fd, int events);
} edevt_t;

typedef void (*EVENTCB)(edevt_t* pevt, int fd, int events);

}

#endif