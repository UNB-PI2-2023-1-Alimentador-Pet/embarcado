#ifndef PRESENCA_H
#define PRESENCA_H

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_timer.h>

#define TRIGGER_PIN 2
#define ECHO_PIN 4

void Pultrasonic_task(void *pvParameters);
float Psensor_nivel();

#endif  // PRESENCA_H
