/*
 * EdHttp.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: netmind
 */
#include "../config.h"


#include <stdio.h>
#include <time.h>
#include <unordered_map>
#include <string>
#include <string.h>

using namespace std;

static bool _gHttpServerInit=false;

static char _monthString[][4] ={ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char _dayString[][4] = {  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static unordered_map<string, int> mHdrDic;

void EdHttpInit() {
	if(_gHttpServerInit==false) {

	}
}

void es_get_httpDate(char* buf)
{
	time_t t;
	struct tm tmdata;
	time(&t);
	gmtime_r(&t, &tmdata);
	//Tue, 08 Jul 2014 00:09:47 GMT
	sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT", _dayString[tmdata.tm_wday],
			tmdata.tm_mday, _monthString[tmdata.tm_mon], 1900+tmdata.tm_year, tmdata.tm_hour, tmdata.tm_min, tmdata.tm_sec);
}



const char* es_get_http_desp(char* code)
{
	if(!strcmp(code, "200"))
		return "OK";
	if(!strcmp(code, "301"))
		return "Moved Permanently";
	if(!strcmp(code, "302"))
		return "Found";
	if(!strcmp(code, "307"))
		return "Temporary Redirect";
	if(!strcmp(code, "400"))
		return "Bad Request";
	if(!strcmp(code, "401"))
		return "Unauthorized";
	if(!strcmp(code, "402"))
		return "Payment Required";
	if(!strcmp(code, "403"))
		return "Forbidden";
	if(!strcmp(code, "404"))
		return "Not Found";
	if(!strcmp(code, "405"))
		return "Method Not Allowed";
	if(!strcmp(code, "406"))
		return "Not Acceptable";
	if(!strcmp(code, "407"))
		return "Proxy Authentication Required";
	if(!strcmp(code, "408"))
		return "Request Timeout";
	if(!strcmp(code, "415"))
		return "Unsupported Media Type";
	if(!strcmp(code, "429"))
		return "Too Many Requests";
	if(!strcmp(code, "600"))
		return "Server Error";
	return "";
}


