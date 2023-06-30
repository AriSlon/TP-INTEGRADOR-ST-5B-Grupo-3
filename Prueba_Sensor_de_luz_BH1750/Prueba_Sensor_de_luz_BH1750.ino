/* Prueba Sensor de Luz BH1750
   Trabajo Practico Integrador

   Materia: Seminario de Informatica y Telecomunicaciones (ST).
   Grupo: 3.
   Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
   Profesor: Mirko Veckiardo.

*/

#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter(0x23);

void setup() {

  Serial.begin(9600);

  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

}

void loop() {

  uint16_t lux = lightMeter.readLightLevel();

  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

}
