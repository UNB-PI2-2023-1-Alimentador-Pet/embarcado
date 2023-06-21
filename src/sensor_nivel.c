#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_timer.h>
//definição dos pinos
#define TRIGGER_PIN 12
#define ECHO_PIN 14
// #define LED_PIN 13

void ultrasonic_task(void *pvParameters) {
  gpio_reset_pin(TRIGGER_PIN);
  gpio_reset_pin(ECHO_PIN);
  gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

  TickType_t last_wake_time = xTaskGetTickCount();

  while (1) {
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

    printf("Distancia: %.2f cm\n", distance * 100);

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000));
  }
}


void sensor_nivel() {
  xTaskCreate(ultrasonic_task, "ultrasonic_task", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}
