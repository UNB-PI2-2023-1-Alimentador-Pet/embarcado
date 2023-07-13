#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>

// Definição dos pinos do motor de passo
#define PIN_STEP  GPIO_NUM_32
#define PIN_DIR   GPIO_NUM_33

// Definição das variáveis de controle do motor
// int Rvelocidade = 500;  
// int Rrpm = 18000; 
int Rvelocidade = 2000;  
int Rrpm = 98; 

// Variável de controle do motor
int RmotorLigado = 1;

// Função para girar o motor de passo
void RmotorPasso(int numPassos) {
  for (int i = 0; i < numPassos; i++) {
    if (!RmotorLigado) {
      return;  
    }
    gpio_set_level(PIN_STEP, 1);
    esp_rom_delay_us(Rvelocidade);
    gpio_set_level(PIN_STEP, 0);
    esp_rom_delay_us(Rvelocidade);
  }
}

// Tarefa principal
void Rgira_motor(int sentido) {
  // Configuração dos pinos de controle do motor como saída
  gpio_reset_pin(PIN_STEP);
  gpio_set_direction(PIN_STEP, GPIO_MODE_OUTPUT);
  gpio_reset_pin(PIN_DIR);
  gpio_set_direction(PIN_DIR, GPIO_MODE_OUTPUT);

  // Cálculo do número de passos por minuto com base nas RRPM
  int passosPorMinuto = 200 * Rrpm;  // Considerando motor Nema 17 com 200 passos por rotação

  // Cálculo do número de passos por segundo
  int passosPorSegundo = passosPorMinuto / 60;
  int passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  // portTICK_PERIOD_MS é a duração de cada tick (geralmente 1 ms em ESP-IDF)
    
    // Girando o motor por um segundo
    if (sentido == 1) {
      gpio_set_level(PIN_DIR, 1); // Sentido horário
      printf("gira no sentido anti horario\n");
      RmotorPasso(passosPorLoop);
    } else {
      gpio_set_level(PIN_DIR, 0); // Sentido anti-horário
      printf("gira no sentido horario\n");
      passosPorLoop = passosPorSegundo / portTICK_PERIOD_MS;  // portTICK_PERIOD_MS é a duração de cada tick (geralmente 1 ms em ESP-IDF)
      RmotorPasso(passosPorLoop);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    gpio_set_level(PIN_STEP, 0);
}


// Função para ligar o motor
void despejar_comida() {
  RmotorLigado = 1;
  //xTaskCreate(gira_motor, "gira_motor", 2048, NULL, 5, NULL);
  Rgira_motor(1);
}

void girar_sentido_contrario() {
  RmotorLigado = 1;
  //xTaskCreate(gira_motor, "gira_motor", 2048, NULL, 5, NULL);
  Rgira_motor(0);
}

// Função para desligar o motor
void Rdesligar_motor() {
  RmotorLigado = 0;
}
