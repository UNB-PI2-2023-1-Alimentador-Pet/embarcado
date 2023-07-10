#ifndef FUNCOES_H
#define FUNCOES_H

void task_balanca(void *pvParameters);
void task_sensor_nivel(void *pvParameters);
void task_sensor_presenca(void *pvParameters);
void verifica_nivel();
void verifica_presenca();
void aciona_fluxo_de_tarefas(int tempo_min, int peso_gramas);

#endif  // FUNCOES_H
