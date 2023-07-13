#ifndef SENSOR_NIVEL_H
#define SENSOR_NIVEL_H

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_timer.h>

#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define LED_PIN 13

void ultrasonic_task(void *pvParameters);
float sensor_nivel();

#endif  // SENSOR_NIVEL_H
