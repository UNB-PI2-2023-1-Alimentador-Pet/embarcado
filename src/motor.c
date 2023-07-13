#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>

// Definição dos pinos do motor de passo
#define PIN_STEP  GPIO_NUM_19
#define PIN_DIR   GPIO_NUM_18

int velocidade = 500;  
int rpm = 18000;
// int velocidade = 2000;  
// int rpm = 98; 

int motorLigado = 1;

// Função para girar o motor de passo
void motorPasso(int numPassos) {
  for (int i = 0; i < numPassos; i++) {
    if (!motorLigado) {
      return; 
    }
    gpio_set_level(PIN_STEP, 1);
    esp_rom_delay_us(velocidade);
    gpio_set_level(PIN_STEP, 0);
    esp_rom_delay_us(velocidade);
  }
}

void gira_motor(int sentido) {
  gpio_reset_pin(PIN_STEP);
  gpio_set_direction(PIN_STEP, GPIO_MODE_OUTPUT);
  gpio_reset_pin(PIN_DIR);
  gpio_set_direction(PIN_DIR, GPIO_MODE_OUTPUT);

  // Cálculo do número de passos por minuto com base nas RPM
  int passosPorMinuto = 200 * rpm; 
  // Cálculo do número de passos por segundo
  int passosPorSegundo = passosPorMinuto / 60;

    int passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  
    
    if (sentido == 1) {
      gpio_set_level(PIN_DIR, 1); // Sentido horário
      printf("gira no sentido anti horario\n");
      motorPasso(passosPorLoop);
    } else {
      gpio_set_level(PIN_DIR, 0); // Sentido anti-horário
      printf("gira no sentido horario\n");
      passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  
      motorPasso(passosPorLoop);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  gpio_set_level(PIN_STEP, 0);
}

void task_pra_girar_motor() {
  gpio_reset_pin(PIN_STEP);
  gpio_set_direction(PIN_STEP, GPIO_MODE_OUTPUT);
  gpio_reset_pin(PIN_DIR);
  gpio_set_direction(PIN_DIR, GPIO_MODE_OUTPUT);

  // Cálculo do número de passos por minuto com base nas RPM
  int passosPorMinuto = 200 * rpm; 
  // Cálculo do número de passos por segundo
  int passosPorSegundo = passosPorMinuto / 60;

    int passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  
    
    gpio_set_level(PIN_DIR, 1); // Sentido horário
    printf("gira no sentido anti horario\n");
    motorPasso(passosPorLoop);

    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  gpio_set_level(PIN_STEP, 0);
}


// Função para ligar o motor
void abrir_bandeja() {
  motorLigado = 1;
  //xTaskCreate(task_pra_girar_motor, "task_pra_girar_motor", 2048, NULL, 5, NULL);
  gira_motor(0);
}

void fechar_bandeja() {
  motorLigado = 1;
  //xTaskCreate(gira_motor, "gira_motor", 2048, NULL, 5, NULL);
  gira_motor(1);
}

// Função para desligar o motor
void desligar_motor() {
  motorLigado = 0;
}