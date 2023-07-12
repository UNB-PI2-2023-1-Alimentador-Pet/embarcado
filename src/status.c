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
#include "variaveis_globais.h"
#include "cJSON.h"
#include "mqtthandler.h"
#include "mqtt.h"

static char* wifi_sta_saved_status_key = "wifi_sta_avail";

static uint8_t connection_status = CONN_IDLE;
static bool bandeja_aberta = false;

void set_connection_status(uint8_t status) {
    connection_status = status;
}

uint8_t get_connection_status() {
    return connection_status;
}

void set_wifi_sta_saved(bool saved) {
    nvs_handle_t handle;
    nvs_open("nvs", NVS_READWRITE, &handle);

    nvs_set_u8(handle, wifi_sta_saved_status_key, saved);
    nvs_commit(handle);

    nvs_close(handle);
}

bool get_wifi_sta_saved() {
    nvs_handle_t handle;
    nvs_open("nvs", NVS_READONLY, &handle);

    uint8_t status = 0;
    nvs_get_u8(handle, wifi_sta_saved_status_key, &status);

    nvs_close(handle);
    return status;
}

void task_send_status(void *pvParameters) {
    while (1) {
        if (is_mqtt_connected()) {
            send_status();
        }
        vTaskDelay( 3000 / portTICK_PERIOD_MS );
    }
}

void init_task_send_status() {
    xTaskCreate(task_send_status, "task_send_status", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
}

void set_bandeja_aberta(bool state) {
    bandeja_aberta = state;
}

bool get_bandeja_aberta() {
    return bandeja_aberta;
}

// void get_user_hash(char* buffer) {
//     nvs_handle_t handle;
//     nvs_open("nvs", NVS_READONLY, &handle);

//     size_t len = 30;
//     nvs_get_str(handle, "user_hash", buffer, &len);

//     nvs_close(handle);
// }