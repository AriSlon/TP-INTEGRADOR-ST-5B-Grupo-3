#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>


#define PIN_BOTON_1 34
#define PIN_BOTON_2 35
#define PIN_BOTON_3 32
#define PIN_BOTON_4 33
#define PIN_BOTON_5 25

#define PIN_BOTON_DERECHA 34
#define PIN_BOTON_IZQUIERDA 35
#define PIN_BOTON_ARRIBA 32
#define PIN_BOTON_ABAJO 33
#define PIN_BOTON_ENTER 25

#define PIN_SENSOR_HUMEDAD 2
#define PIN_RELE_COOLER 15
#define PIN_LED_ROJO 12
#define PIN_LED_AMARILLO 14
#define PIN_LED_VERDE 27

#define BUZZER_PIN 4    // Pin del buzzer
#define BUZZER_CHANNEL 0 // Canal PWM del buzzer

#define PRESIONADO 0
#define SUELTO 1

#define ARRIBA 0
#define ABAJO 1

#define PANTALLA_1 0
#define ESPERA_1 1
#define PANTALLA_2 2
#define ESPERA_2 3



Adafruit_BMP280 bmp;

BH1750 lightMeter(0x23);

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences preferences;
Preferences preferences2;

int estadoBoton1;
int estadoBoton2;
int estadoBoton3;
int estadoBoton4;
int estadoBoton5;
int estadoMaquinaGeneral;

unsigned long milisActuales;
unsigned long milisPrevios;

bool estadoCooler;

int luzPorcentaje;
int luz;

int humedad;
int humedadPorcentaje;

int valorUmbralTemp;
int valorUmbralHum;

int cursorPantalla;

bool chequeoCursor;

void setup() {

  Serial.begin(9600);

  pinMode(PIN_BOTON_1, INPUT);
  pinMode(PIN_BOTON_2, INPUT);
  pinMode(PIN_BOTON_3, INPUT_PULLUP);
  pinMode(PIN_BOTON_4, INPUT_PULLUP);
  pinMode(PIN_BOTON_5, INPUT_PULLUP);
  pinMode(PIN_RELE_COOLER, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);


  // /*while ( !Serial ) delay(100);

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

  // Default settings from datasheet.

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode.
                  Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering.
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time.

  //*/

  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

  preferences.begin("memoria", valorUmbralTemp);
  preferences2.begin("memoria", valorUmbralHum);


  valorUmbralTemp = preferences2.getInt("memoria", 0);
  valorUmbralHum = preferences.getInt("memoria", 0);


  ledcSetup(BUZZER_CHANNEL, 2000, 8); // Configurar el canal PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // Asociar el pin al canal PWM
  ledcWrite(BUZZER_CHANNEL, 0);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  pantalla1();

  lcd.setCursor(0, 19);
  lcd.print("*");

}



void loop() {

  milisActuales = millis();

  estadoBoton1 = !digitalRead(PIN_BOTON_1);
  estadoBoton2 = digitalRead(PIN_BOTON_2);
  estadoBoton3 = digitalRead(PIN_BOTON_3);
  estadoBoton4 = digitalRead(PIN_BOTON_4);
  estadoBoton5 = digitalRead(PIN_BOTON_5);

  /*Serial.print("Boton 1: ");
    Serial.println(estadoBoton1);
    Serial.print("Boton 2: ");
    Serial.println(estadoBoton2);
    Serial.print("Boton 3: ");
    Serial.println(estadoBoton3);
    Serial.print("Boton 4: ");
    Serial.println(estadoBoton4);
    Serial.print("Boton 5: ");
    Serial.println(estadoBoton5);
  */

  Serial.println(cursorPantalla);

  digitalWrite(PIN_LED_VERDE, HIGH);
  digitalWrite(PIN_LED_AMARILLO, HIGH);
  digitalWrite(PIN_LED_ROJO, HIGH);

  humedad = analogRead(PIN_SENSOR_HUMEDAD);
  humedadPorcentaje = map(humedad, 0, 2950, 0, 100);

  luz = lightMeter.readLightLevel();

  luzPorcentaje = map(luz, 0, 65535, 0, 100);

  if (PIN_RELE_COOLER == HIGH) {

    estadoCooler = 1;
  }

  if (PIN_RELE_COOLER == LOW) {

    estadoCooler = 0;
  }

  if (cursorPantalla < 0) {
    cursorPantalla = 0;
  }

  if (cursorPantalla > 3) {
    cursorPantalla = 3;
  }

  maquinaDeEstadosGeneral();

}


void maquinaDeEstadosGeneral () {

  switch (estadoMaquinaGeneral) {

    case 0:

      if (cursorPantalla == 0) {
        lcd.setCursor(19, 0);
        lcd.print("*");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");

      }

      if (cursorPantalla == 1) {
        lcd.setCursor(19, 0);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print("*");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");

      }

      if (cursorPantalla == 2) {
        lcd.setCursor(19, 0);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print("*");
        lcd.setCursor(19, 3);
        lcd.print(" ");

      }

      if (cursorPantalla == 3) {
        lcd.setCursor(19, 0);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print("*");

      }

      if (estadoBoton1 == PRESIONADO) {
        chequeoCursor = ABAJO;
        estadoMaquinaGeneral = 6;
      }

      if (estadoBoton2 == PRESIONADO) {
        chequeoCursor = ARRIBA;
        estadoMaquinaGeneral = 6;
      }



      if ((milisActuales - milisPrevios) > 1000) {

        pantalla1();

        milisPrevios = milisActuales;

      }

      if (estadoBoton4 == PRESIONADO && estadoBoton5 == PRESIONADO) {
        estadoMaquinaGeneral = 1;
      }

      break;

    case 1:

      pantalla1();

      if (estadoBoton4 == SUELTO && estadoBoton5 == SUELTO) {
        estadoMaquinaGeneral = 2;
        lcd.clear();
      }


      break;

    case 2:

      pantalla2();

      if (estadoBoton3 == PRESIONADO && estadoBoton4 == PRESIONADO) {
        estadoMaquinaGeneral = 3;
      }

      if (estadoBoton3 == PRESIONADO && estadoBoton5 == PRESIONADO) {
        estadoMaquinaGeneral = 4;
      }

      if (estadoBoton4 == PRESIONADO && estadoBoton5 == PRESIONADO) {
        estadoMaquinaGeneral = 5;
      }

      break;

    case 3:

      pantalla2();

      if (estadoBoton3 == SUELTO && estadoBoton4 == SUELTO) {
        valorUmbralHum += 1;
        estadoMaquinaGeneral = 2;
      }

      break;

    case 4:

      pantalla2();

      if (estadoBoton3 == SUELTO && estadoBoton4 == SUELTO) {
        valorUmbralTemp += 1;
        estadoMaquinaGeneral = 2;
      }

      break;

    case 5:

      pantalla2();

      if (estadoBoton4 == SUELTO && estadoBoton5 == SUELTO) {
        preferences2.putInt("memoria", valorUmbralTemp);
        preferences2.putInt("memoria", valorUmbralHum);

        lcd.clear();
        estadoMaquinaGeneral = 0;
      }

      break;

    case 6:

      pantalla1();

      if (estadoBoton1 == SUELTO && chequeoCursor == ABAJO ) {
        cursorPantalla += 1;
        estadoMaquinaGeneral = 0;
      }

      if (estadoBoton2 == SUELTO && chequeoCursor == ARRIBA ) {
        cursorPantalla -= 1;
        estadoMaquinaGeneral = 0;
      }

      break;

  }
}

void pantalla1() {

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(bmp.readTemperature());

  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(humedadPorcentaje);

  lcd.setCursor(0, 2);
  lcd.print("Luz: ");
  lcd.print(luzPorcentaje);


  lcd.setCursor(0, 3);
  lcd.print("Cooler: ");

  if (estadoCooler == 1) {
    lcd.setCursor(8, 3);
    lcd.print("On");
  }

  if (estadoCooler == 0) {
    lcd.setCursor(8, 3);
    lcd.print("Off");
  }



}

void pantalla2() {

  lcd.setCursor(0, 0);
  lcd.print("Umbral Temp: ");
  lcd.print(valorUmbralTemp);

  lcd.setCursor(0, 1);
  lcd.print("Umbral Humedad: ");
  lcd.print(valorUmbralHum);



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

  humedad = analogRead(PIN_SENSOR_HUMEDAD);
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

  Serial.print(digitalRead(13));
  ledcWrite(BUZZER_CHANNEL, 128);

  delay(2000);

  ledcWrite(BUZZER_CHANNEL, 0);

  delay(2000);

  }

  void loop() {

  digitalWrite(PIN_RELE_COOLER, HIGH);

  delay(5000);

  digitalWrite(PIN_RELE_COOLER, LOW);

  delay(5000);

  }

  void loop() {

  digitalWrite(PIN_LED_ROJO, HIGH);
  digitalWrite(PIN_LED_AMARILLO, HIGH);
  digitalWrite(PIN_LED_VERDE, HIGH);


  #define PIN_LED_ROJO 12
  #define PIN_LED_AMARILLO 14
  #define PIN_LED_VERDE 27

  void setup() {
  // put your setup code here, to run once:

  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);


  }

  void loop() {

  digitalWrite(PIN_LED_VERDE, HIGH);
  digitalWrite(PIN_LED_AMARILLO, HIGH);
  digitalWrite(PIN_LED_ROJO, HIGH);

  }
*/
