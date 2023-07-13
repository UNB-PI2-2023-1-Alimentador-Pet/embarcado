#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include "variaveis_globais.h"

#define TRIGGER_PIN GPIO_NUM_13
#define ECHO_PIN GPIO_NUM_12


//void Pultrasonic_task(void *pvParameters) {
float Pultrasonic_task() {
  printf("Função dentro do sensor de Pultrassonic\n");
  gpio_reset_pin(TRIGGER_PIN);
  gpio_reset_pin(ECHO_PIN);
  gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

  TickType_t last_wake_time = xTaskGetTickCount();
  float dist = 0;
  float return_v = 0;

  for (int i = 0; i < 5; i++) {
    gpio_set_level(TRIGGER_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(2));
    gpio_set_level(TRIGGER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TRIGGER_PIN, 0);

    uint32_t start_time = esp_timer_get_time();
    uint32_t end_time = esp_timer_get_time();

    while (gpio_get_level(ECHO_PIN) == 0) {
      start_time = esp_timer_get_time();
    }
    while (gpio_get_level(ECHO_PIN) == 1) {
      end_time = esp_timer_get_time();
    }

    uint32_t duration = end_time - start_time;
    float distance = (duration * (343.0 / 1000000)) / 2;

    //printf("Leitura na função Distancia: %.2f cm\n", distance * 100);

    if (distance * 100 > dist)
      dist = distance * 100;

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(100));
  }
  return_v = dist;
  return return_v;
  //vTaskDelete(NULL); 
}


float Psensor_nivel() {
  float return_value = Pultrasonic_task();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  //xTaskCreate(Pultrasonic_task, "Pultrasonic_task", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
  printf("\n\nLeitura do sensor de presenca: %f cm\n\n", return_value);
  setSensorPresenca(return_value);
  return return_value;
}
