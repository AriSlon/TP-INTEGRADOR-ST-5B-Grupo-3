
/* Prueba Sensor de Temperatura BMP280
 * Trabajo Practico Integrador 
 * 
 * Materia: Seminario de Informatica y Telecomunicaciones (ST).
 * Grupo: 3.
 * Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
 * Profesor: Mirko Veckiardo.
 * 
 */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>


Adafruit_BMP280 bmp; // I2C


void setup() {
  
  Serial.begin(9600);
  
  while ( !Serial ) delay(100); 
  
  Serial.println(F("BMP280 test"));
  
  unsigned status;

  status = bmp.begin(0x76);
  
  if (!status) {
    
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  
    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
}
