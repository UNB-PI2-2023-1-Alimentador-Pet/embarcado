#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include "variaveis_globais.h"
#include "mqtt.h"

#define TRIGGER_PIN GPIO_NUM_14
#define ECHO_PIN GPIO_NUM_27

//void ultrasonic_task(void *pvParameters) {
float ultrasonic_task() {
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
    char message[200];
    sprintf(message, "duration: %lu, distance: %lf", duration, distance);
    mqtt_app_publish("feeder/sensor", message, 1);

    if (distance * 100 > dist)
      dist = distance * 100;

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(200));
  }
  return_v = dist;
  return return_v;
  //vTaskDelete(NULL); 
}


float sensor_nivel() {
  float return_value = ultrasonic_task();
  vTaskDelay(1100 / portTICK_PERIOD_MS);
  //xTaskCreate(ultrasonic_task, "ultrasonic_task", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
  printf("\n\n\nLeitura na função sensor_nivel: %f cm\n\n\n", return_value);
  setNivelRacao(return_value);
  return return_value;
}
