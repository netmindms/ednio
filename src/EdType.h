/*
 * EdType.h
 *
 *  Created on: Jul 19, 2014
 *      Author: netmind
 */

#ifndef EDTYPE_H_
#define EDTYPE_H_
#include "config.h"

#include <stdint.h>

namespace edft {

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

typedef struct {
	void* buf;
	int size;
	bool takeBuffer;
} EdBufferInfo;

}

#endif /* EDTYPE_H_ */
