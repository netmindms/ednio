/*
 * EsHttp.h
 *
 *  Created on: Jul 8, 2014
 *      Author: netmind
 */

#ifndef EDHTTP_H_
#define EDHTTP_H_

#define HTTPHDR_DATE "Date"
#define HTTPHDR_SERVER "Server"
#define HTTPHDR_CONTENT_LEN "Content-Length"
#define HTTPHDR_CONTENT_TYPE "Content-Type"

void es_get_httpDate(char* buf);
const char* es_get_http_desp(char* code);

#endif /* EDHTTP_H_ */
