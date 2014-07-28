/*
 * edslog.cpp
 *
 *  Created on: Dec 24, 2010
 *      Author: netmind
 */

#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// for debugging log
#include "edslog.h"

#define TEMP_LOG_BUF_SIZE (1*1024)

void edlog(const char *tagid, int line, const char *fmtstr, ...)
{
	struct timeval tm;
	va_list ap;

	gettimeofday(&tm, NULL);
	struct tm* ptr_time = localtime(&tm.tv_sec);

	char buf[TEMP_LOG_BUF_SIZE];
	va_start(ap, fmtstr);
	vsnprintf(buf, TEMP_LOG_BUF_SIZE-1, fmtstr, ap);
	va_end(ap);

	printf("%02d:%02d:%02d.%02d [%s]:%-5d %s\n", ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec, (int)(tm.tv_usec/10000), tagid, line, buf);
}
