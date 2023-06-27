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
#include "esp_sntp.h"
#include "lwip/apps/sntp.h"
#include "sys/time.h"


#include "wifi.h"
#include "httpserver.h"
#include "status.h"
#include "mqtt.h"

struct tm get_time() {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%F %T", &timeinfo);
    ESP_LOGI("TIME", "current time=%s", strftime_buf);
    return timeinfo;
}

void sync_time() {
    while(!CONN_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    ESP_LOGI("Time", "Waiting time sync");
    sntp_init();
    // sntp_setservername(1, "pool.ntp.org");

    sntp_setservername(0, "pool.ntp.org");
    const char* servername = sntp_getservername(0);

    ESP_LOGI("Time", "%s", servername);

    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    struct timeval time_;
    sntp_sync_time(&time_);

    setenv("TZ", "GMT+3", 1);
    tzset();
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sntp_sync_status_t status = sntp_get_sync_status();

        // ESP_LOGI("SYNC status", "%d", (int)status);
        if (status == SNTP_SYNC_STATUS_COMPLETED) {
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            break;
        }
    }
}

void app_main() {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();
    start_webserver();

    if (get_wifi_sta_saved() || USE_STA_DEFAULT) {
        // connect to mqtt server
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        sync_time();
        ESP_LOGI("TIME", "time sync");

        get_time();
        mqtt_app_start();
    }

}
