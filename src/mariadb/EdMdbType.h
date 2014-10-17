/*
 * EdMdbType.h
 *
 *  Created on: Oct 17, 2014
 *      Author: netmind
 */

#ifndef EDMDBTYPE_H_
#define EDMDBTYPE_H_

#include <vector>

using namespace std;

namespace edft {

enum {
	MDB_FAIL=-1,
	MDB_COMPLETE=0,
	MDB_CONTINUE,
};


typedef struct {
	void* buf;
	int len;
} mdb_col_t;

typedef vector<mdb_col_t> MDB_ROWS;



} // namespace edft
#endif /* EDMDBTYPE_H_ */
