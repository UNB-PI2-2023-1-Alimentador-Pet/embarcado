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
#include "wifi.h"
#include "status.h"
#include "time_handle.h"
#include "mqtt.h"
#include "espcam_comm.h"
#include "mac.h"
#include "cJSON.h"

#define CODE_WIFI 1
#define CODE_PHOTO 2

static const char* TAG = "wifi";

static wifi_config_t wifi_config;
static wifi_config_t wifi_sta_config;

void esp_data_message(char* message) {
    nvs_handle_t handle;
    char ssid[32];
    char pass[64];
    char user_hash[40];
    char* token;
    size_t ssid_size = 32;
    size_t pass_size = 64;
    size_t user_hash_size = 40;
    
    nvs_open("nvs", NVS_READONLY, &handle);

    nvs_get_str(handle, "wifi_sta_ssid", ssid, &ssid_size);
    nvs_get_str(handle, "wifi_sta_pass", pass, &pass_size);
    nvs_get_str(handle, "user_hash", user_hash, &user_hash_size);
    token = get_mac_address();

    nvs_close(handle);

    cJSON* obj = cJSON_CreateObject();

    cJSON_AddNumberToObject(obj, "code", CODE_WIFI);
    cJSON_AddStringToObject(obj, "ssid", ssid);
    cJSON_AddStringToObject(obj, "password", pass);
    cJSON_AddStringToObject(obj, "user_hash", user_hash);
    cJSON_AddStringToObject(obj, "feeder_token", token);

    strcpy(message, cJSON_Print(obj));

    cJSON_Delete(obj);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*)event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*)event_data;
        ESP_LOGI(TAG, "CONNECTED");

        wifi_ap_record_t ap_info;
        ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_info));

        ESP_LOGI("http_serverWIFI", "%s", ap_info.bssid);

        if (!get_wifi_sta_saved()) {
            set_wifi_sta_saved(true);
            set_connection_status(CONN_CONNECTED);
            return;
        }
        set_connection_status(CONN_CONNECTED);

        sync_time();
        mqtt_app_start();

        char message[200];
        esp_data_message(message);
        espcam_enqueue_message(message);
    
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*)event_data;
        ESP_LOGI(TAG, "DISCONNECTED\n");
        if (get_connection_status() == CONN_TRYING) {
            set_connection_status(CONN_FAIL);
            return;
        }

        mqtt_app_close();

        ESP_ERROR_CHECK(esp_wifi_connect());

    }
}

void get_wifi_sta_data_from_flash() {
    nvs_handle_t handle;
    nvs_open("nvs", NVS_READONLY, &handle);

    char ssid[32];
    char password[64];

    size_t ssid_size = 32;
    size_t password_size = 64;

    if (USE_STA_DEFAULT) {
        strcpy(ssid, "Redmi Note 10 5G");
        strcpy(password, "12345678");
    }
    else {
        nvs_get_str(handle, "wifi_sta_ssid", ssid, &ssid_size);
        nvs_get_str(handle, "wifi_sta_pass", password, &password_size);
    }

    nvs_close(handle);

    memcpy(wifi_sta_config.sta.ssid, ssid, ssid_size);
    memcpy(wifi_sta_config.sta.password, password, password_size);
}

void wifi_init_softap_and_sta() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    // wifi_config.ap = {
    //         .ssid = "FROST ACCESS POINT",
    //         .ssid_len = strlen("FROST ACCESS POINT"),
    //         .channel = 1,
    //         .password = "12345678",
    //         .max_connection = 2,
    //         .authmode = WIFI_AUTH_WPA2_PSK
    // };

    memcpy(&wifi_config.ap.ssid, "MiAuFeeder-0001", strlen("MiAuFeeder-0001"));
    wifi_config.ap.ssid_len = strlen("MiAuFeeder-0001");
    // wifi_config.ap.channel = 1;
    memcpy(&wifi_config.ap.password, "12345678", strlen("12345678"));
    // wifi_config.ap.channel = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.ap.max_connection = 2;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    // memcpy(wifi_sta_config.sta.ssid, "frostConn2.5G", strlen("frostConn2.5G"));
    // memcpy(wifi_sta_config.sta.password, "0000ff99GG", strlen("0000ff99GG"));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    // ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // ESP_ERROR_CHECK(esp_wifi_connect());
}

void wifi_init_sta() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    get_wifi_sta_data_from_flash();

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());    
}

void connect_wifi_sta(const char* ssid, const char* password) {
    set_connection_status(CONN_TRYING);

    memcpy(wifi_sta_config.sta.ssid, ssid, strlen(ssid));
    memcpy(wifi_sta_config.sta.password, password, strlen(password));
    // wifi_config.sta.bssid_set = 0;
    // wifi_config.sta.ssid = ssid;
    // wifi_config.sta.password = password;
    // ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    // ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void wifi_init() {
    set_connection_status(CONN_IDLE);

    if (get_wifi_sta_saved() || USE_STA_DEFAULT) {
        // init wifi sta mode only
        wifi_init_sta();
    }
    else {
        wifi_init_softap_and_sta();
    }
};
