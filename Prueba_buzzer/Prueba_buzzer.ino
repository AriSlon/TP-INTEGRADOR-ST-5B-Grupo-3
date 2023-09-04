/* Prueba Buzzer TMB12A
   Trabajo Practico Integrador

   Materia: Seminario de Informatica y Telecomunicaciones (ST).
   Grupo: 3.
   Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
   Profesor: Mirko Veckiardo.

*/

#define BUZZER_PIN 4    // Pin del buzzer
#define BUZZER_CHANNEL 0 // Canal PWM del buzzer


void setup() {

  Serial.begin(9600);

  ledcSetup(BUZZER_CHANNEL, 2000, 8); // Configurar el canal PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // Asociar el pin al canal PWM
  ledcWrite(BUZZER_CHANNEL, 0);

}

void loop() {

  ledcWrite(BUZZER_CHANNEL, 128);
  Serial.print(digitalRead(13));

  delay(1000);

  ledcWrite(BUZZER_CHANNEL, 0);
  Serial.print(digitalRead(13));

  delay(1000);

}
