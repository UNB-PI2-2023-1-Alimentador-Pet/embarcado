#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "esp_http_server.h"

void start_webserver();
void stop_webserver(httpd_handle_t);

#endif
