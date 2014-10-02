/*
 * EdHttpHdrValue.cpp
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */
#define DBGTAG "hdrvp"
#define DBG_LEVEL DBG_DEBUG

#include <string>
#include "EdHttpHdrValue.h"

#include <string>
#include "../edslog.h"

using namespace std;

namespace edft
{

#define REMOVEWSP(LEN,IDX,PTR) {\
	for(;IDX<LEN && *PTR==' ';IDX++,PTR++);\
	if(IDX>LEN) goto PARSE_END; \
}

#define GETSYM(LEN,IDX,PTR,D1,SYM,SLEN) {\
	REMOVEWSP(LEN,IDX,PTR); \
	SYM = PTR; \
	for(;IDX<LEN && *PTR != D1 ;IDX++,PTR++);\
	SLEN = PTR-SYM;\
	if(IDX>LEN) goto PARSE_END; \
}

#define GETSYM2(LEN,IDX,PTR,D1,D2,SYM,SLEN) {\
	REMOVEWSP(LEN,IDX,PTR); \
	SYM = PTR; \
	for(;IDX<LEN && *PTR != D1 && *PTR != D2  ;IDX++,PTR++);\
	SLEN = PTR-SYM;\
	if(IDX>LEN) goto PARSE_END; \
}

#define GETSYM3(LEN,IDX,PTR,D1,D2,D3,SYM,SLEN) {\
	REMOVEWSP(LEN,IDX,PTR); \
	SYM = PTR; \
	for(;IDX<LEN && *PTR != D1 && *PTR != D2 && *PTR!=D3 ;IDX++,PTR++);\
	SLEN = PTR-SYM;\
	if(IDX>LEN) goto PARSE_END; \
}

EdHttpHdrValue::EdHttpHdrValue()
{
	// TODO Auto-generated constructor stub

}

EdHttpHdrValue::~EdHttpHdrValue()
{
	// TODO Auto-generated destructor stub
}

void EdHttpHdrValue::parse(const char* str, int len)
{
	int idx;
	const char* ptr;
	const char* sym;
	int wn=0;
	int slen=0;
	std::string words[100];
	int mode=0;
	for(idx=0,ptr=str,mode=0;;)
	{
		GETSYM3(len, idx, ptr, ' ', '=',';',sym,slen);
		if(mode==0)
		{
			string s;
			s.append(sym, slen);
			dbgd("tag name=%s", s.c_str());
			GETSYM3(len, idx, ptr, ' ', '=',';', sym,slen);
		}
		else
		{

		}

		REMOVEWSP(len, idx, ptr);
		if(*ptr == '=') {
			string s;
			s.append(sym, slen);
			dbgd("sym=%s", s.c_str());
			mode=1;

		} else if(*ptr == ';') {
			mode=0;
		}

	}
	PARSE_END:
	return;
}

} /* namespace edft */
