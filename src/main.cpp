#include <Arduino.h>

#define PWM_PIN    9
#define BUTTON_PIN 2

int velocidades[] = {64, 128, 192, 254};
int nivel = 0;

void setup() {
  pinMode(PWM_PIN, OUTPUT); // motor saida
  pinMode(BUTTON_PIN, INPUT); // botao entrada  
  analogWrite(PWM_PIN, velocidades[0]); // inicia o motor na primeira velo
}

void loop() {
  // aguarda apertar, verifica se apertamo
  if (digitalRead(BUTTON_PIN) == HIGH) {

    // avança nível
    nivel++;
    if (nivel > 3) nivel = 0;
    analogWrite(PWM_PIN, velocidades[nivel]); // aplica nova velo

    // aguarda soltar antes de continuar pra n bugar, evita pra n avançar demais
    while (digitalRead(BUTTON_PIN) == HIGH);
    delay(100);
  }
}