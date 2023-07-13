// #include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// // #include <esp_system.h>
// // #include <driver/gpio.h>

// // // Função para girar o motor de passo
// // void motorPasso(int numPassos);

// // // Tarefa principal
// // //void aciona_motor(void);
// // //void disableMotor(void);
// // void abrir_bandeja(void);
// // void fechar_bandeja(void);
// // void desligar_motor(void);
// // void girar_motor(int sentido);
// // void motorPasso(int numPassos);


#ifndef MOTOR_H
#define MOTOR_H

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>

void abrir_bandeja();
void fechar_bandeja();
void desligar_motor();
void girar_motor(int sentido);
void motorPasso(int numPassos);

#endif /* MOTOR_H */