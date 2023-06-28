// Definição dos pinos
const int pirPin = 4;    // Pino do sensor PIR
const int ledPin = 13;   // Pino do LED

void setup() {
  pinMode(pirPin, INPUT); // Configura o pino do sensor PIR como entrada
  pinMode(ledPin, OUTPUT); // Configura o pino do LED como saída
  Serial.begin(9600);

}

void loop() {
  int pirValue = digitalRead(pirPin); // Lê o valor do sensor PIR
  Serial.println("\\\\nova leitura! \\\\ "); // Exibe a mensagem no monitor serial

  if (pirValue == HIGH) { // Se movimento é detectado
    //Serial.println("nova leitura!"); // Exibe a mensagem no monitor serial
    digitalWrite(ledPin, HIGH); // Acende o LED
    Serial.println("Detectou um obstáculo!"); // Exibe a mensagem no monitor serial
    //delay(500);
  } else if(pirValue == LOW) {
    //Serial.println("nova leitura!"); // Exibe a mensagem no monitor serial
    digitalWrite(ledPin, LOW); // Desliga o LED
    Serial.println("nao detectou nada!"); // Exibe a mensagem no monitor serial
    //delay(500);

  }
   Serial.println("\\\\fim da  leitura! \\\\ "); // Exibe a mensagem no monitor serial
  delay(200); // Aguarda um breve intervalo antes de repetir o loop
}
