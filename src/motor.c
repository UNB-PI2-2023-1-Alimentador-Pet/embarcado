
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>

// Definição dos pinos do motor de passo
#define PIN_STEP  GPIO_NUM_18
#define PIN_DIR   GPIO_NUM_19

// Definição das variáveis de controle do motor
int velocidade = 500;  // Velocidade em microssegundos entre cada pulso (mais baixo = mais rápido) 1000= 1 segundo
//int sentido = 1;       // 1 para horário, -1 para anti-horário
int rpm = 1200;          // Rotações por minuto desejadas 600= 1 volta completa

// Função para girar o motor de passo
void motorPasso(int numPassos) {
  for (int i = 0; i < numPassos; i++) {
    gpio_set_level(PIN_STEP, 1);
    esp_rom_delay_us(velocidade);
    gpio_set_level(PIN_STEP, 0);
    esp_rom_delay_us(velocidade);
  }
}

// Tarefa principal
void aciona_motor(void) {
  // Configuração dos pinos de controle do motor como saída
  gpio_reset_pin(PIN_STEP);
  gpio_set_direction(PIN_STEP, GPIO_MODE_OUTPUT);
  gpio_reset_pin(PIN_DIR);
  gpio_set_direction(PIN_DIR, GPIO_MODE_OUTPUT);

  // Cálculo do número de passos por minuto com base nas RPM
  int passosPorMinuto = 200 * rpm;  // Considerando motor Nema 17 com 200 passos por rotação
  
  // Cálculo do número de passos por segundo
  int passosPorSegundo = passosPorMinuto / 60;

  while (1) {
    
    gpio_set_level(PIN_DIR, 1); // Sentido horário
    int passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  // portTICK_PERIOD_MS é a duração de cada tick (geralmente 1 ms em ESP-IDF)
    
    // Girando o motor por um segundo
    printf("gira no sentido anti horairo\n");
    motorPasso(passosPorLoop);
    vTaskDelay(100 / portTICK_PERIOD_MS); 
    gpio_set_level(PIN_DIR, 0); // Sentido anti-horário
    printf("gira no sentido horairo\n");
    passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  // portTICK_PERIOD_MS é a duração de cada tick (geralmente 1 ms em ESP-IDF)
    motorPasso(passosPorLoop);
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Aguarda 1 segundo antes de girar novamente
  }
}
