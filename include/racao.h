#ifndef RACAO_H
#define RACAO_H

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>

void RmotorPasso(int numPassos);
void Rgira_motor(int sentido);
void despejar_comida();
void girar_sentido_contrario();
void Rdesligar_motor();

#endif /* RACAO_H */
