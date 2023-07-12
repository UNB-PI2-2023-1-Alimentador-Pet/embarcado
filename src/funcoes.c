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

static SemaphoreHandle_t xSemaphore = NULL;

void init_funcoes() {
    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemaphore);
}

void task_balanca(void *pvParameters) {
    float peso_novo;
    peso_novo = balanca();
    setPesoBandeja(peso_novo);

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

    //printf("------------------\n");
    //printf("Nivel de ração: %d\n", nivel_racao);
}

void verifica_presenca(){
    xTaskCreate(task_sensor_presenca, "task_sensor_presenca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);

    // printf("------------------\n");
    // printf("Nivel de ração: %f\n", sensor_presenca);
}

static int tempo_min;
static int peso_gramas;

void task_fluxo_de_tarefas(void *pvParameters) {
    vTaskDelay(2000 / portTICK_PERIOD_MS); 

    printf("1 - Iniciou o processo para disponibilizar a ração...\n");
    //xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
    printf("pesei o valor da balança\n");

    int i = 0;
    while( i < 50){
        if(i>=5)
            setPesoBandeja(60);

        printf("%f\n", peso_bandeja);

        if(peso_gramas > peso_bandeja){
            printf("Despejando ração pela %d x\n", i);
            //despejar_comida();
            vTaskDelay(1000 / portTICK_PERIOD_MS); 

        }
        else{
            printf("Peso da bandeja atingido...\n");
            break;
        }
        i++;
        //xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }

    printf("2 - Verificação do nível da ração...\n");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    //verifica_nivel();

    printf("3 - Ativação do sensor de presença\n");
    //verifica_presenca();
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    int j = 0;
    while(j < 50){
        printf("3.1 - Aguardando detecção de presença menor que 10cm\n");
        //verifica_presenca();
        if(j>=5)
            setSensorPresenca(8);

        printf("%f\n", sensor_presenca);

        if(sensor_presenca <= 10){
            printf("4 - Abrindo a bandeja...\n");
            set_bandeja_aberta(true);
            //abrir_bandeja();
            //despejar_comida();

            tempo_min = tempo_min * 1000 *60;

            vTaskDelay(tempo_min / portTICK_PERIOD_MS); 

            printf("5 - Fechando a bandeja...\n");
            //fechar_bandeja();
            //girar_sentido_contrario();
            set_bandeja_aberta(false);
            break;
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS); 
        j++;
    }
    xSemaphoreGive(xSemaphore);
    vTaskDelete(NULL);
}

void aciona_fluxo_de_tarefas(int _tempo_min, int _peso_gramas){

    if (xSemaphoreTake(xSemaphore, 1) == pdFALSE) {
        ESP_LOGI("FLUXO", "SEMAPHORE NOT TAKEN");
        return;
    }
    tempo_min = _tempo_min;
    peso_gramas = _peso_gramas;

    xTaskCreate(task_fluxo_de_tarefas, "task_fluxo_de_tarefas", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}