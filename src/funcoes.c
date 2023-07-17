#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "variaveis_globais.h"
#include "funcoes.h"

#include "balanca.h"
#include "sensor_nivel.h"
#include "motor.h"
#include "racao.h"
#include "presenca.h"
#include "status.h"
#include "mqtt.h"
#include "string.h"

static char* topic = "feeder/mfteste";

static SemaphoreHandle_t xSemaphore = NULL;

void init_funcoes() {
    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemaphore);
}

//------------------Threads------------------

void task_balanca(void *pvParameters) {
    float peso_novo;
    peso_novo = balanca();
    setPesoBandeja(peso_novo);
    vTaskDelay(1000 / portTICK_PERIOD_MS);


    vTaskDelete(NULL);
}

void task_sensor_nivel(void *pvParameters) {
    float nivel_novo;
    nivel_novo = sensor_nivel();
    setNivelRacao(nivel_novo);

    vTaskDelete(NULL);
}

void task_sensor_presenca(void *pvParameters) {
    float presenca_novo;
    presenca_novo = Psensor_nivel();
    setSensorPresenca(presenca_novo);

    vTaskDelete(NULL);
}

void verifica_nivel(){
    xTaskCreate(task_sensor_nivel, "task_sensor_nivel", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}

void verifica_presenca(){
    xTaskCreate(task_sensor_presenca, "task_sensor_presenca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}

void loop_balanca() {
    float ver_peso = 0;
    char message_buffer[500];
    memset(message_buffer, 0, sizeof(message_buffer));

    while (true) {
        despejar_comida();
        vTaskDelay( 500 / portTICK_PERIOD_MS);
    }

    while (true) {

    //     for (int i = 0; i < 10; i++) {
    //         float nivel_racao = 0;
    //         nivel_racao = sensor_nivel();        

    //         sprintf(message_buffer, "Nivel racao: %lf", nivel_racao);
    //         mqtt_app_publish(topic, message_buffer, 1);

    //         vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     }

    //     for (int i = 0; i < 10; i++) {
    //         float sensor_presenca = 0;
    //         sensor_presenca = Psensor_nivel();

    //         sprintf(message_buffer, "Sensor_presenca: %lf", sensor_presenca);
    //         mqtt_app_publish(topic, message_buffer, 1);

    //         vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     }
        ver_peso = balanca();

        sprintf(message_buffer, "Peso balanca: %lf", ver_peso);
        mqtt_app_publish(topic, message_buffer, 1);

        // sprintf(message_buffer, "Despejando racao");
        // mqtt_app_publish(topic, message_buffer, 1);

        despejar_comida();
        if ((int)ver_peso > 200) {

            sprintf(message_buffer, "Abrindo bandeja");
            mqtt_app_publish(topic, message_buffer, 1);

            // abrir_bandeja();

            vTaskDelay(1000 / portTICK_PERIOD_MS);

            // fechar_bandeja();
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

//------------------Fluxo de tarefas (não está utilizando threads)------------------
void aciona_fluxo_de_tarefas(int tempo_min, int peso_gramas){

    // loop_balanca();
    char message_buffer[500];
    memset(message_buffer, 0, sizeof(message_buffer));

    if (xSemaphoreTake(xSemaphore, 1) == pdFALSE) {
        // ESP_LOGI("FLUXO", "SEMAPHORE NOT TAKEN");
        sprintf(message_buffer, "SEMAPHORE NOT TAKEN");
        mqtt_app_publish(topic, message_buffer, 1);
        return;
    }

    float nivel_racao = 0;

    nivel_racao = sensor_nivel();

    sprintf(message_buffer, "Nivel racao (antes de despejar): %lf", nivel_racao);
    mqtt_app_publish(topic, message_buffer, 1);


    // printf("1 - Iniciou o processo para disponibilizar a ração...\n");

    sprintf(message_buffer, "starting...");
    mqtt_app_publish(topic, message_buffer, 1);

    float ver_peso = 0;

    int i = 0;
    while( i < 50){
        
        ver_peso = balanca();//Caso não funcione, utilize as tasks abaixo
        sprintf(message_buffer, "Peso balanca (antes da verif.): %lf", ver_peso);
        mqtt_app_publish(topic, message_buffer, 1);

        //xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
        ver_peso = getPesoBandeja();
        
        if (ver_peso < 0 || ver_peso > 500){
            ver_peso = 0;
        }
        sprintf(message_buffer, "Peso balanca (depois da verif.): %lf", ver_peso);
        mqtt_app_publish(topic, message_buffer, 1);

        // printf("Peso da bandeja: %f\n", ver_peso);

        if(ver_peso < peso_gramas){
            // printf("Despejando ração pela %d x\n", i);
            sprintf(message_buffer, "Despejando racao pela %d x", i);
            mqtt_app_publish(topic, message_buffer, 1);

            despejar_comida();
            vTaskDelay(500 / portTICK_PERIOD_MS);//Tempo para a ração cair na bandeja, pode alterar quando fizerem os testes 

        }
        else{
            sprintf(message_buffer, "Peso da bandeja atingido...");
            mqtt_app_publish(topic, message_buffer, 1);
            
            // printf("Peso da bandeja atingido...\n");
            break;
        }
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }

    // printf("2 - Verificação do nível da ração...\n");
    sprintf(message_buffer, "2 - Verificando nivel da racao...");
    mqtt_app_publish(topic, message_buffer, 1);


    // nivel_racao = sensor_nivel();

    sprintf(message_buffer, "Nivel racao (depois de despejar): %lf", nivel_racao);
    mqtt_app_publish(topic, message_buffer, 1);

    // printf("3 - Ativação do sensor de presença\n");
    sprintf(message_buffer, "3 - Abrindo Bandeja");
    mqtt_app_publish(topic, message_buffer, 1);

    // sprintf(message_buffer, "4 - Abrindo a bandeja...");
    // mqtt_app_publish(topic, message_buffer, 1);

    sprintf(message_buffer, "Peso antes de abrir a bandeja: %lf", ver_peso);
    mqtt_app_publish(topic, message_buffer, 1);

    abrir_bandeja();

    sprintf(message_buffer, "Waiting %d minutes", tempo_min);
    mqtt_app_publish(topic, message_buffer, 1);

    tempo_min = tempo_min * 1000 *60;
    vTaskDelay( tempo_min / portTICK_PERIOD_MS);

    // float sensor_presenca = 0;
    // int j = 0;
    // while(j < 50){
    //     // printf("3.1 - Aguardando detecção de presença menor que 10cm\n");

    //     sprintf(message_buffer, "4 - Aguardando detecção de presença menor que 10cm");
    //     mqtt_app_publish(topic, message_buffer, 1);

    //     // sensor_presenca = Psensor_nivel();
    //     sensor_presenca = 0;

    //     sprintf(message_buffer, "Distância do sensor de presença(antes): %lf", sensor_presenca);
    //     mqtt_app_publish(topic, message_buffer, 1);
        
    //     if (sensor_presenca < 0 || sensor_presenca > 500){
    //         sensor_presenca = 0;
    //     }

    //     sprintf(message_buffer, "Distância do sensor de presença(depois): %lf", sensor_presenca);
    //     mqtt_app_publish(topic, message_buffer, 1);

    //     if (sensor_presenca <= 10 || true) {

    //         sprintf(message_buffer, "Waiting %d minutes", tempo_min);
    //         mqtt_app_publish(topic, message_buffer, 1);

    //         tempo_min = tempo_min * 1000 *60;
    //         vTaskDelay(tempo_min / portTICK_PERIOD_MS); 
        
    //         break;
    //     }

        // printf("Distância do sensor de presença: %f\n", sensor_presenca);

        // if(sensor_presenca <= 10){ //Esse 10 é um valor de exemplo, pode ser alterado
            // printf("4 - Abrindo a bandeja...\n");
            // sprintf(message_buffer, "4 - Abrindo a bandeja...");
            // mqtt_app_publish(topic, message_buffer, 1);

            // abrir_bandeja();
            // //despejar_comida();

            // tempo_min = tempo_min * 1000 *60;

            // vTaskDelay(tempo_min / portTICK_PERIOD_MS); 


            // sprintf(message_buffer, "5 - Fechando a bandeja...");
            // mqtt_app_publish(topic, message_buffer, 1);
            // // printf("5 - Fechando a bandeja...\n");
            // fechar_bandeja();
            // //girar_sentido_contrario();
            // set_bandeja_aberta(false);
            // break;
        // }
        // j++;
    // vTaskDelay(500 / portTICK_PERIOD_MS); 
    // }

    sprintf(message_buffer, "7 - Fechando a bandeja...");
    mqtt_app_publish(topic, message_buffer, 1);
    // printf("5 - Fechando a bandeja...\n");
    fechar_bandeja();

    sprintf(message_buffer, "Peso depois de fechar a bandeja: %lf", ver_peso);
    mqtt_app_publish(topic, message_buffer, 1);

    //girar_sentido_contrario();
    set_bandeja_aberta(false);

    sprintf(message_buffer, "6 - Sistema finalizado...\n Nivel de ração remanescente: %f", nivel_racao);
    mqtt_app_publish(topic, message_buffer, 1);

    // printf("6 - Sistema finalizado...\n");
    // printf("Nivel de ração remanescente: %f\n", nivel_racao);

    xSemaphoreGive(xSemaphore);
//     vTaskDelete(NULL);
}

// void aciona_fluxo_de_tarefas(int _tempo_min, int _peso_gramas){

//     if (xSemaphoreTake(xSemaphore, 1) == pdFALSE) {
//         ESP_LOGI("FLUXO", "SEMAPHORE NOT TAKEN");
//         return;
//     }
//     tempo_min = _tempo_min;
//     peso_gramas = _peso_gramas;

//     xTaskCreate(task_fluxo_de_tarefas, "task_fluxo_de_tarefas", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
// }