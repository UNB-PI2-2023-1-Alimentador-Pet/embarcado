// variaveis_globais.c

#include "variaveis_globais.h"

float peso_bandeja = 0;
float nivel_racao = 0;
float sensor_presenca = 100;

float getPesoBandeja() {
    return peso_bandeja;
}

void setPesoBandeja(float peso) {
    peso_bandeja = peso;
}

float getNivelRacao() {
    return nivel_racao;
}

void setNivelRacao(float nivel) {
    nivel_racao = nivel;
}

float getSensorPresenca() {
    return sensor_presenca;
}

void setSensorPresenca(float valor) {
    sensor_presenca = valor;
}
