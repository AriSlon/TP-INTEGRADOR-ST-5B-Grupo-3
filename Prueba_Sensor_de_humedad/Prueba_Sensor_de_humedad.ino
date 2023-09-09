/* Prueba Sensor de humedad Higrometro
 * Trabajo Practico Integrador
 *  
 * Materia: Seminario de Informatica y Telecomunicaciones (ST).
 * Grupo: 3.
 * Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
 * Profesor: Mirko Veckiardo.
 *
*/

#define PIN_SENSOR 0

float humedad;

void setup() {

  Serial.begin(9600);

}

void loop() {

  humedad = analogRead(PIN_SENSOR);
  Serial.println(humedad);

}
