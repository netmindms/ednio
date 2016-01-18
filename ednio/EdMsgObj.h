/*
 * EdMsgObj.h
 *
 *  Created on: Sep 1, 2015
 *      Author: netmind
 */

#ifndef SRC_EDMSGOBJ_H_
#define SRC_EDMSGOBJ_H_
#include <memory>
#include <string>

namespace edft {

class EdMsgObj {
public:
	EdMsgObj();
	virtual ~EdMsgObj();
};

typedef std::unique_ptr<EdMsgObj> upEdMsgObj;

} /* namespace edft */

#endif /* SRC_EDMSGOBJ_H_ */
