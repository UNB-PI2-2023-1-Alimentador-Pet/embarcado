#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>

// Definição dos pinos do motor de passo
#define PIN_STEP  GPIO_NUM_18
#define PIN_DIR   GPIO_NUM_19

// Definição das variáveis de controle do motor
extern int velocidade;
extern int rpm;

// Função para girar o motor de passo
void motorPasso(int numPassos);

// Tarefa principal
void aciona_motor(void);
