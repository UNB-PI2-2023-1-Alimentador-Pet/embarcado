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

void aciona_fluxo_de_tarefas(int tempo_min, int peso_gramas){

    //vTaskDelay(2000 / portTICK_PERIOD_MS); 
    printf("1 - Iniciou o processo para disponibilizar a ração...\n");
    float ver_peso = 0;

    int i = 0;
    while( i < 50){
        xTaskCreate(task_balanca, "task_balanca", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        ver_peso = getPesoBandeja();
        if (ver_peso < 0 || ver_peso > 500){
            ver_peso = 0;
        }
        else
            printf("Peso da bandeja: %f\n", ver_peso);

        if(ver_peso < peso_gramas){
            printf("Despejando ração pela %d x\n", i);
            despejar_comida();
            vTaskDelay(1000 / portTICK_PERIOD_MS); 

        }
        else{
            printf("Peso da bandeja atingido...\n");
            break;
        }
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }

    printf("2 - Verificação do nível da ração...\n");
    verifica_nivel();

    printf("3 - Ativação do sensor de presença\n");
    float sensor_presenca = 0;
    int j = 0;
    while(j < 50){
        printf("3.1 - Aguardando detecção de presença menor que 10cm\n");
        verifica_presenca();
        sensor_presenca = getSensorPresenca();
        if (sensor_presenca < 0 || sensor_presenca > 500){
            sensor_presenca = 0;
        }
        else
        printf("Distância do sensor de presença: %f\n", sensor_presenca);

        if(sensor_presenca <= 10){
            printf("4 - Abrindo a bandeja...\n");
            abrir_bandeja();
            //despejar_comida();

            tempo_min = tempo_min * 1000 *60;

            vTaskDelay(tempo_min / portTICK_PERIOD_MS); 

            printf("5 - Fechando a bandeja...\n");
            fechar_bandeja();
            //girar_sentido_contrario();

            break;
        }
        j++;
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    printf("6 - Sistema finalizado...\n");

    float nivel_racao = 0;
    nivel_racao = getNivelRacao();
    printf("Nivel de ração remanescente: %f\n", nivel_racao);

}