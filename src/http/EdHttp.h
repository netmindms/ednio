/*
 * EsHttp.h
 *
 *  Created on: Jul 8, 2014
 *      Author: netmind
 */

#ifndef EDHTTP_H_
#define EDHTTP_H_

#include "../ednio_config.h"

#define HTTPHDR_DATE "Date"
#define HTTPHDR_SERVER "Server"
#define HTTPHDR_CONTENT_LEN "Content-Length"
#define HTTPHDR_CONTENT_TYPE "Content-Type"

#include "EdHttpWriter.h"
#include "EdHttpTask.h"
#include "EdHttpStringReader.h"
#include "EdHttpStringWriter.h"
#include "EdHttpServer.h"
#include "EdHttpTask.h"
#include "EdHttpFileReader.h"
#include "EdHttpFileWriter.h"
#include "EdHdrDate.h"
#include "EdHttpDefMultiPartCtrl.h"
#include "EdHttpUploadCtrl.h"


void es_get_httpDate(char* buf);
const char* es_get_http_desp(char* code);

#endif /* EDHTTP_H_ */
