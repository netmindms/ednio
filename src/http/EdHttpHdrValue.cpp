/*
 * EdHttpHdrValue.cpp
 *
 *  Created on: Oct 2, 2014
 *      Author: netmind
 */
#define DBGTAG "HHDRV"
#define DBG_LEVEL DBG_WARN

#include <string>
#include "EdHttpHdrValue.h"

#include <string>
#include "../edslog.h"

using namespace std;

namespace edft
{

#define HP_REMOVEWS(LEN,IDX,PTR) {\
	for(;IDX<LEN && *PTR==' ';IDX++,PTR++);\
	if(IDX>LEN) goto PARSE_END; \
}

#define HP_STEP(LEN, IDX, PTR) { \
	PTR++, IDX++;\
	if(IDX>LEN) goto PARSE_END; \
}

#define HP_GETWORD(LEN,IDX,PTR,D1,SYM,SLEN) {\
	HP_REMOVEWS(LEN,IDX,PTR); \
	SYM = PTR; \
	for(;IDX<LEN && *PTR != D1 ;IDX++,PTR++);\
	SLEN = PTR-SYM;\
	if(IDX>LEN) goto PARSE_END; \
}

#define HP_GETWORD2(LEN,IDX,PTR,D1,D2,SYM,SLEN) {\
	HP_REMOVEWS(LEN,IDX,PTR); \
	SYM = PTR; \
	for(;IDX<LEN && *PTR != D1 && *PTR != D2  ;IDX++,PTR++);\
	SLEN = PTR-SYM;\
	if(IDX>LEN) goto PARSE_END; \
}

#define HP_GETWORD3(LEN,IDX,PTR,D1,D2,D3,SYM,SLEN) {\
	HP_REMOVEWS(LEN,IDX,PTR); \
	SYM = PTR; \
	for(;IDX<LEN && *PTR != D1 && *PTR != D2 && *PTR!=D3 ;IDX++,PTR++);\
	SLEN = PTR-SYM;\
	if(IDX>LEN) goto PARSE_END; \
}

EdHttpHdrValue::EdHttpHdrValue()
{

}

EdHttpHdrValue::~EdHttpHdrValue()
{
}

struct tagval
{
	string name;
	string val;
};

void EdHttpHdrValue::parseGeneral(const char* str, int len)
{
	int idx;
	const char* ptr;
	const char* sym;
	int slen = 0;
	const char* v1, *v2;
	int l1, l2;

	for (idx = 0, ptr = str;;)
	{
		HP_GETWORD2(len, idx, ptr, '=', ';', sym, slen);
		if (slen == 0)
		{
			HP_STEP(len, idx, ptr);
			continue;
		}
		v1 = sym, l1 = slen;
		HP_GETWORD2(len, idx, ptr, '=', ';', sym, slen);
		if (*ptr == '=')
		{
			HP_STEP(len, idx, ptr);
			HP_GETWORD2(len, idx, ptr, '"', ';', sym, slen);
			if (*ptr == '"')
			{
				HP_STEP(len, idx, ptr);
				HP_GETWORD(len, idx, ptr, '"', sym, slen);

			}
			if (slen > 0)
			{
				v2 = sym, l2 = slen;
				OnTag(v1, l1, v2, l2);
			}
			HP_STEP(len, idx, ptr);
		}
		else
		{
			OnTag(v1, l1, NULL, 0);
			HP_STEP(len, idx, ptr);
			continue;
		}

	}
	PARSE_END: return;
}

void EdHttpHdrValue::OnTag(const char* name, int nlen, const char* val, int vlen)
{

	//dbgd("name=%s,  val=%s", n.c_str(), v.c_str());
}

void EdHttpHdrValue::parseDate(const char* str, int len)
{
	int idx;
	int slen;
	const char* ptr;
	const char* sym;

	idx = 0, ptr = str, slen=0;
	for (;;)
	{
		HP_GETWORD2(len, idx, ptr, ' ', ',', sym, slen);
		if (slen > 0)
			OnTag(sym, slen, NULL, 0);
		HP_STEP(len, idx, ptr);
	}


	PARSE_END: return;
}



} /* namespace edft */
