/*
 * tglobal.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: netmind
 */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>


int get_num_fds()
{
	int fd_count;
	char buf[300];
	struct dirent *dp;

	snprintf(buf, 256, "/proc/%i/fd/", getpid());

	fd_count = 0;
	DIR *dir = opendir(buf);
	while ((dp = readdir(dir)) != NULL)
	{
		//if(!(dp->d_type & DT_DIR))	logs("file = %s", dp->d_name);
		fd_count++;
	}
	closedir(dir);
	return fd_count;
}

//void fdcheck_start()
//{
//	_gStartFds = get_num_fds();
//
//}
//
//void fdcheck_end()
//{
//	long fdn = get_num_fds();
//	if (_gStartFds != fdn)
//	{
////		logm("### Fail: fd count check error, start=%ld, end=%ld", _gStartFds, fdn);
//		assert(0);
//	}
//}
