#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "esp_http_server.h"

typedef struct sta_conn_data {
    char ssid[32];
    char password[64];
    char user_hash[200];
} sta_conn_data;

void start_webserver();
void stop_webserver(httpd_handle_t);

#endif
