/*
 * EdUrl.cpp
 *
 *  Created on: Sep 6, 2014
 *      Author: netmind
 */
#include "../config.h"

#include <string.h>
#include "EdUrl.h"
#include "http_parser.h"

namespace edft
{

EdUrl::EdUrl()
{
	// TODO Auto-generated constructor stub

}

EdUrl::~EdUrl()
{
	// TODO Auto-generated destructor stub
}

int EdUrl::parse(const char* str, int len)
{
	http_parser_url url;
	if (len == 0)
		len = strlen(str);

	int ur = http_parser_parse_url(str, len, 0, &url);
	if( ur != 0 )
		return ur;

//	for (int i = 0; i < UF_MAX; i++)
//	{
//		if (url.field_set & (1 << i))
//		{
//			string f;
//			f.assign(raw + url.field_data[i].off, url.field_data[i].len);
//			dbgd("url parsing: %s = %s", urlis[i], f.c_str());
//		}
//	}
	return 0;// TODO
}

} /* namespace edft */
