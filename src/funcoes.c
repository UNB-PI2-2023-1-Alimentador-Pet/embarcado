#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "variaveis_globais.h"
#include "funcoes.h"

#include "balanca.h"
#include "sensor_nivel.h"
#include "motor.h"
#include "racao.h"
#include "presenca.h"

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