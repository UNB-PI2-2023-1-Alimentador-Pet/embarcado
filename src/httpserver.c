#include <stdio.h>
#include <inttypes.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types.h"
#include "hal/esp_flash_err.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_wifi.h"
#include "string.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi_types.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include <string.h>

typedef struct sta_conn_data {
    char ssid[200];
    char password[200];
    char user_hash[200];
} sta_conn_data;

void save_sta_conn_data_into_flash(const sta_conn_data* data) {
    nvs_handle_t handle;
    nvs_open("nvs", NVS_READWRITE, &handle);
    
    nvs_set_str(handle, "wifi_sta_ssid", data->ssid);
    nvs_commit(handle);

    nvs_set_str(handle, "wifi_sta_pass", data->password);
    nvs_commit(handle);

    nvs_set_str(handle, "user_hash", data->user_hash);
    nvs_commit(handle);

    nvs_close(handle);
}

sta_conn_data jsonstr_to_sta_conn_data(const char* jsonstr) {
    sta_conn_data data;    
    cJSON* obj = cJSON_Parse(jsonstr);

    cJSON* ssid_obj = cJSON_GetObjectItem(obj, "ssid");
    cJSON* pass_obj = cJSON_GetObjectItem(obj, "password");
    cJSON* user_hash_obj = cJSON_GetObjectItem(obj, "user_hash");

    strcpy(data.ssid, cJSON_GetStringValue(ssid_obj));
    strcpy(data.password, cJSON_GetStringValue(pass_obj));
    strcpy(data.user_hash, cJSON_GetStringValue(user_hash_obj));

    cJSON_free(obj);

    ESP_LOGI("CJSON", "ssid: %s\npass: %s\n user_hash: %s\n", data.ssid, data.password, data.user_hash);

    return data;
}

esp_err_t post_wifi_sta_conn_data_handler(httpd_req_t *req) {
    char* buffer = malloc(req->content_len);
    httpd_req_recv(req, buffer, req->content_len);

    ESP_LOGI("HTTP", "body(len %d): \n%s\n", req->content_len, buffer);

    sta_conn_data sta_data = jsonstr_to_sta_conn_data(buffer);
    save_sta_conn_data_into_flash(&sta_data);

    const char resp[] = "Ok!";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    // Try sta connection
    // Send response
    return ESP_OK;
}

// esp_err_t get_handler(httpd_req_t *req) {
//     const char resp[] = "Hello world";
//     httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
//     return ESP_OK;
// }

// static httpd_uri_t uri_get = {
//     .uri = "/",
//     .method = HTTP_GET,
//     .handler = get_handler,
//     .user_ctx = NULL
// }

static httpd_uri_t uri_post_wifi_sta_conn = {
    .uri = "/wifi",
    .method = HTTP_POST,
    .handler = post_wifi_sta_conn_data_handler,
    .user_ctx = NULL
};

static httpd_handle_t server = NULL;

httpd_handle_t start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        // httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post_wifi_sta_conn);
    }
    return server;
}

void stop_webserver(httpd_handle_t server) {
    if (server) {
        httpd_stop(server);
    }
}