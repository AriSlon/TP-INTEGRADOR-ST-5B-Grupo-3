/* Prueba Sensor de Luz BH1750
   Trabajo Practico Integrador

   Materia: Seminario de Informatica y Telecomunicaciones (ST).
   Grupo: 3.
   Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
   Profesor: Mirko Veckiardo.

*/

#include <BH1750.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


BH1750 lightMeter(0x23);
LiquidCrystal_I2C lcd(0x27, 20, 4);

int estadoMaquina;

int luz;
int lux;

unsigned long milisActuales;
unsigned long milisPrevios;

void setup() {

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

}


void loop() {

  milisActuales = millis();

  lux = lightMeter.readLightLevel();

  luz = map(lux, 0, 65535, 100, 0);

  if ((milisActuales - milisPrevios) > 1000) {


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Light: ");
    lcd.print(lux);

    milisPrevios = milisActuales;
    
  }

  Serial.print("Light: ");
  Serial.println(luz);






}
