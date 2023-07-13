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


//------------------Fluxo de tarefas (não está utilizando threads)------------------
void aciona_fluxo_de_tarefas(int tempo_min, int peso_gramas){

    printf("1 - Iniciou o processo para disponibilizar a ração...\n");
    float ver_peso = 0;

    int i = 0;
    while( i < 50){
        
        ver_peso = balanca();//Caso não funcione, utilize as tasks abaixo

        //xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
        //ver_peso = getPesoBandeja();
        
        if (ver_peso < 0 || ver_peso > 500){
            ver_peso = 0;
        }

        printf("Peso da bandeja: %f\n", ver_peso);

        if(ver_peso < peso_gramas){
            printf("Despejando ração pela %d x\n", i);
            despejar_comida();
            vTaskDelay(1000 / portTICK_PERIOD_MS);//Tempo para a ração cair na bandeja, pode alterar quando fizerem os testes 

        }
        else{
            printf("Peso da bandeja atingido...\n");
            break;
        }
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }

    printf("2 - Verificação do nível da ração...\n");
    float nivel_racao = 0;
    nivel_racao = sensor_nivel();

    printf("3 - Ativação do sensor de presença\n");
    float sensor_presenca = 0;
    int j = 0;
    while(j < 50){
        printf("3.1 - Aguardando detecção de presença menor que 10cm\n");
        sensor_presenca = Psensor_nivel();
        if (sensor_presenca < 0 || sensor_presenca > 500){
            sensor_presenca = 0;
        }

        printf("Distância do sensor de presença: %f\n", sensor_presenca);

        if(sensor_presenca <= 10){ //Esse 10 é um valor de exemplo, pode ser alterado
            printf("4 - Abrindo a bandeja...\n");

            abrir_bandeja();
            //despejar_comida();

            tempo_min = tempo_min * 1000 *60;

            vTaskDelay(tempo_min / portTICK_PERIOD_MS); 

            printf("5 - Fechando a bandeja...\n");
            fechar_bandeja();
            //girar_sentido_contrario();
            set_bandeja_aberta(false);
            break;
        }
        j++;
        vTaskDelay(500 / portTICK_PERIOD_MS); 
    }

    printf("6 - Sistema finalizado...\n");
    printf("Nivel de ração remanescente: %f\n", nivel_racao);

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