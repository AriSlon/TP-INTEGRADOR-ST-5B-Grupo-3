/* Prueba Display LCD 20x4
 * Trabajo Practico Integrador 
 * 
 * Materia: Seminario de Informatica y Telecomunicaciones (ST).
 * Grupo: 3.
 * Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
 * Profesor: Mirko Veckiardo.
 * 
 */

#include <LiquidCrystal_I2C.h>

int estadoMaquina;

unsigned long milisActuales;
unsigned long milisPrevios;


LiquidCrystal_I2C lcd(0x27, 20, 4); 

void setup() {
  
  lcd.init();                      
  lcd.backlight();

}


void loop() {

  milisActuales = millis();

  switch (estadoMaquina) {

    case 0:

      lcd.setCursor(0, 0);
      lcd.print("Test Funcionamiento");
      lcd.setCursor(0, 1);
      lcd.print("Pantalla LCD");
      lcd.setCursor(0, 2);
      lcd.print("20x4 I2C");


      if ((milisActuales - milisPrevios) > 6000) {


        estadoMaquina = 1;
        milisPrevios = milisActuales;

      }

      break;

    case 1:

      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Hola Veckiardo");

      estadoMaquina = 2;
      
      break;

      case 2:

      break;

  }

}
