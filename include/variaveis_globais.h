// variaveis_globais.h

#ifndef VARIAVEIS_GLOBAIS_H
#define VARIAVEIS_GLOBAIS_H

extern float peso_bandeja;
extern float nivel_racao;
extern float sensor_presenca;

// Funções de acesso
float getPesoBandeja();
void setPesoBandeja(float peso);
float getNivelRacao();
void setNivelRacao(float nivel);
float getSensorPresenca();
void setSensorPresenca(float valor);

#endif // VARIAVEIS_GLOBAIS_H
