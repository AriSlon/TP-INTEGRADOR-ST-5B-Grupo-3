#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>


#define PIN_BOTON_1 34
#define PIN_BOTON_2 35
#define PIN_BOTON_3 32
#define PIN_BOTON_4 33
#define PIN_BOTON_5 25
#define PIN_SENSOR 36


Adafruit_BMP280 bmp; // I2C
BH1750 lightMeter(0x23);
LiquidCrystal_I2C lcd(0x27, 20, 4);


float humedad;

int estadoBoton1;
int estadoBoton2;
int estadoBoton3;
int estadoBoton4;
int estadoBoton5;
int estadoMaquina;

unsigned long milisActuales;
unsigned long milisPrevios;


void setup() {

  Serial.begin(9600);

  pinMode(PIN_BOTON_1, INPUT);
  pinMode(PIN_BOTON_2, INPUT);
  pinMode(PIN_BOTON_3, INPUT);
  pinMode(PIN_BOTON_4, INPUT);
  pinMode(PIN_BOTON_5, INPUT);
  pinMode(16, OUTPUT);
  pinMode(13, OUTPUT);

  while ( !Serial ) delay(100);

  Serial.println(F("BMP280 test"));

  unsigned status;

  status = bmp.begin(0x76);

  if (!status) {

    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
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

  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

  lcd.init();
  lcd.backlight();

}



void loop() {

  estadoBoton1 = digitalRead(PIN_BOTON_1);
  estadoBoton2 = digitalRead(PIN_BOTON_2);
  estadoBoton3 = digitalRead(PIN_BOTON_3);
  estadoBoton4 = digitalRead(PIN_BOTON_4);
  estadoBoton5 = digitalRead(PIN_BOTON_5);

  Serial.print("Boton 1: ");
  Serial.println(estadoBoton1);
  Serial.print("Boton 2: ");
  Serial.println(estadoBoton2);
  Serial.print("Boton 3: ");
  Serial.println(estadoBoton3);
  Serial.print("Boton 4: ");
  Serial.println(estadoBoton4);
  Serial.print("Boton 5: ");
  Serial.println(estadoBoton5);

}

/*

  void loop() {

    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

  }

  void loop() {

  uint16_t lux = lightMeter.readLightLevel();

  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  }

  void loop() {

  humedad = analogRead(PIN_SENSOR);
  Serial.println(humedad);

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

  void loop() {

  digitalWrite(16, HIGH);
  digitalWrite(13, HIGH);

  delay(5000);

  digitalWrite(16, LOW);
  digitalWrite(13, LOW);

  delay(5000);

  }


*/
