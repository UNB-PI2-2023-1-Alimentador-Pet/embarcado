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
#include "wifi.h"
#include "status.h"
#include "httpserver.h"

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

esp_err_t reset_wifi_status(httpd_req_t* req) {

    set_wifi_sta_saved(false);

    const char resp[] = "reset indeed";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t post_wifi_sta_conn_data_handler(httpd_req_t* req) {
    char* buffer = malloc(req->content_len);
    httpd_req_recv(req, buffer, req->content_len);

    ESP_LOGI("HTTP", "body(len %d): \n%s\n", req->content_len, buffer);

    sta_conn_data sta_data = jsonstr_to_sta_conn_data(buffer);
    save_sta_conn_data_into_flash(&sta_data);

    connect_wifi_sta(sta_data.ssid, sta_data.password);

    const char resp[] = "Ok!";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t get_verify_connection_handler(httpd_req_t* req) {
    uint8_t status = get_connection_status();
    
    cJSON* json_resp = cJSON_CreateObject();

    char status_str[10];
    
    switch (status) {
        case CONN_TRYING:
            strcpy(status_str, "trying");
            break;
        case CONN_FAIL:
            strcpy(status_str, "fail");
            break;
        case CONN_CONNECTED:
            strcpy(status_str, "connected");
            break;
        default:
            strcpy(status_str, "idle");
            break;
    }

    cJSON_AddStringToObject(json_resp, "status", status_str);

    char* body_resp = cJSON_Print(json_resp);
    httpd_resp_send(req, body_resp, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(json_resp);
    free(body_resp);
    return ESP_OK;
}

esp_err_t post_restart_handler(httpd_req_t* req) {
    char* resp = "{\"action\": restart}";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    usleep(1000000);
    esp_restart();
    return ESP_OK;
}

static httpd_uri_t uri_get_verify_connection = {
    .uri = "/verify_connection",
    .method = HTTP_GET,
    .handler = get_verify_connection_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_post_restart_esp = {
    .uri = "/restart",
    .method = HTTP_POST,
    .handler = post_restart_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_post_wifi_sta_conn = {
    .uri = "/wifi",
    .method = HTTP_POST,
    .handler = post_wifi_sta_conn_data_handler,
    .user_ctx = NULL
};

static httpd_uri_t  uri_reset_wifi = {
    .uri = "/reset",
    .method = HTTP_POST,
    .handler = reset_wifi_status,
    .user_ctx = NULL
};

static httpd_handle_t server = NULL;

void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        // httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post_wifi_sta_conn);
        httpd_register_uri_handler(server, &uri_reset_wifi);
        httpd_register_uri_handler(server, &uri_get_verify_connection);
        httpd_register_uri_handler(server, &uri_post_restart_esp);
    }
}

void stop_webserver(httpd_handle_t server) {
    if (server) {
        httpd_stop(server);
    }
}