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
#include "httpserver.h"
#include "status.h"
#include "mqtt.h"
#include "time_handle.h"

#include "balanca.h"
#include "sensor_nivel.h"
#include "motor.h"

// int velocidade = 500; 
// int rpm = 1200; 

void task_balanca(void *pvParameters) {
    // Coloque o código da função balanca() aqui
    balanca();

    vTaskDelete(NULL); // Exclui a tarefa quando a função balanca() terminar
}

void task_sensor_nivel(void *pvParameters) {
    // Coloque o código da função sensor_nivel() aqui
    sensor_nivel();

    vTaskDelete(NULL); // Exclui a tarefa quando a função sensor_nivel() terminar
}

// void task_motor(void *pvParameters) {
//     // Coloque o código da função sensor_nivel() aqui
//     ligar_motor();

//     vTaskDelete(NULL); // Exclui a tarefa quando a função sensor_nivel() terminar
// }

void app_main() {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();
    start_webserver();

    // sensor_nivel();
    while (1) {
        ESP_LOGI("WARNING", "Starting in 10 seconds!");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        abrir_bandeja();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        fechar_bandeja();
    }

    // if (get_wifi_sta_saved() || USE_STA_DEFAULT) {
    //     // connect to mqtt server
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    //     // sync_time();
    //     // ESP_LOGI("TIME", "time sync");

    //     // while (1) {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     // get_time();
    //     // }
    //     // mqtt_app_start();

    //     while(1) {
            
    //         vTaskDelay(500 / portTICK_PERIOD_MS);

    //         if (is_time_or_later("2023-06-28 20:00:00")) {
    //             abrir_bandeja();
    //             vTaskDelay(1000 / portTICK_PERIOD_MS); 
    //             fechar_bandeja();
    //             break;
    //         }
            
    //     }

    // }
    // xTaskCreate(task_motor, "task_motor", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);



    // wifi_init_softap();
    // wifi_init_softap_and_sta();
    //wifi_init();
    // balanca();

    //xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
    
    //sensor_nivel();
    //xTaskCreate(task_sensor_nivel, "task_sensor_nivel", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}
