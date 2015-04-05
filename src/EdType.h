/*
 * EdType.h
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */

#ifndef EDTYPE_H_
#define EDTYPE_H_
#include "ednio_config.h"

#include <functional>
#include <memory>
#include <stdint.h>

namespace edft
{

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define ESMIN(A, B) ( (A)<(B) ? (A):(B) )
#define ESMAX(A, B) ( (A)>(B) ? (A):(B) )

#define HIDX(H) (0x0000ffff & H)
#define HANDLE_TO_OBJ(H, OBJS, FILED_NAME, MAX) (HIDX(H) >= MAX ? NULL : ( OBJS[HIDX(H)].FILED_NAME==H ? &OBJS[HIDX(H)] : NULL ) )

#define CHECK_DELETE_OBJ(PTR) { if(PTR != NULL) { delete PTR;PTR=NULL;} }
#define CHECK_FREE_MEM(PTR) { if(PTR != NULL) { free(PTR);PTR=NULL;} }

typedef struct
{
	void* buf;
	int size;
	bool takeBuffer;
} EdBufferInfo;

typedef struct
{
	u8 frameIndi;
	union
	{
		struct
		{
			u16 version;
			u16 type;
		};
		u32 sid;
	};
	u8 flag;
	u32 len;
} spdy_frame_hdr_t;


struct charmalloc_deallocator {
	void operator()(char* ptr) const {
		free(ptr);
	}
};

typedef std::unique_ptr<char, charmalloc_deallocator> upmChar;

typedef std::function<void ()> lfvv;
typedef std::function<void (int)> lfvi;
typedef std::function<void (int, int)> lfvii;
typedef std::function<void (uint32_t)> lfvu;

}

#endif /* EDTYPE_H_ */
