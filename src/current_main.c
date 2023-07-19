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
#include "cJSON.h"

#include "wifi.h"
#include "httpserver.h"
#include "status.h"
#include "mqtt.h"
#include "time_handle.h"
#include "variaveis_globais.h"
#include "funcoes.h"
#include "api.h"
#include "balanca.h"
#include "sensor_nivel.h"
#include "motor.h"
#include "racao.h"
#include "presenca.h"
#include "scheduler.h"
#include "espcam_comm.h"
#include "mac.h"

void teste_balancas(){
    printf("Simulção de balanças\n");
    float ver_peso = 0;
    //vTaskDelay(1000 / portTICK_PERIOD_MS); 

    int i = 0;
    while(i < 50){
            ver_peso = balanca();
            if (ver_peso < 0 || ver_peso > 500){
                ver_peso = 0;
            }

            char peso_bandeja[300];
            printf("Peso da bandeja: %f\n", ver_peso);
            sprintf(peso_bandeja, "Peso da bandeja: %lf", ver_peso);
            mqtt_app_publish("feeder/testebalanca", peso_bandeja, 1);
            
            if(ver_peso > 100){
                mqtt_app_publish("feeder/testebalanca", "Atingiu o peso necessário", 1);
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

    mac_address_init();

    start_uart_espcam();
    init_funcoes();

    wifi_init();
    start_webserver();

    ESP_LOGI("ESP_TOKEN", "%s", get_mac_address());

    sensor_nivel();
    init_task_send_status();

    // vTaskDelay( 2000 / portTICK_PERIOD_MS );
    vTaskDelay( 15000 / portTICK_PERIOD_MS );

    // aciona_fluxo_de_tarefas(1, 100);

    // teste_balancas();

    // while(1) {
    //     vTaskDelay( 2000 / portTICK_PERIOD_MS);
    //     float peso = balanca();
    //     char peso_bandeja[300];
    //     sprintf(peso_bandeja, "Peso da bandeja: %lf", peso);
    //     mqtt_app_publish("feeder/testebalanca", peso_bandeja, 1);
    // }

    // int id = api_create_history("08:00:00", 120);
    // ESP_LOGI("CREATE HISTORY", "id: %d", id);

    // api_update_history(0, 40);

    if (get_wifi_sta_saved() || USE_STA_DEFAULT) {
        
        // start schedule check event loop
        bool feed_time = false;
        char horario[8];
        int quantidade_total = 0;
        int tempo_bandeja = 0;
        while(1) {
            feed_time = scheduler_feed_time(horario, &quantidade_total, &tempo_bandeja);
            if (feed_time) {
                // api_create_history(horario, quantidade_total);
                aciona_fluxo_de_tarefas(tempo_bandeja, quantidade_total);
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}