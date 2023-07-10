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
#include "variaveis_globais.h"
#include "funcoes.h"

#include "balanca.h"
#include "sensor_nivel.h"
#include "motor.h"
#include "racao.h"
#include "presenca.h"

void testemotores(){
    printf("Simulção de motores\n");
    
    int i = 0;
    while( i < 50){
            despejar_comida();
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
            abrir_bandeja();
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
            fechar_bandeja();
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
            girar_sentido_contrario();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            i++;
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

// testemotores();
//aciona_fluxo_de_tarefas(1,60);


}