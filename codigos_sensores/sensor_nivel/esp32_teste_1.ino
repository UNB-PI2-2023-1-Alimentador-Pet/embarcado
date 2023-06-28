// Inclua a biblioteca do ESP32
#include <Arduino.h>

// Define os pinos do sensor ultrassônico
const int triggerPin = 12;
const int echoPin = 14;
const int ledPin = 13;

void setup() {
  // Inicializa o monitor serial
  Serial.begin(9600);

  // Define os pinos do sensor ultrassônico como saída e entrada
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Define o pino de trigger em nível baixo por 2 microssegundos
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);

  // Emite um pulso de ultrassom
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Mede o tempo de duração do pulso de eco
  long duration = pulseIn(echoPin, HIGH);

  // Calcula a distância com base no tempo de duração
  // A velocidade do som é de aproximadamente 343 metros por segundo
  // A fórmula para calcular a distância é: distância = velocidade * tempo / 2
  float distance = (duration * 0.0343) / 2;

  // Exibe a distância no monitor serial
  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Verifica se a distância é maior que 15 cm
  if (distance > 15) {
    digitalWrite(ledPin, HIGH); // Ligar o LED
  } else {
    digitalWrite(ledPin, LOW); // Desligar o LED
  }

  // Aguarda um intervalo antes de realizar a próxima leitura
  delay(1000);
}
