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

            fechar_bandeja();
            vTaskDelay(2000 / portTICK_PERIOD_MS); 
            abrir_bandeja();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            girar_sentido_contrario();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            i++;
    }
}

void teste_balancas(){
    printf("Simulção de balanças\n");
    float ver_peso = 0;
    //task_balanca();
    //vTaskDelay(1000 / portTICK_PERIOD_MS); 

    int i = 0;
    while(i < 50){
            xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ver_peso = getPesoBandeja();
            if (ver_peso < 0 || ver_peso > 500){
                ver_peso = 0;
            }
            else
            printf("Peso da bandeja: %f\n", ver_peso);
            
            if(ver_peso > 10){
                printf("Atingiu o peso necessário\nSó esperar o burro do cachorro comer\n");
                fechar_bandeja();
                break;
            }
            else
                despejar_comida();
                
            i++;
            printf("Chegou aqui\n");
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

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    mqtt_app_start();
    // start_webserver();

    // if (get_wifi_sta_saved() || USE_STA_DEFAULT) {
    //     // connect to mqtt server
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    //     sync_time();
    //     ESP_LOGI("TIME", "time sync");

    //     // while (1) {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     get_time();
    //     // }
    //     // mqtt_app_start();

    //     while(1) {
            
    //         vTaskDelay(500 / portTICK_PERIOD_MS);

    //         if (is_time_or_later("2023-06-28 20:00:00")) {
    //             aciona_fluxo_de_tarefas(1,60);
    //             break;
    //         }
            
    //     }

    // }


printf("--------------- Iniciando simulação ---------------\n");
vTaskDelay(15000 / portTICK_PERIOD_MS);

while(1){

    //testemotores();
    teste_balancas();
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
}

//testemotores();
// aciona_fluxo_de_tarefas(1,60);


}