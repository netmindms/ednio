#ifndef __APPLOGH__
#define __APPLOGH__

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

void levlog(int lev, const char *tagid, int line, const char *fmtstr, ...)
{
	struct timeval tm;
	va_list ap;

	gettimeofday(&tm, NULL);
	struct tm* ptr_time = localtime(&tm.tv_sec);

	char buf[2048];

	int splen = 2 * lev;
	char spbuf[splen + 1];
	memset(spbuf, ' ', splen);
	spbuf[splen] = 0;

	va_start(ap, fmtstr);
	vsnprintf(buf, 4096 - 1, fmtstr, ap);
	va_end(ap);

	printf("%02d:%02d:%02d.%02d [%s]:%-5d %s%s\n", ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec, (int) (tm.tv_usec / 10000), tagid, line, spbuf, buf);
}

#define logs(...) {levlog(1, "SUB  ", __LINE__, __VA_ARGS__); }

#endif
