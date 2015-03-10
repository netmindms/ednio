/*
 * tglobal.h
 *
 *  Created on: Mar 10, 2015
 *      Author: netmind
 */

#ifndef TEST_TGLOBAL_H_
#define TEST_TGLOBAL_H_


int get_num_fds();

#define FDCHK_S() int _sfd = get_num_fds();
#define FDCHK_E() int _efd = get_num_fds(); assert(_sfd == _efd);

#endif /* TEST_TGLOBAL_H_ */
