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
#include "presenca.h"

float peso_bandeja = 0;
float nivel_racao = 0;
float sensor_presenca = 100;

void task_balanca(void *pvParameters) {
    peso_bandeja = balanca();

    vTaskDelete(NULL);
}

void task_sensor_nivel(void *pvParameters) {
    nivel_racao = sensor_nivel();

    vTaskDelete(NULL);
}

void task_sensor_presenca(void *pvParameters) {
    sensor_presenca = Psensor_nivel();

    vTaskDelete(NULL);
}

void verifica_nivel(){
    xTaskCreate(task_sensor_nivel, "task_sensor_nivel", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);

    //printf("------------------\n");
    //printf("Nivel de ração: %d\n", nivel_racao);
    
}

void verifica_presenca(){
    xTaskCreate(task_sensor_presenca, "task_sensor_presenca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);

    // printf("------------------\n");
    // printf("Nivel de ração: %f\n", sensor_presenca);
    
}

void aciona_fluxo_de_tarefas(int tempo_min, int peso_gramas){

    vTaskDelay(2000 / portTICK_PERIOD_MS); 

    printf("1 - Iniciou o processo para disponibilizar a ração...\n");
    xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);

    int i = 0;
    while( i < 50){
        if(peso_gramas < peso_bandeja){
            printf("Despejando ração pela %d x\n", i);
            despejar_comida();
            vTaskDelay(1000 / portTICK_PERIOD_MS); 

        }
        else{
            printf("Peso da bandeja atingido...\n");
            break;
        }
        i++;
        xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }

    printf("2 - Verificação do nível da ração...\n");
    verifica_nivel();

    printf("3 - Ativação do sensor de presença\n");
    verifica_presenca();
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    while(1){
        printf("3.1 - Aguardando detecção de presença menor que 10cm\n");
        verifica_presenca();

        if(sensor_presenca <= 10){
            abrir_bandeja();
            //despejar_comida();
            vTaskDelay(tempo_min / portTICK_PERIOD_MS); 

            fechar_bandeja();
            //girar_sentido_contrario();

            break;
        }
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
                aciona_fluxo_de_tarefas(1,60);
                break;
            }
            
        }

    }


// printf("--------------- Iniciando simulação ---------------\n");
// vTaskDelay(3000 / portTICK_PERIOD_MS);

// while(1){
//    aciona_fluxo_de_tarefas(1,60);
// }
}