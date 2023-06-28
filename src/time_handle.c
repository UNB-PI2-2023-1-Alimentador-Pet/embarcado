#include "esp_err.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "lwip/apps/sntp.h"
#include "sys/time.h"
#include "time.h"
#include "status.h"

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

bool is_time_or_later(const char* _time) {
    struct tm target_time;
    strptime(_time, "%F %T", &target_time);
    
    time_t target_time_ = mktime(&target_time);

    struct tm current = get_time();
    time_t current_time = mktime(&current);

    return current_time >= target_time_;
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