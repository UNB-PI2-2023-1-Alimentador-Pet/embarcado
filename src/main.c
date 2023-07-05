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
#include "racao.h"

float peso_bandeja = 0;
float nivel_racao = 0;

void task_balanca(void *pvParameters) {
    peso_bandeja = balanca();

    vTaskDelete(NULL);
}

void task_sensor_nivel(void *pvParameters) {
    nivel_racao = sensor_nivel();

    vTaskDelete(NULL);
}

void verifica_nivel(){
    xTaskCreate(task_sensor_nivel, "task_sensor_nivel", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);

    //printf("------------------\n");
    //printf("Nivel de ração: %d\n", nivel_racao);
    
}

void aciona_fluxo_de_tarefas(){
    xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
    //printf("Primeiro peso da bandeja: %f\n", peso_bandeja);

    while(1){
        if (peso_bandeja >= 100){
            printf("Despejando ração\n\n\n");
            despejar_comida();
            vTaskDelay(2000 / portTICK_PERIOD_MS);

            printf("Acionando sensor de nível\n\n\n");
            verifica_nivel();
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            printf("Abrindo bandeja\n\n\n");
            abrir_bandeja();
            vTaskDelay(2000 / portTICK_PERIOD_MS); 

            printf("Fechando bandeja\n\n\n");
            fechar_bandeja();
            vTaskDelay(20000 / portTICK_PERIOD_MS);
        }
        else{
            xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
        }
        printf("Peso da bandeja: %f\n", peso_bandeja);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
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

        // while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        get_time();
        // }
        // mqtt_app_start();

        while(1) {
            
            vTaskDelay(500 / portTICK_PERIOD_MS);

            if (is_time_or_later("2023-06-28 20:00:00")) {
                // abrir_bandeja();
                // vTaskDelay(1000 / portTICK_PERIOD_MS); 
                // fechar_bandeja();
                aciona_fluxo_de_tarefas();
                break;
            }
            
        }

    }


// printf("--------------- Iniciando simulação ---------------\n");
// vTaskDelay(3000 / portTICK_PERIOD_MS);
// aciona_fluxo_de_tarefas();



    // xTaskCreate(task_motor, "task_motor", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);



    // wifi_init_softap();
    // wifi_init_softap_and_sta();
    //wifi_init();
    // balanca();

    //xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
    
    //sensor_nivel();
    //xTaskCreate(task_sensor_nivel, "task_sensor_nivel", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}